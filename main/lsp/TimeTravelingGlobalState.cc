#include "main/lsp/TimeTravelingGlobalState.h"
#include "core/Unfreeze.h"
#include "main/lsp/LSPPreprocessor.h"
#include "main/pipeline/pipeline.h"

using namespace std;

namespace sorbet::realmain::lsp {
TimeTravelingGlobalState::TimeTravelingGlobalState(const shared_ptr<LSPConfiguration> &config,
                                                   unique_ptr<core::GlobalState> gs, WorkerPool &workers,
                                                   u4 initialVersion)
    : config(config), gs(move(gs)), workers(workers), activeVersion(initialVersion), latestVersion(initialVersion) {
    auto errorQueue = dynamic_pointer_cast<core::ErrorQueue>(this->gs->errorQueue);
    ENFORCE(errorQueue, "TimeTravelingGlobalState got an unexpected error queue");
    ENFORCE(errorQueue->ignoreFlushes, "TimeTravelingGlobalState's error queue is not ignoring flushes, which will "
                                       "prevent it from hashing files with syntax errors.");
}

vector<core::FileRef> TimeTravelingGlobalState::applyUpdate(TimeTravelUpdate &ttUpdate, bool undo) {
    vector<core::FileRef> rv;

    const auto &update = undo ? ttUpdate.undoUpdate : ttUpdate.update;
    ENFORCE(update.fileUpdates.size() == update.hashUpdates.size());
    core::UnfreezeFileTable fileTableAccess(*gs);
    int i = -1;
    for (auto &file : update.fileUpdates) {
        auto fref = gs->findFileByPath(file->path());
        i++;
        if (fref.exists()) {
            ENFORCE(fref.id() < globalStateHashes.size());
            gs = core::GlobalState::replaceFile(move(gs), fref, file);
        } else {
            fref = gs->enterFile(file);
            fref.data(*gs).strictLevel = pipeline::decideStrictLevel(*gs, fref, config->opts);
            if (fref.id() >= globalStateHashes.size()) {
                globalStateHashes.resize(fref.id() + 1);
            }
        }
        globalStateHashes[fref.id()] = update.hashUpdates[i];
        rv.push_back(fref);
    }
    // Travel to version before this update if undoing, or to this version if updating.
    activeVersion = undo ? ttUpdate.version - 1 : ttUpdate.version;
    return rv;
}

bool TimeTravelingGlobalState::comesBefore(u4 a, u4 b) const {
    // The code may request versions up to `latestVersion + 1`, so be one more than that.
    const u4 maxVersion = latestVersion + 1;
    if (b <= maxVersion) {
        // Not in [currentVersion, maxVersion]
        return a < b || a > maxVersion;
    } else {
        // In (maxVersion, currentVersion)
        return a > maxVersion && a < b;
    }
}

u4 TimeTravelingGlobalState::minVersion(u4 a, u4 b) const {
    if (comesBefore(a, b)) {
        return a;
    }
    return b;
}

vector<TimeTravelingGlobalState::TimeTravelUpdate *> TimeTravelingGlobalState::updatesBetweenExclusive(u4 start,
                                                                                                       u4 end) {
    ENFORCE(comesBefore(start, end));
    vector<TimeTravelUpdate *> updates;
    for (auto &entry : log) {
        // start < entry.version < end
        if (comesBefore(start, entry.version) && comesBefore(entry.version, end)) {
            updates.push_back(&entry);
        }
    }
    return updates;
}

void TimeTravelingGlobalState::travel(u4 version) {
    const bool undo = comesBefore(version, activeVersion);
    // Undo: Undo (version, activeVersion].
    // Redo: Redo (activeVersion, version].
    const u4 startVersion = undo ? version : activeVersion;
    const u4 endVersion = undo ? activeVersion + 1 : version + 1;
    auto updates = updatesBetweenExclusive(startVersion, endVersion);
    if (undo) {
        // Apply undos in reverse order (newest to oldest).
        reverse(updates.begin(), updates.end());
    }
    for (auto &update : updates) {
        applyUpdate(*update, undo);
    }
    // Should exactly match if redoing, or be at/just over version if undoing.
    ENFORCE((undo && comesBefore(version, activeVersion + 1)) || activeVersion == version);
}

const core::GlobalState &TimeTravelingGlobalState::getGlobalState() const {
    return *gs;
}

const vector<core::FileHash> &TimeTravelingGlobalState::getGlobalStateHashes() const {
    return globalStateHashes;
}

vector<ast::ParsedFile> TimeTravelingGlobalState::indexFromFileSystem() {
    vector<ast::ParsedFile> indexed;
    {
        Timer timeit(config->logger, "reIndexFromFileSystem");
        vector<core::FileRef> inputFiles = pipeline::reserveFiles(gs, config->opts.inputFileNames);
        for (auto &t : pipeline::index(gs, inputFiles, config->opts, workers, kvstore)) {
            int id = t.file.id();
            if (id >= indexed.size()) {
                indexed.resize(id + 1);
            }
            indexed[id] = move(t);
        }
        // Clear error queue.
        // (Note: Flushing is disabled in LSP mode, so we have to drain.)
        gs->errorQueue->drainWithQueryResponses();
    }
    globalStateHashes = computeStateHashes(gs->getFiles());
    return indexed;
}

vector<core::FileHash> TimeTravelingGlobalState::computeStateHashes(const vector<shared_ptr<core::File>> &files) const {
    Timer timeit(config->logger, "computeStateHashes");
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

void TimeTravelingGlobalState::pruneBefore(u4 version) {
    // Time-travel to the present before we forget how to.
    travel(latestVersion);
    for (auto it = log.begin(); it != log.end();) {
        if (comesBefore(it->version, version)) {
            it = log.erase(it);
        } else {
            it++;
        }
    }
}

void TimeTravelingGlobalState::commitEdits(LSPFileUpdates &update) {
    Timer timeit(config->logger, "ttgs_commit_edits");
    // Hash changes.
    update.updatedFileHashes = computeStateHashes(update.updatedFiles);
    update.canTakeFastPath = canTakeFastPath(latestVersion, update);

    TimeTravelUpdate newUpdate{update.versionEnd, update.hasNewFiles};
    newUpdate.update.fileUpdates = update.updatedFiles;
    newUpdate.update.hashUpdates = update.updatedFileHashes;

    // Generate undo log entry. Note: We only care about undoing changes to files and hashes.
    auto &allFiles = gs->getFiles();
    for (auto &file : update.updatedFiles) {
        auto fref = gs->findFileByPath(file->path());
        if (fref.exists()) {
            newUpdate.undoUpdate.fileUpdates.push_back(allFiles[fref.id()]);
            ENFORCE(fref.id() < globalStateHashes.size());
            newUpdate.undoUpdate.hashUpdates.push_back(globalStateHashes[fref.id()]);
        } else {
            update.hasNewFiles = true;
            // Reversal of a new file is... an empty file...
            auto emptyFile =
                make_shared<core::File>(string(file->path()), "", core::File::Type::Normal, update.versionStart - 1);
            newUpdate.undoUpdate.hashUpdates.push_back(pipeline::computeFileHash(emptyFile, *config->logger));
            newUpdate.undoUpdate.fileUpdates.push_back(move(emptyFile));
        }
    }
    auto frefs = applyUpdate(newUpdate, false);
    latestVersion = update.versionEnd;

    // Index changes. pipeline::index sorts output by file id, but we need to reorder to match the order of other
    // fields.
    UnorderedMap<u2, int> fileToPos;
    int i = -1;
    for (auto fref : frefs) {
        // We should have ensured before reaching here that there are no duplicates.
        ENFORCE(!fileToPos.contains(fref.id()));
        i++;
        fileToPos[fref.id()] = i;
    }

    auto trees = pipeline::index(gs, frefs, config->opts, workers, kvstore);
    update.updatedFileIndexes.resize(trees.size());
    newUpdate.update.updatedFileIndexes.resize(trees.size());
    for (auto &ast : trees) {
        const int i = fileToPos[ast.file.id()];
        newUpdate.update.updatedFileIndexes[i] = ast::ParsedFile{ast.tree->deepCopy(), ast.file};
        update.updatedFileIndexes[i] = move(ast);
    }

    log.push_back(move(newUpdate));

    // Clear error queue.
    // (Note: Flushing is disabled in LSP mode, so we have to drain.)
    gs->errorQueue->drainWithQueryResponses();
}

bool TimeTravelingGlobalState::canTakeFastPath(u4 fromId, const LSPFileUpdates &updates) {
    Timer timeit(config->logger, "fast_path_decision");
    auto &logger = *config->logger;
    travel(fromId);
    if (config->disableFastPath) {
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
            auto fref = gs->findFileByPath(f->path());
            if (!fref.exists()) {
                logger.debug("Taking slow path because {} is a new file", f->path());
                prodCategoryCounterInc("lsp.slow_path_reason", "new_file");
                return false;
            } else {
                auto &oldHash = globalStateHashes[fref.id()];
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

LSPFileUpdates TimeTravelingGlobalState::getCombinedUpdates(u4 fromId, u4 toId) {
    auto ttus = updatesBetweenExclusive(fromId - 1, toId + 1);
    // Apply backwards so later updates take precedence.
    reverse(ttus.begin(), ttus.end());

    UnorderedSet<string> encountered;
    LSPFileUpdates merged;
    merged.versionEnd = toId;
    merged.versionStart = fromId;
    for (const auto ttu : ttus) {
        const auto &update = ttu->update;
        int i = -1;
        merged.hasNewFiles = merged.hasNewFiles || ttu->hasNewFiles;
        for (const auto &file : update.fileUpdates) {
            i++;
            if (!encountered.contains(file->path())) {
                encountered.insert(string(file->path()));
                merged.updatedFiles.push_back(file);
                merged.updatedFileHashes.push_back(update.hashUpdates[i]);
                auto &ast = update.updatedFileIndexes[i];
                merged.updatedFileIndexes.push_back(ast::ParsedFile{ast.tree->deepCopy(), ast.file});
            }
        }
    }
    merged.canTakeFastPath = canTakeFastPath(fromId - 1, merged);

    return merged;
}

void TimeTravelingGlobalState::switchToNewThread() {
    // Re-create error queue that is owned by new thread.
    gs->errorQueue = make_shared<core::ErrorQueue>(gs->errorQueue->logger, gs->errorQueue->tracer);
}

} // namespace sorbet::realmain::lsp
