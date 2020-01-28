#include "main/lsp/LSPIndexer.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/ShowOperation.h"
#include "main/pipeline/pipeline.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

const core::FileHash &findHash(int id, const vector<core::FileHash> &globalStateHashes,
                               const UnorderedMap<int, core::FileHash> &overriddingStateHashes) {
    const auto it = overriddingStateHashes.find(id);
    if (it == overriddingStateHashes.end()) {
        return globalStateHashes[id];
    }
    return it->second;
}

bool canTakeFastPathInternal(
    const core::GlobalState &gs, const LSPConfiguration &config, const vector<core::FileHash> &globalStateHashes,
    const vector<core::FileHash> &changedHashes, const vector<shared_ptr<core::File>> &changedFiles,
    bool hasNewFiles = false,
    const UnorderedMap<int, core::FileHash> &overriddingStateHashes = UnorderedMap<int, core::FileHash>()) {
    Timer timeit(config.logger, "fast_path_decision");
    auto &logger = *config.logger;
    if (config.disableFastPath) {
        logger.debug("Taking slow path because fast path is disabled.");
        prodCategoryCounterInc("lsp.slow_path_reason", "fast_path_disabled");
        return false;
    }
    // Path taken after the first time an update has been encountered. Hack since we can't roll back new files just yet.
    if (hasNewFiles) {
        logger.debug("Taking slow path because update has a new file");
        prodCategoryCounterInc("lsp.slow_path_reason", "new_file");
        return false;
    }
    logger.debug("Trying to see if fast path is available after {} file changes", changedFiles.size());

    ENFORCE(changedFiles.size() == changedHashes.size());
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
                if (changedHashes[i].definitions.hierarchyHash == core::GlobalStateHash::HASH_STATE_INVALID) {
                    logger.debug("Taking slow path because {} has a syntax error", f->path());
                    prodCategoryCounterInc("lsp.slow_path_reason", "syntax_error");
                    return false;
                } else if (changedHashes[i].definitions.hierarchyHash != core::GlobalStateHash::HASH_STATE_INVALID &&
                           changedHashes[i].definitions.hierarchyHash != oldHash.definitions.hierarchyHash) {
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

bool updateCanTakeFastPath(
    const core::GlobalState &gs, const LSPConfiguration &config, const vector<core::FileHash> &globalStateHashes,
    const LSPFileUpdates &updates,
    const UnorderedMap<int, core::FileHash> &overriddingStateHashes = UnorderedMap<int, core::FileHash>()) {
    return canTakeFastPathInternal(gs, config, globalStateHashes, updates.updatedFileHashes, updates.updatedFiles,
                                   updates.hasNewFiles, overriddingStateHashes);
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

LSPIndexer::LSPIndexer(shared_ptr<const LSPConfiguration> config, unique_ptr<core::GlobalState> initialGS)
    : config(config), initialGS(move(initialGS)), emptyWorkers(WorkerPool::create(0, *config->logger)) {}

vector<core::FileHash> LSPIndexer::computeFileHashes(const vector<shared_ptr<core::File>> &files,
                                                     WorkerPool &workers) const {
    Timer timeit(config->logger, "computeFileHashes");
    vector<core::FileHash> res(files.size());
    shared_ptr<ConcurrentBoundedQueue<int>> fileq = make_shared<ConcurrentBoundedQueue<int>>(files.size());
    for (int i = 0; i < files.size(); i++) {
        auto copy = i;
        fileq->push(move(copy), 1);
    }

    auto &logger = *config->logger;
    logger.debug("Computing state hashes for {} files", files.size());

    res.resize(files.size());

    shared_ptr<BlockingBoundedQueue<vector<pair<int, core::FileHash>>>> resultq =
        make_shared<BlockingBoundedQueue<vector<pair<int, core::FileHash>>>>(files.size());
    workers.multiplexJob("lspStateHash", [fileq, resultq, files, &logger]() {
        vector<pair<int, core::FileHash>> threadResult;
        int processedByThread = 0;
        int job;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;

                    if (!files[job]) {
                        threadResult.emplace_back(job, core::FileHash{});
                        continue;
                    }
                    auto hash = pipeline::computeFileHash(files[job], logger);
                    threadResult.emplace_back(job, move(hash));
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    {
        vector<pair<int, core::FileHash>> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger); !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger)) {
            if (result.gotItem()) {
                for (auto &a : threadResult) {
                    res[a.first] = move(a.second);
                }
            }
        }
    }
    return res;
}

vector<core::FileHash> LSPIndexer::computeFileHashes(const vector<shared_ptr<core::File>> &files) const {
    return computeFileHashes(files, *emptyWorkers);
}

bool LSPIndexer::canTakeFastPath(const SorbetWorkspaceEditParams &params,
                                 const vector<core::FileHash> &fileHashes) const {
    return canTakeFastPathInternal(*initialGS, *config, globalStateHashes, fileHashes, params.updates);
}

void LSPIndexer::initialize(LSPFileUpdates &updates, WorkerPool &workers) {
    if (initialized) {
        Exception::raise("Indexer is already initialized; cannot initialize a second time.");
    }
    initialized = true;
    // Temporarily replace error queue, as it asserts that the same thread that created it uses it and we're
    // going to use it on typechecker thread for this one operation.
    auto savedErrorQueue = initialGS->errorQueue;
    initialGS->errorQueue = make_shared<core::ErrorQueue>(savedErrorQueue->logger, savedErrorQueue->tracer);
    initialGS->errorQueue->ignoreFlushes = true;
    // Enforce that this is only run once.
    ENFORCE(globalStateHashes.empty());

    vector<ast::ParsedFile> indexed;
    Timer timeit(config->logger, "initial_index");
    ShowOperation op(*config, "Indexing", "Indexing files...");
    {
        Timer timeit(config->logger, "reIndexFromFileSystem");
        vector<core::FileRef> inputFiles = pipeline::reserveFiles(initialGS, config->opts.inputFileNames);
        for (auto &t : pipeline::index(initialGS, inputFiles, config->opts, workers, kvstore)) {
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

    globalStateHashes = computeFileHashes(initialGS->getFiles(), workers);

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

LSPFileUpdates LSPIndexer::commitEdit(SorbetWorkspaceEditParams &edit, std::vector<core::FileHash> newHashesOrEmpty) {
    LSPFileUpdates update;
    update.epoch = edit.epoch;
    update.editCount = edit.mergeCount + 1;
    update.updatedFileHashes =
        newHashesOrEmpty.empty() ? computeFileHashes(edit.updates, *emptyWorkers) : move(newHashesOrEmpty);
    update.updatedFiles = move(edit.updates);
    ENFORCE(update.updatedFileHashes.size() == update.updatedFiles.size());
    update.canTakeFastPath = updateCanTakeFastPath(*initialGS, *config, globalStateHashes, update);
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
        // Create a throwaway error queue. commitEdit may be called on two different threads, and we can't anticipate
        // which one it will be.
        initialGS->errorQueue =
            make_shared<core::ErrorQueue>(initialGS->errorQueue->logger, initialGS->errorQueue->tracer);
        initialGS->errorQueue->ignoreFlushes = true;
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
        merged.canTakeFastPath = updateCanTakeFastPath(*initialGS, *config, globalStateHashes, merged, mergedEvictions);
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

} // namespace sorbet::realmain::lsp
