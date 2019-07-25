#include "common/Timer.h"
#include "lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPResult LSPLoop::processRequest(unique_ptr<core::GlobalState> gs, const string &json) {
    unique_ptr<LSPMessage> msg = LSPMessage::fromClient(json);
    return LSPLoop::processRequest(move(gs), *msg);
}

LSPResult LSPLoop::processRequest(unique_ptr<core::GlobalState> gs, const LSPMessage &msg) {
    // TODO(jvilk): Make Timer accept multiple FlowIds so we can show merged messages correctly.
    Timer timeit(logger, "process_request");
    return processRequestInternal(move(gs), msg);
}

LSPResult LSPLoop::processRequests(unique_ptr<core::GlobalState> gs, vector<unique_ptr<LSPMessage>> messages) {
    LSPLoop::QueueState state{{}, false, false, 0};
    for (auto &message : messages) {
        enqueueRequest(logger, state, move(message));
    }
    ENFORCE(state.paused == false, "__PAUSE__ not supported in single-threaded mode.");

    LSPResult rv{move(gs), {}};
    for (auto &message : state.pendingRequests) {
        auto rslt = processRequest(move(rv.gs), *message);
        rv.gs = move(rslt.gs);
        rv.responses.insert(rv.responses.end(), make_move_iterator(rslt.responses.begin()),
                            make_move_iterator(rslt.responses.end()));
    }
    return rv;
}

