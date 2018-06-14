#include "LSPLoop.h"
#include "core/ErrorQueue.h"
#include "core/Files.h"
#include "core/Unfreeze.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"
#include "spdlog/fmt/ostr.h"

using namespace std;
using namespace rapidjson;
using namespace sorbet::realmain::LSP;

namespace sorbet {
namespace realmain {

static bool startsWith(const string &str, const string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix.c_str(), prefix.size());
}

bool safeGetline(std::istream &is, std::string &t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf *sb = is.rdbuf();

    for (;;) {
        int c = sb->sbumpc();
        switch (c) {
            case '\n':
                return true;
            case '\r':
                if (sb->sgetc() == '\n')
                    sb->sbumpc();
                return true;
            case std::streambuf::traits_type::eof():
                // Also handle the case when the last line has no line ending
                if (t.empty())
                    is.setstate(std::ios::eofbit);
                return false;
            default:
                t += (char)c;
        }
    }
}

void LSPLoop::runLSP() {
    std::string json;
    Document d(&alloc);

    while (true) {
        int length = -1;
        {
            std::string line;
            while (safeGetline(cin, line)) {
                logger->trace("raw read: {}", line);
                if (line == "") {
                    break;
                }
                sscanf(line.c_str(), "Content-Length: %i", &length);
            }
            logger->trace("final raw read: {}, length: {}", line, length);
        }
        if (length < 0) {
            logger->info("eof");
            return;
        }

        json.resize(length);
        cin.read(&json[0], length);

        logger->info("Read: {}", json);
        if (d.Parse(json.c_str()).HasParseError()) {
            logger->info("json parse error");
            return;
        }

        if (handleReplies(d)) {
            continue;
        }

        const LSPMethod method = LSP::getMethod({d["method"].GetString(), d["method"].GetStringLength()});

        ENFORCE(method.kind == LSPMethod::Kind::ClientInitiated || method.kind == LSPMethod::Kind::Both);
        if (method.isNotification) {
            logger->info("Processing notification {} ", (string)method.name);
            if (method == LSP::DidChangeWatchedFiles) {
                sendRequest(LSP::ReadFile, d["params"],
                            [&](Value &edits) -> void {
                                ENFORCE(edits.IsArray());
                                Timer timeit(logger, "handle update");
                                vector<shared_ptr<core::File>> files;

                                for (auto it = edits.Begin(); it != edits.End(); ++it) {
                                    Value &change = *it;
                                    string uri(change["uri"].GetString(), change["uri"].GetStringLength());
                                    string content(change["content"].GetString(), change["content"].GetStringLength());
                                    if (startsWith(uri, rootUri)) {
                                        string subName(uri.c_str() + 1 + rootUri.length(),
                                                       uri.length() - rootUri.length() - 1);
                                        files.emplace_back(make_shared<core::File>(move(subName), move(content),
                                                                                   core::File::Type::Normal));
                                    }
                                }

                                tryFastPath(files);
                                pushErrors();
                            },
                            [](Value &error) -> void {});
            }
            if (method == LSP::TextDocumentDidChange) {
                Timer timeit(logger, "handle update");
                vector<shared_ptr<core::File>> files;
                auto &edits = d["params"];
                ENFORCE(edits.IsObject());
                /*
                  {
                  "textDocument":{"uri":"file:///Users/dmitry/stripe/pay-server/cibot/lib/cibot/gerald.rb","version":2},
                    "contentChanges":[{"text":"..."}]
                    */
                string uri(edits["textDocument"]["uri"].GetString(), edits["textDocument"]["uri"].GetStringLength());
                string content(edits["contentChanges"][0]["text"].GetString(),
                               edits["contentChanges"][0]["text"].GetStringLength());
                if (startsWith(uri, rootUri)) {
                    string subName(uri.c_str() + 1 + rootUri.length(), uri.length() - rootUri.length() - 1);
                    files.emplace_back(make_shared<core::File>(move(subName), move(content), core::File::Type::Normal));

                    tryFastPath(files);
                    pushErrors();
                }
            }
            if (method == LSP::Inititalized) {
                // initialize ourselves
                {
                    Timer timeit(logger, "index");
                    reIndex(true);
                    vector<shared_ptr<core::File>> changedFiles;
                    runSlowPath(changedFiles);
                    pushErrors();
                }
            }
            if (method == LSP::Exit) {
                return;
            }
        } else {
            logger->info("Processing request {}", method.name);
            // is request
            Value result;
            int errorCode = 0;
            string errorString;
            if (method == LSP::Initialize) {
                result.SetObject();
                rootUri = string(d["params"]["rootUri"].GetString(), d["params"]["rootUri"].GetStringLength());
                string serverCap = "{\"capabilities\": {\"textDocumentSync\": 1}}";
                Document temp;
                auto &r = temp.Parse(serverCap.c_str());
                ENFORCE(!r.HasParseError());
                result.CopyFrom(temp, alloc);
            } else if (method == LSP::Shutdown) {
                // return default value: null
            } else {
                ENFORCE(!method.isSupported, "failing a supported method");
                errorCode = (int)LSP::LSPErrorCodes::MethodNotFound;
                errorString = fmt::format("Unknown method: {}", method.name);
            }

            if (errorCode == 0) {
                sendResult(d, result);
            } else {
                sendError(d, errorCode, errorString);
            }
        }
    }
}

void LSPLoop::sendRaw(Document &raw) {
    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    raw.Accept(writer);
    string outResult = fmt::format("Content-Length: {}\r\n\r\n{}", strbuf.GetLength(), strbuf.GetString());
    logger->info("Write: {}", strbuf.GetString());
    logger->info("\n");
    std::cout << outResult << std::flush;
}

void LSPLoop::sendNotification(LSPMethod meth, Value &data) {
    ENFORCE(meth.isNotification);
    ENFORCE(meth.kind == LSPMethod::Kind::ServerInitiated || meth.kind == LSPMethod::Kind::Both);
    Document request(&alloc);
    request.SetObject();
    string idStr = fmt::format("ruby-typer-req-{}", ++requestCounter);

    {
        // fill in method
        Value method;
        method.SetString(meth.name.data(), meth.name.size());
        request.AddMember("method", method, alloc);
    }

    {
        // fill in params
        request.AddMember("params", data, alloc);
    }

    sendRaw(request);
}
void LSPLoop::sendRequest(LSPMethod meth, Value &data, std::function<void(Value &)> onComplete,
                          std::function<void(Value &)> onFail) {
    ENFORCE(!meth.isNotification);
    ENFORCE(meth.kind == LSPMethod::Kind::ServerInitiated || meth.kind == LSPMethod::Kind::Both);
    Document request(&alloc);
    request.SetObject();
    string idStr = fmt::format("ruby-typer-req-{}", ++requestCounter);

    {
        // fill in ID
        Value id;
        id.SetString(idStr.c_str(), alloc);
        request.AddMember("id", id, alloc);
    }

    {
        // fill in method
        Value method;
        method.SetString(meth.name.data(), meth.name.size());
        request.AddMember("method", method, alloc);
    }

    {
        // fill in params
        request.AddMember("params", data, alloc);
    }

    awaitingResponse[idStr] = ResponseHandler{move(onComplete), move(onFail)};

    sendRaw(request);
}

bool silenceError(core::ErrorClass what) {
    if (what == core::errors::Namer::RedefinitionOfMethod ||
        what == core::errors::Resolver::DuplicateVariableDeclaration ||
        what == core::errors::Resolver::RedefinitionOfParents) {
        return true;
    }
    return false;
}

void LSPLoop::drainErrors() {
    for (auto &e : initialGS->errorQueue->drainErrors()) {
        if (silenceError(e->what)) {
            continue;
        }
        auto file = e->loc.file;
        errorsAccumulated[file].emplace_back(move(e));

        if (!updatedErrors.empty() && updatedErrors.back() == file) {
            continue;
        }
        updatedErrors.emplace_back(file);
    }
    auto iter = errorsAccumulated.begin();
    for (; iter != errorsAccumulated.end();) {
        if (iter->first.data(*initialGS).source_type == core::File::TombStone) {
            errorsAccumulated.erase(iter++);
        } else {
            ++iter;
        }
    }
}

Value LSPLoop::loc2Range(core::Loc loc) {
    /**
       {
        start: { line: 5, character: 23 }
        end : { line 6, character : 0 }
        }
     */
    Value ret;
    ret.SetObject();
    Value start;
    start.SetObject();
    Value end;
    end.SetObject();

    auto pair = loc.position(*finalGs);
    // All LSP numbers are zero-based, ours are 1-based.
    start.AddMember("line", pair.first.line - 1, alloc);
    start.AddMember("character", pair.first.column - 1, alloc);
    end.AddMember("line", pair.second.line - 1, alloc);
    end.AddMember("character", pair.second.column - 1, alloc);

    ret.AddMember("start", start, alloc);
    ret.AddMember("end", end, alloc);
    return ret;
}

void LSPLoop::pushErrors() {
    drainErrors();

    for (auto file : updatedErrors) {
        if (file.exists()) {
            Value publishDiagnosticsParams;

            /**
             * interface PublishDiagnosticsParams {
             *      uri: DocumentUri; // The URI for which diagnostic information is reported.
             *      diagnostics: Diagnostic[]; // An array of diagnostic information items.
             * }
             **/
            /** interface Diagnostic {
             *      range: Range; // The range at which the message applies.
             *      severity?: number; // The diagnostic's severity.
             *      code?: number | string; // The diagnostic's code
             *      source?: string; // A human-readable string describing the source of this diagnostic, e.g.
             *                       // 'typescript' or 'super lint'.
             *      message: string; // The diagnostic's message. relatedInformation?:
             *      DiagnosticRelatedInformation[]; // An array of related diagnostic information
             * }
             **/

            publishDiagnosticsParams.SetObject();
            { // uri
                Value uri;
                string uriStr;
                if (file.data(*finalGs).source_type == core::File::Type::Payload) {
                    uriStr = (string)file.data(*finalGs).path();
                } else {
                    uriStr = fmt::format("{}/{}", rootUri, file.data(*finalGs).path());
                }
                uri.SetString(uriStr.c_str(), uriStr.length(), alloc);
                publishDiagnosticsParams.AddMember("uri", uri, alloc);
            }

            {
                // diagnostics
                Value diagnostics;
                diagnostics.SetArray();
                for (auto &e : errorsAccumulated[file]) {
                    Value diagnostic;
                    diagnostic.SetObject();

                    diagnostic.AddMember("range", loc2Range(e->loc), alloc);
                    diagnostic.AddMember("code", e->what.code, alloc);
                    Value message;
                    message.SetString(e->formatted.c_str(), e->formatted.length(), alloc);
                    diagnostic.AddMember("message", message, alloc);

                    // TODO: add other lines

                    diagnostics.PushBack(diagnostic, alloc);
                }
                publishDiagnosticsParams.AddMember("diagnostics", diagnostics, alloc);
            }

            sendNotification(LSP::PushDiagnostics, publishDiagnosticsParams);
        }
    }
    updatedErrors.clear();
}

void LSPLoop::sendResult(Document &forRequest, Value &result) {
    forRequest.AddMember("result", result, alloc);
    forRequest.RemoveMember("method");
    forRequest.RemoveMember("params");
    sendRaw(forRequest);
}

void LSPLoop::sendError(Document &forRequest, int errorCode, string errorStr) {
    forRequest.RemoveMember("method");
    forRequest.RemoveMember("params");
    Value error;
    error.SetObject();
    error.AddMember("code", errorCode, alloc);
    Value message(errorStr.c_str(), alloc);
    error.AddMember("message", message, alloc);
    forRequest.AddMember("error", error, alloc);
    sendRaw(forRequest);
}

bool LSPLoop::handleReplies(Document &d) {
    if (d.FindMember("result") != d.MemberEnd()) {
        if (d.FindMember("id") != d.MemberEnd()) {
            string key(d["id"].GetString(), d["id"].GetStringLength());
            auto fnd = awaitingResponse.find(key);
            if (fnd != awaitingResponse.end()) {
                auto func = move(fnd->second.onResult);
                awaitingResponse.erase(fnd);
                func(d["result"]);
            }
        }
        return true;
    }

    if (d.FindMember("error") != d.MemberEnd()) {
        if (d.FindMember("id") != d.MemberEnd()) {
            string key(d["id"].GetString(), d["id"].GetStringLength());
            auto fnd = awaitingResponse.find(key);
            if (fnd != awaitingResponse.end()) {
                auto func = move(fnd->second.onError);
                awaitingResponse.erase(fnd);
                func(d["error"]);
            }
        }
        return true;
    }
    return false;
}

void LSPLoop::reIndex(bool initial) {
    indexed.clear();
    vector<core::FileRef> inputFiles;
    vector<string> inputNames;
    if (!initial) {
        for (int i = 1; i < initialGS->filesUsed(); i++) {
            core::FileRef f(i);
            if (f.data(*initialGS, true).source_type == core::File::Type::Normal) {
                inputFiles.emplace_back(f);
            }
        }
    } else {
        inputNames = opts.inputFileNames;
    }

    for (auto &t : index(initialGS, inputNames, inputFiles, opts, workers, kvstore)) {
        int id = t->loc.file.id();
        if (id >= indexed.size()) {
            indexed.resize(id + 1);
        }
        indexed[id] = move(t);
    }
}

void LSPLoop::invalidateAllErrors() {
    errorsAccumulated.clear();
    updatedErrors.clear();
}

void LSPLoop::runSlowPath(std::vector<shared_ptr<core::File>> changedFiles) {
    logger->info("Taking slow path");
    invalidateAllErrors();
    vector<unique_ptr<ast::Expression>> indexedCopies;
    for (const auto &tree : indexed) {
        if (tree) {
            indexedCopies.emplace_back(tree->deepCopy());
        }
    }
    std::vector<core::FileRef> changedFileRefs;
    for (auto &t : changedFiles) {
        changedFileRefs.emplace_back(initialGS->enterFile(t));
    }

    std::vector<std::string> emptyInputNames;
    for (auto &t : index(initialGS, emptyInputNames, changedFileRefs, opts, workers, kvstore)) {
        indexedCopies.emplace_back(move(t));
    }
    finalGs = initialGS->deepCopy(true);
    typecheck(finalGs, resolve(*finalGs, move(indexedCopies), opts), opts, workers);
}

const LSP::LSPMethod LSP::getMethod(const absl::string_view name) {
    for (auto &candidate : ALL) {
        if (candidate.name == name) {
            return candidate;
        }
    }
    return LSPMethod{(string)name, true, LSPMethod::Kind::ClientInitiated, false};
}

void LSPLoop::tryFastPath(std::vector<shared_ptr<core::File>> changedFiles) {
    runSlowPath(changedFiles);
}

} // namespace realmain
} // namespace sorbet
