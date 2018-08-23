#include "absl/strings/match.h"
#include "common/Timer.h"
#include "lsp.h"

using namespace std;

namespace sorbet {
namespace realmain {
namespace lsp {

void LSPLoop::processRequest(rapidjson::Document &d) {
    const LSPMethod method = LSPMethod::getByName({d["method"].GetString(), d["method"].GetStringLength()});

    ENFORCE(method.kind == LSPMethod::Kind::ClientInitiated || method.kind == LSPMethod::Kind::Both);

    if (!ensureInitialized(method, d)) {
        return;
    }
    if (method.isNotification) {
        logger->debug("Processing notification {} ", (string)method.name);
        if (method == LSPMethod::DidChangeWatchedFiles()) {
            sendRequest(LSPMethod::ReadFile(), d["params"],
                        [&](rapidjson::Value &edits) -> void {
                            ENFORCE(edits.IsArray());
                            Timer timeit(logger, "handle update");
                            vector<shared_ptr<core::File>> files;

                            for (auto it = edits.Begin(); it != edits.End(); ++it) {
                                rapidjson::Value &change = *it;
                                string uri(change["uri"].GetString(), change["uri"].GetStringLength());
                                string content(change["content"].GetString(), change["content"].GetStringLength());
                                if (absl::StartsWith(uri, rootUri)) {
                                    files.emplace_back(make_shared<core::File>(remoteName2Local(uri), move(content),
                                                                               core::File::Type::Normal));
                                }
                            }

                            tryFastPath(files);
                            pushErrors();
                        },
                        [](rapidjson::Value &error) -> void {});
        }
        if (method == LSPMethod::TextDocumentDidChange()) {
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
            // TODO: if this is ever updated to support diffs, be aware that the coordinator thread should be
            // taught about it too: it merges consecutive TextDocumentDidChange
            if (absl::StartsWith(uri, rootUri)) {
                files.emplace_back(
                    make_shared<core::File>(remoteName2Local(uri), move(content), core::File::Type::Normal));

                tryFastPath(files);
                pushErrors();
            }
        }
        if (method == LSPMethod::TextDocumentDidOpen()) {
            Timer timeit(logger, "handle open");
            vector<shared_ptr<core::File>> files;
            auto &edits = d["params"];
            ENFORCE(edits.IsObject());
            /*
              {
              "textDocument":{"uri":"file:///Users/dmitry/stripe/pay-server/cibot/lib/cibot/gerald.rb","version":2},
                "contentChanges":[{"text":"..."}]
                */
            string uri(edits["textDocument"]["uri"].GetString(), edits["textDocument"]["uri"].GetStringLength());
            string content(edits["textDocument"]["text"].GetString(), edits["textDocument"]["text"].GetStringLength());
            if (absl::StartsWith(uri, rootUri)) {
                files.emplace_back(
                    make_shared<core::File>(remoteName2Local(uri), move(content), core::File::Type::Normal));

                tryFastPath(files);
                pushErrors();
            }
        }
        if (method == LSPMethod::Initialized()) {
            // initialize ourselves
            {
                Timer timeit(logger, "index");
                reIndexFromFileSystem();
                vector<shared_ptr<core::File>> changedFiles;
                runSlowPath(move(changedFiles));
                ENFORCE(finalGs);
                pushErrors();
                this->globalStateHashes = computeStateHashes(finalGs->getFiles());
            }
        }
        if (method == LSPMethod::Exit()) {
            return;
        }
    } else {
        logger->debug("Processing request {}", method.name);
        // is request
        rapidjson::Value result;
        int errorCode = 0;
        string errorString;
        if (d.FindMember("cancelled") != d.MemberEnd()) {
            errorCode = (int)LSPErrorCodes::RequestCancelled;
            errorString = "Request was cancelled";
            sendError(d, errorCode, errorString);
        } else if (method == LSPMethod::Initialize()) {
            result.SetObject();
            rootUri = string(d["params"]["rootUri"].GetString(), d["params"]["rootUri"].GetStringLength());
            string serverCap = "{\"capabilities\": "
                               "   {"
                               "       \"textDocumentSync\": 1, "
                               "       \"documentSymbolProvider\": true, "
                               "       \"workspaceSymbolProvider\": true, "
                               "       \"definitionProvider\": true, "
                               "       \"hoverProvider\":true,"
                               "       \"referencesProvider\":true,"
                               "       \"signatureHelpProvider\": { "
                               "           \"triggerCharacters\": [\"(\", \",\"]"
                               "       },"
                               "       \"completionProvider\": { "
                               "           \"triggerCharacters\": [\".\"]"
                               "       }"
                               "   }"
                               "}";
            rapidjson::Document temp;
            auto &r = temp.Parse(serverCap.c_str());
            ENFORCE(!r.HasParseError());
            result.CopyFrom(temp, alloc);
            sendResult(d, result);
        } else if (method == LSPMethod::Shutdown()) {
            // return default value: null
            sendResult(d, result);
        } else if (method == LSPMethod::TextDocumentDocumentSymbol()) {
            handleTextDocumentDocumentSymbol(result, d);
        } else if (method == LSPMethod::WorkspaceSymbols()) {
            handleWorkspaceSymbols(result, d);
        } else if (method == LSPMethod::TextDocumentDefinition()) {
            handleTextDocumentDefinition(result, d);
        } else if (method == LSPMethod::TextDocumentHover()) {
            handleTextDocumentHover(result, d);
        } else if (method == LSPMethod::TextDocumentCompletion()) {
            handleTextDocumentCompletion(result, d);
        } else if (method == LSPMethod::TextDocumentSignatureHelp()) {
            handleTextSignatureHelp(result, d);
        } else if (method == LSPMethod::TextDocumentRefernces()) {
            handleTextDocumentReferences(result, d);
        } else {
            ENFORCE(!method.isSupported, "failing a supported method");
            errorCode = (int)LSPErrorCodes::MethodNotFound;
            errorString = fmt::format("Unknown method: {}", method.name);
            sendError(d, errorCode, errorString);
        }
    }
}
} // namespace lsp
} // namespace realmain
} // namespace sorbet