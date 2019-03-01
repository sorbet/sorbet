#include "absl/strings/match.h"
#include "common/Timer.h"
#include "lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

unique_ptr<core::GlobalState> LSPLoop::processRequest(unique_ptr<core::GlobalState> gs, const string &json) {
    return LSPLoop::processRequest(move(gs), LSPMessage(alloc, json));
}

unique_ptr<core::GlobalState> LSPLoop::processRequest(unique_ptr<core::GlobalState> gs, const LSPMessage &msg) {
    auto id = msg.id();
    try {
        return processRequestInternal(move(gs), msg);
    } catch (const DeserializationError &e) {
        if (id.has_value()) {
            sendError(*id, (int)LSPErrorCodes::InvalidParams, e.what());
        }
        // Run the slow path so that the caller still has a GlobalState.
        return runSlowPath({}).gs;
    }
}

unique_ptr<core::GlobalState> LSPLoop::processRequests(unique_ptr<core::GlobalState> gs,
                                                       vector<unique_ptr<LSPMessage>> messages) {
    int requestCounter = 0;
    LSPLoop::QueueState state{{}, false, false};
    for (auto &message : messages) {
        preprocessRequest(alloc, logger, state, move(message), requestCounter++);
    }
    ENFORCE(state.paused == false, "__PAUSE__ not supported in single-threaded mode.");

    for (auto &message : state.pendingRequests) {
        gs = processRequest(move(gs), *message);
    }
    return gs;
}

