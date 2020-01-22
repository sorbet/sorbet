#include "common/Timer.h"
#include "core/Unfreeze.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/lsp.h"
#include "main/lsp/requests/requests.h"
#include "main/pipeline/pipeline.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
class SorbetFenceTask final : public LSPTask {
    int id;

public:
    SorbetFenceTask(const LSPConfiguration &config, int id) : LSPTask(config, true), id(id) {}
    void run(LSPTypecheckerDelegate &tc) override {
        // Send the same fence back to acknowledge the fence.
        // NOTE: Fence is a notification rather than a request so that we don't have to worry about clashes with
        // client-chosen IDs when using fences internally.
        auto response = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, id);
        config.output->write(move(response));
    }
};

class FastPathTypecheckTask final : public LSPTask {
    LSPFileUpdates updates;
    unique_ptr<Timer> timeUntilRun;

public:
    FastPathTypecheckTask(const LSPConfiguration &config, LSPFileUpdates updates)
        : LSPTask(config, false), updates(move(updates)) {
        if (updates.canceledSlowPath) {
            // Measure the time it takes to cancel slow path and run this task
            timeUntilRun = make_unique<Timer>(*config.logger, "latency.cancel_slow_path");
        }
    }
    void run(LSPTypecheckerDelegate &tc) override {
        // Trigger destructor of Timer, which reports metric.
        timeUntilRun = nullptr;
        tc.typecheckOnFastPath(move(updates));
        prodCategoryCounterInc("lsp.messages.processed", "sorbet/workspaceEdit");
        prodCategoryCounterAdd("lsp.messages.processed", "sorbet/mergedEdits", updates.editCount - 1);
    }
};

class IndexAndHashWorkspaceTask final : public LSPTask {
    unique_ptr<core::GlobalState> &initialGS;
    vector<core::FileHash> &globalStateHashes;
    LSPFileUpdates &updates;

    vector<ast::ParsedFile> indexFromFileSystem(WorkerPool &workers, std::unique_ptr<KeyValueStore> kvstore) {
        vector<ast::ParsedFile> indexed;
        {
            Timer timeit(config.logger, "reIndexFromFileSystem");
            vector<core::FileRef> inputFiles = pipeline::reserveFiles(initialGS, config.opts.inputFileNames);
            for (auto &t : pipeline::index(initialGS, inputFiles, config.opts, workers, kvstore)) {
                int id = t.file.id();
                if (id >= indexed.size()) {
                    indexed.resize(id + 1);
                }
                indexed[id] = move(t);
            }
            // Clear error queue.
            // (Note: Flushing is disabled in LSP mode, so we have to drain.)
            initialGS->errorQueue->drainWithQueryResponses();
        }

        // When inputFileNames is 0 (as in tests), indexed ends up being size 0 because we don't index payload files.
        // At the same time, we expect indexed to be the same size as GlobalStateHash, which _does_ have payload files.
        // Resize the indexed array accordingly.
        if (indexed.size() < initialGS->getFiles().size()) {
            indexed.resize(initialGS->getFiles().size());
        }

        return indexed;
    }

public:
    // Outputs:
    // - initialGS that has indexed the workspace.
    // - globalStateHashes for every file in workspace.
    // - updates containing the state needed to pass to the typechecker for initialization.
    IndexAndHashWorkspaceTask(const LSPConfiguration &config, unique_ptr<core::GlobalState> &initialGS,
                              vector<core::FileHash> &globalStateHashes, LSPFileUpdates &updates)
        : LSPTask(config, true), initialGS(initialGS), globalStateHashes(globalStateHashes), updates(updates) {}

    void run(LSPTypecheckerDelegate &tc) override {
        auto &workers = tc.workers;
        // Temporarily replace error queue, as it asserts that the same thread that created it uses it and we're
        // going to use it on typechecker thread for this one operation.
        auto savedErrorQueue = initialGS->errorQueue;
        initialGS->errorQueue = make_shared<core::ErrorQueue>(savedErrorQueue->logger, savedErrorQueue->tracer);
        initialGS->errorQueue->ignoreFlushes = true;
        // Enforce that this is only run once.
        ENFORCE(globalStateHashes.empty());

        vector<ast::ParsedFile> indexed;
        {
            Timer timeit(config.logger, "initial_index");
            ShowOperation op(config, "Indexing", "Indexing files...");
            indexed = indexFromFileSystem(workers, nullptr /* TODO(jvilk): Thread through kvstore */);
            globalStateHashes = LSPTypechecker::computeFileHashes(config, initialGS->getFiles(), workers);
        }
        updates.epoch = 0;
        updates.canTakeFastPath = false;
        // *Copy* global state hashes; both LSPLoop and LSPTypechecker need a copy (LSPLoop to figure out
        // cancelation, LSPTypechecker to run queries)
        updates.updatedFileHashes = globalStateHashes;
        updates.updatedFileIndexes = move(indexed);
        updates.updatedGS = initialGS->deepCopy();

        // Restore error queue, as initialGS will be used on the LSPLoop thread from now on.
        initialGS->errorQueue = move(savedErrorQueue);
    }
};