LSPResult LSPLoop::processRequestInternal(unique_ptr<core::GlobalState> gs, const LSPMessage &msg) {
    const LSPMethod method = msg.method();

    if (!ensureInitialized(method, msg, gs)) {
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
        auto &params = msg.asNotification().params;
        if (method == LSPMethod::TextDocumentDidChange) {
            prodCategoryCounterInc("lsp.messages.processed", "textDocument.didChange");
            return handleSorbetWorkspaceEdit(move(gs), *get<unique_ptr<DidChangeTextDocumentParams>>(params));
        }
        if (method == LSPMethod::TextDocumentDidOpen) {
            prodCategoryCounterInc("lsp.messages.processed", "textDocument.didOpen");
            return handleSorbetWorkspaceEdit(move(gs), *get<unique_ptr<DidOpenTextDocumentParams>>(params));
        }
        if (method == LSPMethod::TextDocumentDidClose) {
            prodCategoryCounterInc("lsp.messages.processed", "textDocument.didClose");
            return handleSorbetWorkspaceEdit(move(gs), *get<unique_ptr<DidCloseTextDocumentParams>>(params));
        }
        if (method == LSPMethod::SorbetWatchmanFileChange) {
            prodCategoryCounterInc("lsp.messages.processed", "sorbet/watchmanFileChange");
            return handleSorbetWorkspaceEdit(move(gs), *get<unique_ptr<WatchmanQueryResponse>>(params));
        }
        if (method == LSPMethod::SorbetWorkspaceEdit) {
            // Note: We increment `lsp.messages.processed` when the original requests were merged into this one.
            auto &editParams = get<unique_ptr<SorbetWorkspaceEditParams>>(params);
            auto &edits = editParams->changes;
            auto &counts = editParams->counts;
            prodCategoryCounterAdd("lsp.messages.processed", "textDocument.didChange", counts->textDocumentDidChange);
            prodCategoryCounterAdd("lsp.messages.processed", "textDocument.didOpen", counts->textDocumentDidOpen);
            prodCategoryCounterAdd("lsp.messages.processed", "textDocument.didClose", counts->textDocumentDidClose);
            prodCategoryCounterAdd("lsp.messages.processed", "sorbet/watchmanFileChange",
                                   counts->sorbetWatchmanFileChange);
            // Number of messages merged together into a workspace edit.
            prodCounterAdd("lsp.messages.merged", (counts->textDocumentDidChange + counts->textDocumentDidOpen +
                                                   counts->textDocumentDidClose + counts->sorbetWatchmanFileChange) -
                                                      1);
            return handleSorbetWorkspaceEdits(move(gs), edits);
        }
        if (method == LSPMethod::Initialized) {
            prodCategoryCounterInc("lsp.messages.processed", "initialized");
            Timer timeit(logger, "initial_index");
            reIndexFromFileSystem();
            LSPResult result = pushDiagnostics(runSlowPath());
            ENFORCE(result.gs);
            if (!disableFastPath) {
                ShowOperation stateHashOp(*this, "GlobalStateHash", "Finishing initialization...");
                this->globalStateHashes = computeStateHashes(result.gs->getFiles());
            }
            initialized = true;
            return result;
        }
        if (method == LSPMethod::Exit) {
            prodCategoryCounterInc("lsp.messages.processed", "exit");
            return LSPResult{move(gs), {}};
        }
        if (method == LSPMethod::SorbetError) {
            auto &errorInfo = get<unique_ptr<SorbetErrorParams>>(params);
            if (errorInfo->code == (int)LSPErrorCodes::MethodNotFound) {
                // Not an error; we just don't care about this notification type (e.g. TextDocumentDidSave).
                logger->debug(errorInfo->message);
            } else {
                logger->error(errorInfo->message);
            }
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
            auto &params = get<unique_ptr<InitializeParams>>(rawParams);
            if (auto rootUriString = get_if<string>(&params->rootUri)) {
                rootUri = *rootUriString;
            }
            clientCompletionItemSnippetSupport = false;
            clientHoverMarkupKind = MarkupKind::Plaintext;
            if (params->capabilities->textDocument) {
                auto &textDocument = *params->capabilities->textDocument;
                if (textDocument->completion) {
                    auto &completion = *textDocument->completion;
                    if (completion->completionItem) {
                        clientCompletionItemSnippetSupport =
                            (*completion->completionItem)->snippetSupport.value_or(false);
                    }
                }
                if (textDocument->hover) {
                    auto &hover = *textDocument->hover;
                    if (hover->contentFormat) {
                        auto &contentFormat = *hover->contentFormat;
                        clientHoverMarkupKind = find(contentFormat.begin(), contentFormat.end(),
                                                     MarkupKind::Markdown) != contentFormat.end()
                                                    ? MarkupKind::Markdown
                                                    : MarkupKind::Plaintext;
                    }
                }
            }

            if (params->initializationOptions) {
                auto &initOptions = *params->initializationOptions;
                enableOperationNotifications = initOptions->supportsOperationNotifications.value_or(false);
                enableTypecheckInfo = initOptions->enableTypecheckInfo.value_or(false);
            }

            auto serverCap = make_unique<ServerCapabilities>();
            serverCap->textDocumentSync = TextDocumentSyncKind::Full;
            serverCap->definitionProvider = opts.lspGoToDefinitionEnabled;
            serverCap->documentSymbolProvider = opts.lspDocumentSymbolEnabled;
            serverCap->workspaceSymbolProvider = opts.lspWorkspaceSymbolsEnabled;
            serverCap->hoverProvider = true;
            serverCap->referencesProvider = opts.lspFindReferencesEnabled;

            auto codeActionProvider = make_unique<CodeActionOptions>();
            codeActionProvider->codeActionKinds = {CodeActionKind::Quickfix};
            serverCap->codeActionProvider = move(codeActionProvider);

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
        } else if (method == LSPMethod::TextDocumentHover) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            return handleTextDocumentHover(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentCompletion) {
            auto &params = get<unique_ptr<CompletionParams>>(rawParams);
            return handleTextDocumentCompletion(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentSignatureHelp) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            return handleTextSignatureHelp(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentReferences) {
            auto &params = get<unique_ptr<ReferenceParams>>(rawParams);
            return handleTextDocumentReferences(move(gs), id, *params);
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