unique_ptr<core::GlobalState> LSPLoop::processRequestInternal(unique_ptr<core::GlobalState> gs, const LSPMessage &msg) {
    if (handleReplies(msg)) {
        return gs;
    }

    const LSPMethod method = LSPMethod::getByName(msg.method());

    ENFORCE(method.kind == LSPMethod::Kind::ClientInitiated || method.kind == LSPMethod::Kind::Both);

    if (!ensureInitialized(method, msg, gs)) {
        return gs;
    }
    if (method.isNotification) {
        logger->debug("Processing notification {} ", method.name);
        if (method == LSPMethod::TextDocumentDidChange()) {
            prodCategoryCounterInc("lsp.requests.processed", "textDocument.didChange");
            Timer timeit(logger, "text_document_did_change");
            vector<shared_ptr<core::File>> files;
            auto edits = DidChangeTextDocumentParams::fromJSONValue(alloc, msg.params(), "root.params");

            string_view uri = edits->textDocument->uri;
            // TODO: if this is ever updated to support diffs, be aware that the coordinator thread should be
            // taught about it too: it merges consecutive TextDocumentDidChange
            if (absl::StartsWith(uri, rootUri)) {
                auto currentFileRef = initialGS->findFileByPath(remoteName2Local(uri));
                unique_ptr<core::File> file;
                if (currentFileRef.exists()) {
                    file = make_unique<core::File>(string(currentFileRef.data(*initialGS).path()),
                                                   string(currentFileRef.data(*initialGS).source()),
                                                   core::File::Type::Normal);
                } else {
                    file = make_unique<core::File>(remoteName2Local(uri), "", core::File::Type::Normal);
                }

                for (auto &change : edits->contentChanges) {
                    if (change->range) {
                        auto &range = *change->range;
                        // incremental update
                        auto old = move(file);
                        string oldContent = string(old->source());
                        core::Loc::Detail start, end;
                        start.line = range->start->line + 1;
                        start.column = range->start->character + 1;
                        end.line = range->end->line + 1;
                        end.column = range->end->character + 1;
                        auto startOffset = core::Loc::pos2Offset(*old, start);
                        auto endOffset = core::Loc::pos2Offset(*old, end);
                        string newContent = oldContent.replace(startOffset, endOffset - startOffset, change->text);
                        file = make_unique<core::File>(string(old->path()), move(newContent), core::File::Type::Normal);
                    } else {
                        // replace
                        auto old = move(file);
                        file =
                            make_unique<core::File>(string(old->path()), move(change->text), core::File::Type::Normal);
                    }
                }

                logger->trace("Updating {} to have the following contents: {}", remoteName2Local(uri), file->source());

                files.emplace_back(move(file));

                return pushDiagnostics(tryFastPath(move(gs), files));
            }
        }
        if (method == LSPMethod::TextDocumentDidOpen()) {
            prodCategoryCounterInc("requests.processed", "textDocument.didOpen");
            Timer timeit(logger, "text_document_did_open");
            vector<shared_ptr<core::File>> files;
            auto edits = DidOpenTextDocumentParams::fromJSONValue(alloc, msg.params(), "root.params");
            string_view uri = edits->textDocument->uri;
            if (absl::StartsWith(uri, rootUri)) {
                files.emplace_back(make_shared<core::File>(remoteName2Local(uri), move(edits->textDocument->text),
                                                           core::File::Type::Normal));

                return pushDiagnostics(tryFastPath(move(gs), files));
            }
        }
        if (method == LSPMethod::Initialized()) {
            prodCategoryCounterInc("requests.processed", "initialized");
            // initialize ourselves
            {
                Timer timeit(logger, "initial_index");
                reIndexFromFileSystem();
                vector<shared_ptr<core::File>> changedFiles;
                auto newGs = pushDiagnostics(runSlowPath(move(changedFiles)));
                ENFORCE(newGs);
                if (!disableFastPath) {
                    this->globalStateHashes = computeStateHashes(newGs->getFiles());
                }
                return newGs;
            }
        }
        if (method == LSPMethod::Exit()) {
            return gs;
        }
    } else if (msg.isRequest()) {
        logger->debug("Processing request {}", method.name);
        auto &requestMessage = msg.asRequest();
        auto id = requestMessage.id;

        // LSP hack: We add the property 'canceled' to requests that were canceled before
        // they hit processRequest.
        if (requestMessage.canceled.value_or(false)) {
            prodCounterInc("lsp.requests.canceled");
            sendError(id, (int)LSPErrorCodes::RequestCancelled, "Request was canceled");
            return gs;
        }

        if (!requestMessage.params) {
            sendError(id, (int)LSPErrorCodes::InternalError, "Expected parameters, but found none.");
            return gs;
        }
        auto &rawParams = *requestMessage.params;

        if (method == LSPMethod::Initialize()) {
            prodCategoryCounterInc("lsp.requests.processed", "initialize");
            auto params = InitializeParams::fromJSONValue(alloc, *rawParams, "root.params");
            if (auto rootUriString = get_if<string>(&params->rootUri)) {
                rootUri = *rootUriString;
            }
            clientCompletionItemSnippetSupport = false;
            if (params->capabilities->textDocument) {
                auto &textDocument = *params->capabilities->textDocument;
                if (textDocument->completion) {
                    auto &completion = *textDocument->completion;
                    if (completion->completionItem) {
                        clientCompletionItemSnippetSupport =
                            (*completion->completionItem)->snippetSupport.value_or(false);
                    }
                }
            }

            auto serverCap = make_unique<ServerCapabilities>();
            serverCap->textDocumentSync = TextDocumentSyncKind::Full;
            serverCap->definitionProvider = true;
            serverCap->documentSymbolProvider = true;
            serverCap->workspaceSymbolProvider = true;
            serverCap->hoverProvider = true;
            serverCap->referencesProvider = true;

            auto sigHelpProvider = make_unique<SignatureHelpOptions>();
            sigHelpProvider->triggerCharacters = {"(", ","};
            serverCap->signatureHelpProvider = move(sigHelpProvider);

            auto completionProvider = make_unique<CompletionOptions>();
            completionProvider->triggerCharacters = {"."};
            serverCap->completionProvider = move(completionProvider);

            sendResponse(id, InitializeResult(move(serverCap)));
        } else if (method == LSPMethod::Shutdown()) {
            prodCategoryCounterInc("lsp.requests.processed", "shutdown");
            sendNullResponse(id);
        } else if (method == LSPMethod::TextDocumentDocumentSymbol()) {
            auto params = DocumentSymbolParams::fromJSONValue(alloc, *rawParams);
            return handleTextDocumentDocumentSymbol(move(gs), id, *params);
        } else if (method == LSPMethod::WorkspaceSymbols()) {
            auto params = WorkspaceSymbolParams::fromJSONValue(alloc, *rawParams);
            return handleWorkspaceSymbols(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentDefinition()) {
            auto params = TextDocumentPositionParams::fromJSONValue(alloc, *rawParams);
            return handleTextDocumentDefinition(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentHover()) {
            auto params = TextDocumentPositionParams::fromJSONValue(alloc, *rawParams);
            return handleTextDocumentHover(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentCompletion()) {
            auto params = CompletionParams::fromJSONValue(alloc, *rawParams);
            return handleTextDocumentCompletion(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentSignatureHelp()) {
            auto params = TextDocumentPositionParams::fromJSONValue(alloc, *rawParams);
            return handleTextSignatureHelp(move(gs), id, *params);
        } else if (method == LSPMethod::TextDocumentRefernces()) {
            auto params = ReferenceParams::fromJSONValue(alloc, *rawParams);
            return handleTextDocumentReferences(move(gs), id, *params);
        } else {
            ENFORCE(!method.isSupported, "failing a supported method");
            sendError(id, (int)LSPErrorCodes::MethodNotFound, fmt::format("Unknown method: {}", method.name));
        }
    } else {
        logger->debug("Unable to process request {}; LSP message is not a request.", method.name);
    }
    return gs;
}
} // namespace sorbet::realmain::lsp