const core::FileHash &findHash(int id, const vector<core::FileHash> &globalStateHashes,
                               const UnorderedMap<int, core::FileHash> &overriddingStateHashes) {
    const auto it = overriddingStateHashes.find(id);
    if (it == overriddingStateHashes.end()) {
        return globalStateHashes[id];
    }
    return it->second;
}

bool canTakeFastPath(
    const core::GlobalState &gs, const LSPConfiguration &config, const vector<core::FileHash> &globalStateHashes,
    const LSPFileUpdates &updates,
    const UnorderedMap<int, core::FileHash> &overriddingStateHashes = UnorderedMap<int, core::FileHash>()) {
    Timer timeit(config.logger, "fast_path_decision");
    auto &logger = *config.logger;
    if (config.disableFastPath) {
        logger.debug("Taking slow path because fast path is disabled.");
        prodCategoryCounterInc("lsp.slow_path_reason", "fast_path_disabled");
        return false;
    }
    // Path taken after the first time an update has been encountered. Hack since we can't roll back new files just yet.
    if (updates.hasNewFiles) {
        logger.debug("Taking slow path because update has a new file");
        prodCategoryCounterInc("lsp.slow_path_reason", "new_file");
        return false;
    }
    const auto &hashes = updates.updatedFileHashes;
    auto &changedFiles = updates.updatedFiles;
    logger.debug("Trying to see if fast path is available after {} file changes", changedFiles.size());

    ENFORCE(changedFiles.size() == hashes.size());
    int i = -1;
    {
        for (auto &f : changedFiles) {
            ++i;
            auto fref = gs.findFileByPath(f->path());
            if (!fref.exists()) {
                logger.debug("Taking slow path because {} is a new file", f->path());
                prodCategoryCounterInc("lsp.slow_path_reason", "new_file");
                return false;
            } else {
                auto &oldHash = findHash(fref.id(), globalStateHashes, overriddingStateHashes);
                ENFORCE(oldHash.definitions.hierarchyHash != core::GlobalStateHash::HASH_STATE_NOT_COMPUTED);
                if (hashes[i].definitions.hierarchyHash == core::GlobalStateHash::HASH_STATE_INVALID) {
                    logger.debug("Taking slow path because {} has a syntax error", f->path());
                    prodCategoryCounterInc("lsp.slow_path_reason", "syntax_error");
                    return false;
                } else if (hashes[i].definitions.hierarchyHash != core::GlobalStateHash::HASH_STATE_INVALID &&
                           hashes[i].definitions.hierarchyHash != oldHash.definitions.hierarchyHash) {
                    logger.debug("Taking slow path because {} has changed definitions", f->path());
                    prodCategoryCounterInc("lsp.slow_path_reason", "changed_definition");
                    return false;
                }
            }
        }
    }
    logger.debug("Taking fast path");
    return true;
}

UnorderedMap<int, core::FileHash> mergeEvictions(const UnorderedMap<int, core::FileHash> &olderEvictions,
                                                 const UnorderedMap<int, core::FileHash> &newerEvictions) {
    // For evictions, which are needed for emulating an older `globalStateHashes`, we keep the oldest.
    UnorderedMap<int, core::FileHash> combinedEvictions = olderEvictions;
    for (auto &e : newerEvictions) {
        if (!combinedEvictions.contains(e.first)) {
            combinedEvictions[e.first] = e.second;
        }
    }
    return combinedEvictions;
}

} // namespace

