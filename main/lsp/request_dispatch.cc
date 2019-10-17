#include "common/Timer.h"
#include "lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPResult LSPLoop::processRequest(unique_ptr<core::GlobalState> gs, const string &json) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(LSPMessage::fromClient(json));
    return LSPLoop::processRequests(move(gs), move(messages));
}

LSPResult LSPLoop::processRequest(unique_ptr<core::GlobalState> gs, std::unique_ptr<LSPMessage> msg) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(move(msg));
    return processRequests(move(gs), move(messages));
}

LSPResult LSPLoop::processRequests(unique_ptr<core::GlobalState> gs, vector<unique_ptr<LSPMessage>> messages) {
    QueueState state;
    absl::Mutex mutex;
    for (auto &message : messages) {
        preprocessor.preprocessAndEnqueue(state, move(message), mutex);
    }
    ENFORCE(state.paused == false, "__PAUSE__ not supported in single-threaded mode.");

    LSPResult rv{move(gs), {}};
    for (auto &message : state.pendingRequests) {
        if (rv.gs) {
            maybeStartCommitSlowPathEdit(*rv.gs, *message);
        }
        auto rslt = processRequestInternal(move(rv.gs), *message);
        rv.gs = move(rslt.gs);
        rv.responses.insert(rv.responses.end(), make_move_iterator(rslt.responses.begin()),
                            make_move_iterator(rslt.responses.end()));
    }
    state.pendingRequests.clear();
    return rv;
}

