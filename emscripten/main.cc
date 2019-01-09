#include "main/realmain.h"
#include "main/lsp/lsp.h"
#include "main/lsp/json_types.h"
#include <regex>
#include <sstream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
// so that we can compile this file with normal C++ compiler
#define EMSCRIPTEN_KEEPALIVE
#endif

using namespace std;
using namespace sorbet;
using namespace sorbet::realmain::lsp;

typedef struct Loop {
    void (*respond)(const char *);
    LSPLoop *lspLoop;
    std::stringstream *lspOstream;
} Loop;

regex contentLengthRegex("^Content-Length: ([0-9]+)$");

optional<unique_ptr<JSONDocument<JSONBaseType>>> parseLSPResponse(string message) {
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
            // ADD_FAILURE() << fmt::format("Received invalid LSP message from server; response is not a ResponseMessage "
            //                              "or a NotificationMessage:\n{}",
            //                              message);
        }
    } catch (DeserializationError e) {
        // ADD_FAILURE() << fmt::format("Encountered deserialization error: {}\nOriginal message:\n{}", e.what(), message);
    }
    return nullopt;
}

vector<unique_ptr<JSONDocument<JSONBaseType>>> sendRaw(Loop *loop, string json) {
    // TODO(jvilk): Share parsing code with main LSP codebase.
    // processRequest does not require Content-Length or other headers.
    loop->lspLoop->processRequest(json);

    vector<unique_ptr<JSONDocument<JSONBaseType>>> rv;
    string responses = loop->lspOstream->str();
    // Reset error flags and change contents of stream to the empty string.
    loop->lspOstream->clear();
    loop->lspOstream->str(std::string());

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
            // ADD_FAILURE() << "Couldn't find Content-Length in response.";
            return rv;
        }
        string contentLengthLine = responses.substr(pos, newlinePos - pos);
        smatch matches;
        if (!regex_match(contentLengthLine, matches, contentLengthRegex)) {
            // ADD_FAILURE() << fmt::format("Invalid Content-Length line:\n{}", contentLengthLine);
            return rv;
        }

        int contentLength = stoi(matches[1]);
        pos = newlinePos + 2;
        string emptyLine = responses.substr(pos, 2);
        if (emptyLine != "\r\n") {
            // ADD_FAILURE() << fmt::format("A carraige return and a newline must separate headers and the body of the "
            //                              "LSP message. Instead, got:\n{}",
            //                              emptyLine);
            return rv;
        }
        pos += 2;

        if (pos + contentLength > len) {
            // ADD_FAILURE() << fmt::format(
            //     "Invalid Content-Length: Server specified `{}`, but only `{}` characters provided.", contentLength,
            //     len - pos);
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

/**
 * Required objects that Sorbet assumes we 'own'. Not having these here would result in memory errors, as Sorbet
 * captures references to them. Normally these are stack allocated, but we cannot do that with gtests which
 * implicitly call `SetUp()`.
 */
std::shared_ptr<spd::logger> logger;
std::shared_ptr<spd::logger> typeErrorsConsole;
realmain::options::Options opts;
std::shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink;
std::unique_ptr<WorkerPool> workers;

extern "C" {
void EMSCRIPTEN_KEEPALIVE typecheck(const char *rubySrc) {
    const char *argv[] = {"sorbet", "--color=always", "--silence-dev-message", "-e", rubySrc};
    sorbet::realmain::realmain(std::size(argv), const_cast<char **>(argv));
}

Loop* EMSCRIPTEN_KEEPALIVE lsp_initialize(void (*respond)(const char *)) {
    /** The LSP 'server', which runs in the same thread as the tests. */
    LSPLoop *lspLoop;

    /** The output stream used by LSP. Lets us drain all response messages after sending messages to LSP. */
    std::stringstream *lspOstream = new std::stringstream();
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
    lspLoop = new LSPLoop(move(gs), opts, logger, *workers.get(), std::cin, *lspOstream, true, true, true);

    Loop *loop = new Loop;
    loop->lspLoop = lspLoop;
    loop->lspOstream = lspOstream;
    loop->respond = respond;
    return loop;
    // return new LSPLoop(move(gs), opts, logger, *workers.get(), std::cin, lspOstream, true, true, true);
}

void EMSCRIPTEN_KEEPALIVE lsp_send(Loop *loop, char *message) {
    vector<unique_ptr<JSONDocument<JSONBaseType>>> responses = sendRaw(loop, message);
    for (auto &response : responses) {
        loop->respond(response.get()->root->toJSON().c_str());
    }
}

int main(int argc, char **argv) {}
}