LSPFileUpdates LSPLoop::commitEdit(SorbetWorkspaceEditParams &edit) {
    LSPFileUpdates update;
    update.epoch = edit.epoch;
    update.editCount = edit.mergeCount + 1;
    update.updatedFileHashes = LSPTypechecker::computeFileHashes(*config, edit.updates, *emptyWorkers);
    update.updatedFiles = move(edit.updates);
    update.canTakeFastPath = canTakeFastPath(*initialGS, *config, globalStateHashes, update);
    update.cancellationExpected = edit.sorbetCancellationExpected;
    update.preemptionsExpected = edit.sorbetPreemptionsExpected;

    // Update globalStateHashes. Keep track of file IDs for these files, along with old hashes for these files.
    vector<core::FileRef> frefs;
    UnorderedMap<int, core::FileHash> evictedHashes;
    {
        ENFORCE(update.updatedFiles.size() == update.updatedFileHashes.size());
        core::UnfreezeFileTable fileTableAccess(*initialGS);
        int i = -1;
        for (auto &file : update.updatedFiles) {
            auto fref = initialGS->findFileByPath(file->path());
            i++;
            if (fref.exists()) {
                ENFORCE(fref.id() < globalStateHashes.size());
                initialGS = core::GlobalState::replaceFile(move(initialGS), fref, file);
            } else {
                // This file update adds a new file to GlobalState.
                update.hasNewFiles = true;
                fref = initialGS->enterFile(file);
                fref.data(*initialGS).strictLevel = pipeline::decideStrictLevel(*initialGS, fref, config->opts);
                if (fref.id() >= globalStateHashes.size()) {
                    globalStateHashes.resize(fref.id() + 1);
                }
            }
            evictedHashes[fref.id()] = move(globalStateHashes[fref.id()]);
            globalStateHashes[fref.id()] = update.updatedFileHashes[i];
            frefs.push_back(fref);
        }
    }

    // Index changes in initialGS. pipeline::index sorts output by file id, but we need to reorder to match the order of
    // other fields.
    UnorderedMap<u2, int> fileToPos;
    {
        int i = -1;
        for (auto fref : frefs) {
            // We should have ensured before reaching here that there are no duplicates.
            ENFORCE(!fileToPos.contains(fref.id()));
            i++;
            fileToPos[fref.id()] = i;
        }
    }

    {
        auto trees = pipeline::index(initialGS, frefs, config->opts, *emptyWorkers, kvstore);
        initialGS->errorQueue->drainWithQueryResponses(); // Clear error queue; we don't care about errors here.
        update.updatedFileIndexes.resize(trees.size());
        for (auto &ast : trees) {
            const int i = fileToPos[ast.file.id()];
            update.updatedFileIndexes[i] = move(ast);
        }
    }

    auto runningSlowPath = initialGS->epochManager->getStatus();
    if (runningSlowPath.slowPathRunning) {
        // A cancelable slow path is currently running. Before running deepCopy(), check if we can cancel -- we might be
        // able to avoid it.
        // Invariant: `pendingTypecheckUpdates` should contain the edits currently being typechecked on the slow path.
        // runningSlowPath.epoch should be in the interval (pendingTypecheckUpdates.epoch - editCount,
        // pendingTypecheckUpdates.epoch]
        ENFORCE(runningSlowPath.epoch <= pendingTypecheckUpdates.epoch);
        ENFORCE(runningSlowPath.epoch > (pendingTypecheckUpdates.epoch - pendingTypecheckUpdates.editCount));
        auto merged = update.copy();
        merged.mergeOlder(pendingTypecheckUpdates);
        auto mergedEvictions = mergeEvictions(pendingTypecheckEvictedStateHashes, evictedHashes);
        merged.canTakeFastPath = canTakeFastPath(*initialGS, *config, globalStateHashes, merged, mergedEvictions);
        // Cancel if old + new takes fast path, or if the new update will take the slow path anyway.
        if ((merged.canTakeFastPath || !update.canTakeFastPath) &&
            initialGS->epochManager->tryCancelSlowPath(merged.epoch)) {
            // Cancelation succeeded! Use `merged` as the update.
            update = move(merged);
            update.canceledSlowPath = true;
            evictedHashes = std::move(mergedEvictions);
        }
    }

    ENFORCE(update.updatedFiles.size() == update.updatedFileHashes.size());
    ENFORCE(update.updatedFiles.size() == update.updatedFileIndexes.size());

    // Completely replace `pendingTypecheckUpdates` if this was a slow path update.
    if (!update.canTakeFastPath) {
        update.updatedGS = initialGS->deepCopy();
        pendingTypecheckUpdates = update.copy();
        pendingTypecheckEvictedStateHashes = std::move(evictedHashes);
    } else {
        // Edit takes the fast path. Merge with this edit so we can reverse it if the slow path gets canceled.
        auto merged = update.copy();
        merged.mergeOlder(pendingTypecheckUpdates);
        auto mergedEvictions = mergeEvictions(pendingTypecheckEvictedStateHashes, evictedHashes);
        pendingTypecheckUpdates = move(merged);
        pendingTypecheckEvictedStateHashes = std::move(mergedEvictions);
    }
    // Don't copy over these (test-only) properties, as they only apply to the original request.
    pendingTypecheckUpdates.cancellationExpected = false;
    pendingTypecheckUpdates.preemptionsExpected = 0;

    return update;
}

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
            auto &editParams = get<unique_ptr<SorbetWorkspaceEditParams>>(params);
            auto updates = commitEdit(*editParams);
            if (updates.canTakeFastPath) {
                // The fast path is blocking.
                typecheckerCoord.syncRun(make_unique<FastPathTypecheckTask>(*config, move(updates)));
            } else {
                // The slow path is non-blocking.
                typecheckerCoord.typecheckOnSlowPath(move(updates));
            }
        } else if (method == LSPMethod::Initialized) {
            prodCategoryCounterInc("lsp.messages.processed", "initialized");
            LSPFileUpdates updates;
            // Note: Captures all arguments by reference, and updates `initialGS`, `globalStateHashes`, and `updates`.
            typecheckerCoord.syncRun(
                make_unique<IndexAndHashWorkspaceTask>(*config, initialGS, globalStateHashes, updates));
            // Send processed updates to typechecker (this will eventually be non-blocking w/ preemption).
            typecheckerCoord.initialize(move(updates));
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
            typecheckerCoord.syncRun(make_unique<SorbetFenceTask>(*config, get<int>(msg.asNotification().params)));
        }
    } else if (msg.isRequest()) {
        Timer timeit(logger, "request", {{"method", convertLSPMethodToString(method)}});
        auto &requestMessage = msg.asRequest();
        // asRequest() should guarantee the presence of an ID.
        ENFORCE(msg.id());
        auto id = *msg.id();
        if (msg.isCanceled()) {
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
            typecheckerCoord.syncRun(make_unique<DocumentHighlightTask>(*config, id, move(params)));
        } else if (method == LSPMethod::TextDocumentDocumentSymbol) {
            auto &params = get<unique_ptr<DocumentSymbolParams>>(rawParams);
            typecheckerCoord.syncRun(make_unique<DocumentSymbolTask>(*config, id, move(params)));
        } else if (method == LSPMethod::WorkspaceSymbol) {
            auto &params = get<unique_ptr<WorkspaceSymbolParams>>(rawParams);
            typecheckerCoord.syncRun(make_unique<WorkspaceSymbolsTask>(*config, id, move(params)));
        } else if (method == LSPMethod::TextDocumentDefinition) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            typecheckerCoord.syncRun(make_unique<DefinitionTask>(*config, id, move(params)));
        } else if (method == LSPMethod::TextDocumentTypeDefinition) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            typecheckerCoord.syncRun(make_unique<TypeDefinitionTask>(*config, id, move(params)));
        } else if (method == LSPMethod::TextDocumentHover) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            typecheckerCoord.syncRun(make_unique<HoverTask>(*config, id, move(params)));
        } else if (method == LSPMethod::TextDocumentCompletion) {
            auto &params = get<unique_ptr<CompletionParams>>(rawParams);
            typecheckerCoord.syncRun(make_unique<CompletionTask>(*config, id, move(params)));
        } else if (method == LSPMethod::TextDocumentCodeAction) {
            auto &params = get<unique_ptr<CodeActionParams>>(rawParams);
            typecheckerCoord.syncRun(make_unique<CodeActionTask>(*config, id, move(params)));
        } else if (method == LSPMethod::TextDocumentSignatureHelp) {
            auto &params = get<unique_ptr<TextDocumentPositionParams>>(rawParams);
            typecheckerCoord.syncRun(make_unique<SignatureHelpTask>(*config, id, move(params)));
        } else if (method == LSPMethod::TextDocumentReferences) {
            auto &params = get<unique_ptr<ReferenceParams>>(rawParams);
            typecheckerCoord.syncRun(make_unique<ReferencesTask>(*config, id, move(params)));
        } else if (method == LSPMethod::SorbetReadFile) {
            auto &params = get<unique_ptr<TextDocumentIdentifier>>(rawParams);
            typecheckerCoord.syncRun(make_unique<SorbetReadFileTask>(*config, id, move(params)));
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
