#include "main/lsp/TimeTravelingGlobalState.h"
#include "core/Unfreeze.h"
#include "main/lsp/LSPPreprocessor.h"
#include "main/pipeline/pipeline.h"

using namespace std;

namespace sorbet::realmain::lsp {
TimeTravelingGlobalState::TimeTravelingGlobalState(const LSPConfiguration &config,
                                                   const std::shared_ptr<spdlog::logger> &logger, WorkerPool &workers,
                                                   unique_ptr<core::GlobalState> gs, int initialVersion)
    : config(config), logger(logger), workers(workers), gs(move(gs)), activeVersion(initialVersion),
      latestVersion(initialVersion) {
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
            fref.data(*gs).strictLevel = pipeline::decideStrictLevel(*gs, fref, config.opts);
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

namespace {
// TODO: Unit tests.
bool comesBefore(int version, int currentVersion, int maxVersion) {
    // Is version in (maxVersion, currentVersion)?
    if (currentVersion < maxVersion) {
        // Not in [currentVersion, maxVersion]
        return version < currentVersion || version > maxVersion;
    } else {
        // In (maxVersion, currentVersion)
        return version > maxVersion && version < currentVersion;
    }
}
} // namespace

vector<TimeTravelingGlobalState::TimeTravelUpdate *> TimeTravelingGlobalState::updatesBetweenExclusive(int start,
                                                                                                       int end) {
    // The end may be latest-version-inclusive, or latestVersion+1. So set to one more than that.
    const int latestValidVersion = latestVersion + 21;
    ENFORCE(comesBefore(start, end, latestValidVersion));
    vector<TimeTravelUpdate *> updates;
    for (auto &entry : log) {
        // start < entry.version < end
        if (comesBefore(start, entry.version, latestValidVersion) &&
            comesBefore(entry.version, end, latestValidVersion)) {
            updates.push_back(&entry);
        }
    }
    return updates;
}

void TimeTravelingGlobalState::travel(int version) {
    const bool undo = comesBefore(version, activeVersion, latestVersion);
    // Undo: Undo (version, activeVersion].
    // Redo: Redo (activeVersion, version].
    const int startVersion = undo ? version : activeVersion;
    const int endVersion = undo ? activeVersion + 1 : version + 1;
    auto updates = updatesBetweenExclusive(startVersion, endVersion);
    if (undo) {
        // Apply undos in reverse order (newest to oldest).
        reverse(updates.begin(), updates.end());
    }
    for (auto &update : updates) {
        applyUpdate(*update, undo);
    }
    // Should exactly match if redoing, or be at/just over version if undoing.
    ENFORCE((undo && activeVersion >= version) || activeVersion == version);
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
        Timer timeit(logger, "reIndexFromFileSystem");
        vector<core::FileRef> inputFiles = pipeline::reserveFiles(gs, config.opts.inputFileNames);
        for (auto &t : pipeline::index(gs, inputFiles, config.opts, workers, kvstore)) {
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
    Timer timeit(logger, "computeStateHashes");
    vector<core::FileHash> res(files.size());
    shared_ptr<ConcurrentBoundedQueue<int>> fileq = make_shared<ConcurrentBoundedQueue<int>>(files.size());
    for (int i = 0; i < files.size(); i++) {
        auto copy = i;
        fileq->push(move(copy), 1);
    }

    logger->debug("Computing state hashes for {} files", files.size());

    res.resize(files.size());

    shared_ptr<BlockingBoundedQueue<vector<pair<int, core::FileHash>>>> resultq =
        make_shared<BlockingBoundedQueue<vector<pair<int, core::FileHash>>>>(files.size());
    workers.multiplexJob("lspStateHash", [fileq, resultq, files, logger = this->logger]() {
        vector<pair<int, core::FileHash>> threadResult;
        int processedByThread = 0;
        int job;
        options::Options emptyOpts;
        emptyOpts.runLSP = true;

        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;

                    if (!files[job]) {
                        threadResult.emplace_back(job, core::FileHash{});
                        continue;
                    }
                    auto hash = pipeline::computeFileHash(files[job], *logger);
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
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), *logger); !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), *logger)) {
            if (result.gotItem()) {
                for (auto &a : threadResult) {
                    res[a.first] = move(a.second);
                }
            }
        }
    }
    return res;
}

void TimeTravelingGlobalState::pruneBefore(int version) {
    for (auto it = log.begin(); it != log.end();) {
        if (comesBefore(it->version, version, latestVersion)) {
            it = log.erase(it);
        } else {
            it++;
        }
    }
}

void TimeTravelingGlobalState::commitEdits(int version, LSPFileUpdates &update) {
    travel(latestVersion);

    Timer timeit(logger, "ttgs_commit_edits");
    // Hash changes.
    update.updatedFileHashes = computeStateHashes(update.updatedFiles);
    update.canTakeFastPath = canTakeFastPath(update);

    TimeTravelUpdate newUpdate{version};
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
            auto emptyFile = make_shared<core::File>(string(file->path()), "", core::File::Type::Normal);
            newUpdate.undoUpdate.hashUpdates.push_back(pipeline::computeFileHash(emptyFile, *logger));
            newUpdate.undoUpdate.fileUpdates.push_back(move(emptyFile));
        }
    }
    auto frefs = applyUpdate(newUpdate, false);
    latestVersion = version;

    log.push_back(move(newUpdate));

    // Index changes.
    update.updatedFileIndexes = pipeline::index(gs, frefs, config.opts, workers, kvstore);

    // Clear error queue.
    // (Note: Flushing is disabled in LSP mode, so we have to drain.)
    gs->errorQueue->drainWithQueryResponses();
}

bool TimeTravelingGlobalState::canTakeFastPath(const LSPFileUpdates &updates) const {
    Timer timeit(logger, "fast_path_decision");
    if (config.disableFastPath) {
        logger->debug("Taking slow path because fast path is disabled.");
        return false;
    }
    // Path taken after the first time an update has been encountered. Hack since we can't roll back new files just yet.
    if (updates.hasNewFiles) {
        logger->debug("Taking slow path because update has a new file");
        return false;
    }
    const auto &hashes = updates.updatedFileHashes;
    auto &changedFiles = updates.updatedFiles;
    logger->debug("Trying to see if fast path is available after {} file changes", changedFiles.size());

    ENFORCE(changedFiles.size() == hashes.size());
    int i = -1;
    {
        for (auto &f : changedFiles) {
            ++i;
            auto fref = gs->findFileByPath(f->path());
            if (!fref.exists()) {
                logger->debug("Taking slow path because {} is a new file", f->path());
                return false;
            } else {
                auto &oldHash = globalStateHashes[fref.id()];
                ENFORCE(oldHash.definitions.hierarchyHash != core::GlobalStateHash::HASH_STATE_NOT_COMPUTED);
                if (hashes[i].definitions.hierarchyHash == core::GlobalStateHash::HASH_STATE_INVALID) {
                    logger->debug("Taking slow path because {} has a syntax error", f->path());
                    return false;
                } else if (hashes[i].definitions.hierarchyHash != core::GlobalStateHash::HASH_STATE_INVALID &&
                           hashes[i].definitions.hierarchyHash != oldHash.definitions.hierarchyHash) {
                    logger->debug("Taking slow path because {} has changed definitions", f->path());
                    return false;
                }
            }
        }
    }
    return true;
}

} // namespace sorbet::realmain::lsp