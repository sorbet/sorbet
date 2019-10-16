#include "common/Timer.h"
#include "lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPResult LSPLoop::processRequest(const string &json) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(LSPMessage::fromClient(json));
    return LSPLoop::processRequests(move(messages));
}

LSPResult LSPLoop::processRequest(std::unique_ptr<LSPMessage> msg) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(move(msg));
    return processRequests(move(messages));
}

LSPResult LSPLoop::processRequests(vector<unique_ptr<LSPMessage>> messages) {
    QueueState state;
    absl::Mutex mutex;
    for (auto &message : messages) {
        preprocessor.preprocessAndEnqueue(state, move(message), mutex);
    }
    ENFORCE(state.paused == false, "__PAUSE__ not supported in single-threaded mode.");

    LSPResult rv{{}};
    for (auto &message : state.pendingRequests) {
        maybeStartCommitSlowPathEdit(*message);
        auto rslt = processRequestInternal(*message);
        rv.responses.insert(rv.responses.end(), make_move_iterator(rslt.responses.begin()),
                            make_move_iterator(rslt.responses.end()));
    }
    state.pendingRequests.clear();
    return rv;
}

LSPResult LSPLoop::processRequestInternal(unique_ptr<core::GlobalState> gs, const LSPMessage &msg) {
    // Note: Before this function runs, LSPPreprocessor has already early-rejected any invalid messages sent prior to
    // the initialization handshake. So, we know that `msg` is valid to process given the current state of the server.
    auto &logger = config->logger;
    // TODO(jvilk): Make Timer accept multiple FlowIds so we can show merged messages correctly.
    Timer timeit(logger, "process_request");
    const LSPMethod method = msg.method();
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
            // Only report stats if the edit was committed.
            if (!typechecker.typecheck(move(editParams->updates), output)) {
                prodCategoryCounterInc("lsp.messages.processed", "sorbet/workspaceEdit");
                prodCategoryCounterAdd("lsp.messages.processed", "sorbet/mergedEdits", merged);
            }
            return LSPResult();
        } else if (method == LSPMethod::Initialized) {
            prodCategoryCounterInc("lsp.messages.processed", "initialized");
            auto &initParams = get<unique_ptr<InitializedParams>>(params);
            auto &updates = initParams->updates;
            typechecker.initialize(move(updates), output);
            return LSPResult();
        } else if (method == LSPMethod::Exit) {
            prodCategoryCounterInc("lsp.messages.processed", "exit");
            return LSPResult();
        } else if (method == LSPMethod::SorbetError) {
            auto &errorInfo = get<unique_ptr<SorbetErrorParams>>(params);
            if (errorInfo->code == (int)LSPErrorCodes::MethodNotFound) {
                // Not an error; we just don't care about this notification type (e.g. TextDocumentDidSave).
                logger->debug(errorInfo->message);
            } else {
                logger->error(errorInfo->message);
            }
            return LSPResult();
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
            return LSPResult::make(move(response));
        }

        auto &rawParams = requestMessage.params;
        if (method == LSPMethod::Initialize) {
            prodCategoryCounterInc("lsp.messages.processed", "initialize");
            const auto &opts = config->opts;
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
            return LSPResult::make(move(response));
        } else if (method == LSPMethod::TextDocumentDocumentSymbol) {
            auto &params = get<unique_ptr<DocumentSymbolParams>>(rawParams);
            LSPResult result;
            typechecker.enterCriticalSection(
                [&](const auto &ops) -> void { result = handleTextDocumentDocumentSymbol(ops, id, *params); });
            return result;
        } else if (method == LSPMethod::WorkspaceSymbol) {
            auto &params = get<unique_ptr<WorkspaceSymbolParams>>(rawParams);
            LSPResult result;
            typechecker.enterCriticalSection(
                [&](const auto &ops) -> void { result = handleWorkspaceSymbols(ops, id, *params); });
            return result;
        } else if (method == LSPMethod::TextDocumentDefinition) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            LSPResult result;
            typechecker.enterCriticalSection(
                [&](const auto &ops) -> void { result = handleTextDocumentDefinition(ops, id, *params); });
            return result;
        } else if (method == LSPMethod::TextDocumentTypeDefinition) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            LSPResult result;
            typechecker.enterCriticalSection(
                [&](const auto &ops) -> void { result = handleTextDocumentTypeDefinition(ops, id, *params); });
            return result;
        } else if (method == LSPMethod::TextDocumentHover) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            LSPResult result;
            typechecker.enterCriticalSection(
                [&](const auto &ops) -> void { result = handleTextDocumentHover(ops, id, *params); });
            return result;
        } else if (method == LSPMethod::TextDocumentCompletion) {
            auto &params = get<unique_ptr<CompletionParams>>(rawParams);
            LSPResult result;
            typechecker.enterCriticalSection(
                [&](const auto &ops) -> void { result = handleTextDocumentCompletion(ops, id, *params); });
            return result;
        } else if (method == LSPMethod::TextDocumentCodeAction) {
            auto &params = get<unique_ptr<CodeActionParams>>(rawParams);
            LSPResult result;
            typechecker.enterCriticalSection(
                [&](const auto &ops) -> void { result = move(handleTextDocumentCodeAction(ops, id, *params)); });
            return result;
        } else if (method == LSPMethod::TextDocumentSignatureHelp) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            LSPResult result;
            typechecker.enterCriticalSection(
                [&](const auto &ops) -> void { result = move(handleTextSignatureHelp(ops, id, *params)); });
            return result;
        } else if (method == LSPMethod::TextDocumentReferences) {
            auto &params = get<unique_ptr<ReferenceParams>>(rawParams);
            LSPResult result;
            typechecker.enterCriticalSection(
                [&](const auto &ops) -> void { result = move(handleTextDocumentReferences(ops, id, *params)); });
            return result;
        } else if (method == LSPMethod::SorbetReadFile) {
            auto &params = get<unique_ptr<TextDocumentIdentifier>>(rawParams);
            typechecker.enterCriticalSection([&](auto &ops) -> void {
                auto fref = config->uri2FileRef(ops.gs, params->uri);
                if (fref.exists()) {
                    response->result =
                        make_unique<TextDocumentItem>(params->uri, "ruby", 0, string(fref.data(ops.gs).source()));
                } else {
                    response->error = make_unique<ResponseError>(
                        (int)LSPErrorCodes::InvalidParams, fmt::format("Did not find file at uri {} in {}", params->uri,
                                                                       convertLSPMethodToString(method)));
                }
            });
            return LSPResult::make(move(response));
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
        return LSPResult::make(move(response));
    } else {
        logger->debug("Unable to process request {}; LSP message is not a request.", convertLSPMethodToString(method));
    }
    return LSPResult();
}
} // namespace sorbet::realmain::lsp
