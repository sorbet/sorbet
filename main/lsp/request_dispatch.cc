#include "common/Timer.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

void LSPLoop::processRequest(const string &json) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(LSPMessage::fromClient(json));
    LSPLoop::processRequests(move(messages));
}

void LSPLoop::processRequest(std::unique_ptr<LSPMessage> msg) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(move(msg));
    processRequests(move(messages));
}

void LSPLoop::processRequests(vector<unique_ptr<LSPMessage>> messages) {
    QueueState state;
    absl::Mutex mutex;
    for (auto &message : messages) {
        preprocessor.preprocessAndEnqueue(state, move(message), mutex);
    }
    ENFORCE(state.paused == false, "__PAUSE__ not supported in single-threaded mode.");
    for (auto &message : state.pendingRequests) {
        maybeStartCommitSlowPathEdit(*message);
        processRequestInternal(*message);
    }
}

void LSPLoop::processRequestInternal(LSPMessage &msg) {
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
            shared_ptr<SorbetWorkspaceEditParams> editParams = move(get<unique_ptr<SorbetWorkspaceEditParams>>(params));
            const u4 end = editParams->updates.versionEnd;
            const u4 start = editParams->updates.versionStart;
            // Versions are sequential and wrap around. Use them to figure out how many edits are contained
            // within this update.
            const u4 merged = min(end - start, 0xFFFFFFFF - start + end);
            // Since std::function is copyable, we have to promote captured unique_ptrs into shared_ptrs.
            if (editParams->updates.canTakeFastPath) {
                // Fast path: Block until complete and preempt slow path if running.
                typecheckerCoord.syncRun(
                    [editParams, merged](LSPTypechecker &typechecker) -> void {
                        // Only report stats if the edit was committed.
                        if (!typechecker.typecheck(move(editParams->updates), true)) {
                            prodCategoryCounterInc("lsp.messages.processed", "sorbet/workspaceEdit");
                            prodCategoryCounterAdd("lsp.messages.processed", "sorbet/mergedEdits", merged);
                        }
                    },
                    true);
            } else {
                // Slow path. Run async.
                typecheckerCoord.asyncRun(
                    [editParams = move(editParams), merged](LSPTypechecker &typechecker) -> void {
                        // Only report stats if the edit was committed.
                        if (!typechecker.typecheck(move(editParams->updates), true)) {
                            prodCategoryCounterInc("lsp.messages.processed", "sorbet/workspaceEdit");
                            prodCategoryCounterAdd("lsp.messages.processed", "sorbet/mergedEdits", merged);
                        }
                    },
                    true);
            }
        } else if (method == LSPMethod::Initialized) {
            prodCategoryCounterInc("lsp.messages.processed", "initialized");
            auto &initParams = get<unique_ptr<InitializedParams>>(params);
            // TODO: Can we make this asynchronous?
            typecheckerCoord.syncRun(
                [&](LSPTypechecker &typechecker) -> void {
                    auto &updates = initParams->updates;
                    typechecker.initialize(move(updates));
                },
                true);
        } else if (method == LSPMethod::Exit) {
            prodCategoryCounterInc("lsp.messages.processed", "exit");
        } else if (method == LSPMethod::SorbetError) {
            auto &errorInfo = get<unique_ptr<SorbetErrorParams>>(params);
            if (errorInfo->code == (int)LSPErrorCodes::MethodNotFound) {
                // Not an error; we just don't care about this notification type (e.g. TextDocumentDidSave).
                logger->debug(errorInfo->message);
            } else {
                logger->error(errorInfo->message);
            }
        } else if (method == LSPMethod::SorbetFence) {
            // Ensure all prior messages have finished processing before sending response.
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void {
                    // Send the same fence back to acknowledge the fence.
                    // NOTE: Fence is a notification rather than a request so that we don't have to worry about clashes
                    // with client-chosen IDs when using fences internally.
                    auto response = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence,
                                                                     move(msg.asNotification().params));
                    config->output->write(move(response));
                },
                false);
        }
    } else if (msg.isRequest()) {
        Timer timeit(logger, "request", {{"method", convertLSPMethodToString(method)}});
        auto &requestMessage = msg.asRequest();
        // asRequest() should guarantee the presence of an ID.
        ENFORCE(msg.id());
        auto id = *msg.id();
        if (msg.canceled) {
            auto response = make_unique<ResponseMessage>("2.0", id, method);
            prodCounterInc("lsp.messages.canceled");
            response->error = make_unique<ResponseError>((int)LSPErrorCodes::RequestCancelled, "Request was canceled");
            config->output->write(move(response));
            return;
        }

        auto &rawParams = requestMessage.params;
        if (method == LSPMethod::Initialize) {
            prodCategoryCounterInc("lsp.messages.processed", "initialize");
            auto response = make_unique<ResponseMessage>("2.0", id, method);
            const auto &opts = config->opts;
            auto serverCap = make_unique<ServerCapabilities>();
            serverCap->textDocumentSync = TextDocumentSyncKind::Full;
            serverCap->definitionProvider = true;
            serverCap->typeDefinitionProvider = true;
            serverCap->documentSymbolProvider = opts.lspDocumentSymbolEnabled;
            serverCap->workspaceSymbolProvider = true;
            serverCap->documentHighlightProvider = opts.lspDocumentHighlightEnabled;
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

            auto completionProvider = make_unique<CompletionOptions>();
            completionProvider->triggerCharacters = {"."};
            serverCap->completionProvider = move(completionProvider);

            response->result = make_unique<InitializeResult>(move(serverCap));
            config->output->write(move(response));
        } else if (method == LSPMethod::TextDocumentDocumentHighlight) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            typecheckerCoord.syncRun(
                [&](auto &typechecker) -> void {
                    config->output->write(handleTextDocumentDocumentHighlight(typechecker, id, *params));
                },
                true);
        } else if (method == LSPMethod::TextDocumentDocumentSymbol) {
            auto &params = get<unique_ptr<DocumentSymbolParams>>(rawParams);
            typecheckerCoord.syncRun(
                [&](auto &typechecker) -> void {
                    config->output->write(handleTextDocumentDocumentSymbol(typechecker, id, *params));
                },
                true);
        } else if (method == LSPMethod::WorkspaceSymbol) {
            auto &params = get<unique_ptr<WorkspaceSymbolParams>>(rawParams);
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void { config->output->write(handleWorkspaceSymbols(tc, id, *params)); }, true);
        } else if (method == LSPMethod::TextDocumentDefinition) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void { config->output->write(handleTextDocumentDefinition(tc, id, *params)); }, true);
        } else if (method == LSPMethod::TextDocumentTypeDefinition) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void { config->output->write(handleTextDocumentTypeDefinition(tc, id, *params)); },
                true);
        } else if (method == LSPMethod::TextDocumentHover) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void { config->output->write(handleTextDocumentHover(tc, id, *params)); }, true);
        } else if (method == LSPMethod::TextDocumentCompletion) {
            auto &params = get<unique_ptr<CompletionParams>>(rawParams);
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void { config->output->write(handleTextDocumentCompletion(tc, id, *params)); }, true);
        } else if (method == LSPMethod::TextDocumentCodeAction) {
            auto &params = get<unique_ptr<CodeActionParams>>(rawParams);
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void { config->output->write(handleTextDocumentCodeAction(tc, id, *params)); }, true);
        } else if (method == LSPMethod::TextDocumentSignatureHelp) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void { config->output->write(handleTextSignatureHelp(tc, id, *params)); }, true);
        } else if (method == LSPMethod::TextDocumentReferences) {
            auto &params = get<unique_ptr<ReferenceParams>>(rawParams);
            // Do *not* preempt the slow path for reference requests. They really need to use all cores for performance.
            // TODO: Fix so this uses WorkerPool. Currently, it does not because I've disabled them for fast path.
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void { config->output->write(handleTextDocumentReferences(tc, id, *params)); }, false);
        } else if (method == LSPMethod::SorbetReadFile) {
            auto &params = get<unique_ptr<TextDocumentIdentifier>>(rawParams);
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void {
                    auto response = make_unique<ResponseMessage>("2.0", id, method);
                    auto fref = config->uri2FileRef(tc.state(), params->uri);
                    if (fref.exists()) {
                        response->result = make_unique<TextDocumentItem>(params->uri, "ruby", 0,
                                                                         string(fref.data(tc.state()).source()));
                    } else {
                        response->error =
                            make_unique<ResponseError>((int)LSPErrorCodes::InvalidParams,
                                                       fmt::format("Did not find file at uri {} in {}", params->uri,
                                                                   convertLSPMethodToString(method)));
                    }
                    config->output->write(move(response));
                },
                true);
        } else if (method == LSPMethod::Shutdown) {
            prodCategoryCounterInc("lsp.messages.processed", "shutdown");
            auto response = make_unique<ResponseMessage>("2.0", id, method);
            response->result = JSONNullObject();
            config->output->write(move(response));
        } else if (method == LSPMethod::SorbetError) {
            auto &params = get<unique_ptr<SorbetErrorParams>>(rawParams);
            auto response = make_unique<ResponseMessage>("2.0", id, method);
            response->error = make_unique<ResponseError>(params->code, params->message);
            config->output->write(move(response));
        } else if (method == LSPMethod::SorbetFence) {
            // Non-preempting sync run. When it runs, all prior messages have finished processing.
            typecheckerCoord.syncRun(
                [&](auto &tc) -> void {
                    // Inform client that all messages have finished processing.
                    auto response = make_unique<ResponseMessage>("2.0", id, method);
                    config->output->write(move(response));
                },
                false);
        } else {
            auto response = make_unique<ResponseMessage>("2.0", id, method);
            // Method parsed, but isn't a request. Use SorbetError for `requestMethod`, as `method` isn't valid for a
            // response.
            response->requestMethod = LSPMethod::SorbetError;
            response->error = make_unique<ResponseError>(
                (int)LSPErrorCodes::MethodNotFound,
                fmt::format("Notification method sent as request: {}", convertLSPMethodToString(method)));
            config->output->write(move(response));
        }
    } else {
        logger->debug("Unable to process request {}; LSP message is not a request.", convertLSPMethodToString(method));
    }
}
} // namespace sorbet::realmain::lsp