LSPResult LSPLoop::processRequestInternal(unique_ptr<core::GlobalState> gs, const LSPMessage &msg) {
    // TODO(jvilk): Make Timer accept multiple FlowIds so we can show merged messages correctly.
    Timer timeit(logger, "process_request");
    const LSPMethod method = msg.method();

    if (!ensureInitialized(method, msg)) {
        logger->error("Serving request before got an Initialize & Initialized handshake from IDE");
        vector<unique_ptr<LSPMessage>> responses;
        if (!msg.isNotification()) {
            auto id = msg.id().value_or(0);
            auto response = make_unique<ResponseMessage>("2.0", id, msg.method());
            response->error = make_unique<ResponseError>((int)LSPErrorCodes::ServerNotInitialized,
                                                         "IDE did not initialize Sorbet correctly. No requests should "
                                                         "be made before Initialize & Initialized have been completed");
            responses.push_back(make_unique<LSPMessage>(move(response)));
        }
        return LSPResult{move(gs), move(responses)};
    }

    if (msg.isNotification()) {
        Timer timeit(logger, "notification", {{"method", convertLSPMethodToString(method)}});
        // The preprocessor should canonicalize these messages into SorbetWorkspaceEdits, so they should never appear
        // here.
        ENFORCE(method != LSPMethod::TextDocumentDidChange && method != LSPMethod::TextDocumentDidOpen &&
                method != LSPMethod::TextDocumentDidClose && method != LSPMethod::SorbetWatchmanFileChange);
        auto &params = msg.asNotification().params;
        if (method == LSPMethod::SorbetWorkspaceEdit) {
            // Note: We increment `lsp.messages.processed` when the original requests were merged into this one.
            auto &editParams = get<unique_ptr<SorbetWorkspaceEditParams>>(params);
            const u4 end = editParams->updates.versionEnd;
            const u4 start = editParams->updates.versionStart;
            // Versions are sequential and wrap around. Use them to figure out how many edits are contained within this
            // update.
            const u4 merged = min(end - start, 0xFFFFFFFF - start + end);
            auto run = runTypechecking(move(gs), move(editParams->updates));
            // Only report stats if the edit was committed.
            if (!run.canceled) {
                prodCategoryCounterInc("lsp.messages.processed", "sorbet/workspaceEdit");
                prodCategoryCounterAdd("lsp.messages.processed", "sorbet/mergedEdits", merged);
            }
            return commitTypecheckRun(move(run));
        } else if (method == LSPMethod::Initialized) {
            prodCategoryCounterInc("lsp.messages.processed", "initialized");
            auto &initParams = get<unique_ptr<InitializedParams>>(params);
            auto &updates = initParams->updates;
            globalStateHashes = move(updates.updatedFileHashes);
            indexed = move(updates.updatedFileIndexes);
            // Initialization typecheck is not cancelable.
            LSPResult result = pushDiagnostics(runSlowPath(move(gs), move(updates), /* isCancelable */ false));
            ENFORCE(!result.canceled);
            ENFORCE(result.gs);
            config.initialized = true;
            return result;
        } else if (method == LSPMethod::Exit) {
            prodCategoryCounterInc("lsp.messages.processed", "exit");
            return LSPResult{move(gs), {}};
        } else if (method == LSPMethod::SorbetError) {
            auto &errorInfo = get<unique_ptr<SorbetErrorParams>>(params);
            if (errorInfo->code == (int)LSPErrorCodes::MethodNotFound) {
                // Not an error; we just don't care about this notification type (e.g. TextDocumentDidSave).
                logger->debug(errorInfo->message);
            } else {
                logger->error(errorInfo->message);
            }
            return LSPResult{move(gs), {}};
        } else if (method == LSPMethod::SorbetShowOperation) {
            // Forward to client. These are sent from the preprocessor.
            // TODO: Use a more efficient way to copy this object.
            output.write(LSPMessage::fromClient(msg.toJSON()));
            return LSPResult{move(gs), {}};
        }
    } else if (msg.isRequest()) {
        Timer timeit(logger, "request", {{"method", convertLSPMethodToString(method)}});
        auto &requestMessage = msg.asRequest();
        // asRequest() should guarantee the presence of an ID.
        ENFORCE(msg.id());
        auto id = *msg.id();
        auto response = make_unique<ResponseMessage>("2.0", id, method);
        if (msg.canceled) {
            prodCounterInc("lsp.messages.canceled");
            response->error = make_unique<ResponseError>((int)LSPErrorCodes::RequestCancelled, "Request was canceled");
            return LSPResult::make(move(gs), move(response));
        }

        auto &rawParams = requestMessage.params;
        if (method == LSPMethod::Initialize) {
            prodCategoryCounterInc("lsp.messages.processed", "initialize");
            const auto &params = get<unique_ptr<InitializeParams>>(rawParams);
            config.configure(*params);

            const auto &opts = config.opts;
            auto serverCap = make_unique<ServerCapabilities>();
            serverCap->textDocumentSync = TextDocumentSyncKind::Full;
            serverCap->definitionProvider = true;
            serverCap->typeDefinitionProvider = true;
            serverCap->documentSymbolProvider = opts.lspDocumentSymbolEnabled;
            serverCap->workspaceSymbolProvider = opts.lspWorkspaceSymbolsEnabled;
            serverCap->hoverProvider = true;
            serverCap->referencesProvider = true;

            if (opts.lspQuickFixEnabled) {
                auto codeActionProvider = make_unique<CodeActionOptions>();
                codeActionProvider->codeActionKinds = {CodeActionKind::Quickfix};
                serverCap->codeActionProvider = move(codeActionProvider);
            }

            if (opts.lspSignatureHelpEnabled) {
                auto sigHelpProvider = make_unique<SignatureHelpOptions>();
                sigHelpProvider->triggerCharacters = {"(", ","};
                serverCap->signatureHelpProvider = move(sigHelpProvider);
            }

            if (opts.lspAutocompleteEnabled) {
                auto completionProvider = make_unique<CompletionOptions>();
                completionProvider->triggerCharacters = {"."};
                serverCap->completionProvider = move(completionProvider);
            }

            response->result = make_unique<InitializeResult>(move(serverCap));
            return LSPResult::make(move(gs), move(response));
        } else if (method == LSPMethod::TextDocumentDocumentSymbol) {
            auto &params = get<unique_ptr<DocumentSymbolParams>>(rawParams);
            return handleTextDocumentDocumentSymbol(move(gs), id, *params);
        } else if (method == LSPMethod::WorkspaceSymbol) {
            auto &params = get<unique_ptr<WorkspaceSymbolParams>>(rawParams);
            return handleWorkspaceSymbols(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentDefinition) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            return handleTextDocumentDefinition(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentTypeDefinition) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            return handleTextDocumentTypeDefinition(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentHover) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            return handleTextDocumentHover(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentCompletion) {
            auto &params = get<unique_ptr<CompletionParams>>(rawParams);
            return handleTextDocumentCompletion(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentCodeAction) {
            auto &params = get<unique_ptr<CodeActionParams>>(rawParams);
            return handleTextDocumentCodeAction(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentSignatureHelp) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            return handleTextSignatureHelp(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentReferences) {
            auto &params = get<unique_ptr<ReferenceParams>>(rawParams);
            return handleTextDocumentReferences(move(gs), id, *params);
        } else if (method == LSPMethod::SorbetReadFile) {
            auto &params = get<unique_ptr<TextDocumentIdentifier>>(rawParams);
            auto fref = config.uri2FileRef(*gs, params->uri);
            if (fref.exists()) {
                response->result =
                    make_unique<TextDocumentItem>(params->uri, "ruby", 0, string(fref.data(*gs).source()));
            } else {
                response->error = make_unique<ResponseError>(
                    (int)LSPErrorCodes::InvalidParams,
                    fmt::format("Did not find file at uri {} in {}", params->uri, convertLSPMethodToString(method)));
            }
            return LSPResult::make(move(gs), move(response));
        } else if (method == LSPMethod::Shutdown) {
            prodCategoryCounterInc("lsp.messages.processed", "shutdown");
            response->result = JSONNullObject();
        } else if (method == LSPMethod::SorbetError) {
            auto &params = get<unique_ptr<SorbetErrorParams>>(rawParams);
            response->error = make_unique<ResponseError>(params->code, params->message);
        } else {
            // Method parsed, but isn't a request. Use SorbetError for `requestMethod`, as `method` isn't valid for a
            // response.
            response->requestMethod = LSPMethod::SorbetError;
            response->error = make_unique<ResponseError>(
                (int)LSPErrorCodes::MethodNotFound,
                fmt::format("Notification method sent as request: {}", convertLSPMethodToString(method)));
        }
        return LSPResult::make(move(gs), move(response));
    } else {
        logger->debug("Unable to process request {}; LSP message is not a request.", convertLSPMethodToString(method));
    }
    return LSPResult{move(gs), {}};
}
} // namespace sorbet::realmain::lsp
