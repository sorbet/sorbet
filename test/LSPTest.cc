#include "test/LSPTest.h"

#include <regex>
#include <signal.h>

#include "main/lsp/json_types.h"
#include "main/realmain.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace sorbet::test {
using namespace std;

// Matches the Content-Length header on LSP messages.
regex contentLengthRegex("^Content-Length: ([0-9]+)$");

void LSPTest::SetUp() {
    parseTestFile();
    startLSP();
}

void LSPTest::startLSP() {
    // All of this stuff is ignored by LSP, but we need it to construct ErrorQueue/GlobalState.
    // Cargo-culting from realmain.cc and other test runners.
    stderrColorSink = make_shared<spd::sinks::ansicolor_stderr_sink_mt>();
    logger = make_shared<spd::logger>("console", stderrColorSink);
    // No threads.
    workers = make_unique<WorkerPool>(0, logger);
    typeErrorsConsole = make_shared<spd::logger>("typeDiagnostics", stderrColorSink);
    typeErrorsConsole->set_pattern("%v");
    unique_ptr<core::GlobalState> gs =
        make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*typeErrorsConsole, *logger, vector<int>({}))));
    unique_ptr<KeyValueStore> kvstore;
    realmain::createInitialGlobalState(gs, logger, opts, kvstore);
    // If we don't tell the errorQueue to ignore flushes, then we won't get diagnostic messages.
    gs->errorQueue->ignoreFlushes = true;

    // N.B.: cin will not actually be used the way we are driving LSP.
    // Configure LSPLoop to run on test files (as all test input files are "test" files), disable configatron, and
    // disable the fast path.
    lspLoop = make_unique<LSPLoop>(std::move(gs), opts, logger, *workers.get(), cin, lspOstream, true, true, true);
}

void LSPTest::parseTestFile() {
    auto expectations = GetParam();
    for (auto &sourceFile : expectations.sourceFiles) {
        filenames.insert(expectations.folder + sourceFile);
    }

    assertions = RangeAssertion::parseAssertions(expectations.sourceFileContents);

    if (expectations.expectations.find("autogen") != expectations.expectations.end()) {
        // When autogen is enabled, skip DSL passes...
        opts.skipDSLPasses = true;
        // Some autogen tests assume that some errors will occur from the resolver step, others assume the resolver
        // won't run.
        if (assertions.size() > 0) {
            // ...and stop after the resolver phase if there are errors
            opts.stopAfterPhase = realmain::options::Phase::RESOLVER;
        } else {
            // ...and stop after the namer phase if there are no errors
            opts.stopAfterPhase = realmain::options::Phase::NAMER;
        }
    }
}

optional<unique_ptr<JSONDocument<JSONBaseType>>> LSPTest::parseLSPResponse(string message) {
    rapidjson::Document document;
    document.Parse(message.c_str());

    // What did we receive: ResponseMessage, NotificationMessage, or Unknown?
    try {
        if (document.HasMember("id")) {
            // ResponseMessage
            return ResponseMessage::fromJSON(message)->dynamicCast<JSONBaseType>();
        } else if (document.HasMember("method")) {
            // NotificationMessage
            return NotificationMessage::fromJSON(message)->dynamicCast<JSONBaseType>();
        } else {
            // Something unexpected.
            ADD_FAILURE() << fmt::format("Received invalid LSP message from server; response is not a ResponseMessage "
                                         "or a NotificationMessage:\n{}",
                                         message);
        }
    } catch (DeserializationError e) {
        ADD_FAILURE() << fmt::format("Encountered deserialization error: {}\nOriginal message:\n{}", e.what(), message);
    }
    return nullopt;
}

vector<unique_ptr<JSONDocument<JSONBaseType>>> LSPTest::getLSPResponsesForRaw(string json) {
    // TODO(jvilk): Share parsing code with main LSP codebase.
    // processRequest does not require Content-Length or other headers.
    lspLoop->processRequest(json);

    vector<unique_ptr<JSONDocument<JSONBaseType>>> rv;
    string responses = lspOstream.str();
    // Reset error flags and change contents of stream to the empty string.
    lspOstream.clear();
    lspOstream.str(string());

    if (responses.length() == 0) {
        // No response.
        return rv;
    }

    // Parse the results. Should be of the form:
    // Content-Length: length\r\n
    // \r\n
    // [length characters]
    // ...in sequence.

    int pos = 0;
    int len = responses.length();
    while (pos < len) {
        int newlinePos = responses.find("\r\n", pos);
        if (newlinePos == string::npos) {
            ADD_FAILURE() << "Couldn't find Content-Length in response.";
            return rv;
        }
        string contentLengthLine = responses.substr(pos, newlinePos - pos);
        smatch matches;
        if (!regex_match(contentLengthLine, matches, contentLengthRegex)) {
            ADD_FAILURE() << fmt::format("Invalid Content-Length line:\n{}", contentLengthLine);
            return rv;
        }

        int contentLength = stoi(matches[1]);
        pos = newlinePos + 2;
        string emptyLine = responses.substr(pos, 2);
        if (emptyLine != "\r\n") {
            ADD_FAILURE() << fmt::format("A carraige return and a newline must separate headers and the body of the "
                                         "LSP message. Instead, got:\n{}",
                                         emptyLine);
            return rv;
        }
        pos += 2;

        if (pos + contentLength > len) {
            ADD_FAILURE() << fmt::format(
                "Invalid Content-Length: Server specified `{}`, but only `{}` characters provided.", contentLength,
                len - pos);
            return rv;
        }

        string messageLine = responses.substr(pos, contentLength);
        auto response = parseLSPResponse(messageLine);
        if (response.has_value()) {
            rv.push_back(move(*response));
        }
        pos += contentLength;
    }

    return rv;
}

vector<unique_ptr<JSONDocument<JSONBaseType>>> LSPTest::getLSPResponsesFor(const unique_ptr<JSONBaseType> &message) {
    return getLSPResponsesForRaw(message->toJSON());
}

} // namespace sorbet::test
