#include "main/lsp/LSPIndexer.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "core/ErrorQueue.h"
#include "core/FileHash.h"
#include "core/NullFlusher.h"
#include "core/Unfreeze.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "hashing/hashing.h"
#include "main/cache/cache.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/json_types.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"

using namespace std;

namespace sorbet::realmain::lsp {
namespace {
const core::File &getOldFile(core::FileRef fref, const core::GlobalState &gs,
                             const UnorderedMap<core::FileRef, shared_ptr<core::File>> &evictedFiles) {
    const auto &it = evictedFiles.find(fref);
    if (it != evictedFiles.end()) {
        return *it->second;
    }
    ENFORCE(fref.exists());
    return fref.data(gs);
}

// Merges *oldEvictedFiles* into *newlyEvictedFiles*. Mutates newlyEvictedFiles.
void mergeEvictedFiles(const UnorderedMap<core::FileRef, shared_ptr<core::File>> &oldEvictedFiles,
                       UnorderedMap<core::FileRef, shared_ptr<core::File>> &newlyEvictedFiles) {
    // Keep the older of the two file versions. We want the file version just prior to the currently pending slow path.
    for (const auto &entry : oldEvictedFiles) {
        newlyEvictedFiles[entry.first] = move(entry.second);
    }
}

// Cancels all timers in timers and clears the vector, and replaces with clones of newTimers.
void clearAndReplaceTimers(vector<unique_ptr<Timer>> &timers, const vector<unique_ptr<Timer>> &newTimers) {
    for (auto &timer : timers) {
        if (timer != nullptr) {
            timer->cancel();
        }
    }
    timers.clear();
    timers.reserve(newTimers.size());
    for (auto &timer : newTimers) {
        timers.push_back(make_unique<Timer>(timer->clone()));
    }
}
} // namespace

LSPIndexer::LSPIndexer(shared_ptr<const LSPConfiguration> config, unique_ptr<core::GlobalState> initialGS,
                       unique_ptr<KeyValueStore> kvstore)
    : config(config), initialGS(move(initialGS)), kvstore(move(kvstore)),
      emptyWorkers(WorkerPool::create(0, *config->logger)) {}

LSPIndexer::~LSPIndexer() {
    for (auto &timer : pendingTypecheckDiagnosticLatencyTimers) {
        timer->cancel();
    }
}

void LSPIndexer::computeFileHashes(const vector<shared_ptr<core::File>> &files, WorkerPool &workers) const {
    // Fast abort if all files have hashes.
    if (absl::c_all_of(files, [](const auto &f) { return f == nullptr || f->getFileHash() != nullptr; })) {
        return;
    }

    hashing::Hashing::computeFileHashes(files, *config->logger, workers, config->opts);
}

void LSPIndexer::computeFileHashes(const vector<shared_ptr<core::File>> &files) const {
    computeFileHashes(files, *emptyWorkers);
}

bool LSPIndexer::canTakeFastPathInternal(
    const vector<shared_ptr<core::File>> &changedFiles,
    const UnorderedMap<core::FileRef, shared_ptr<core::File>> &evictedFiles) const {
    Timer timeit(config->logger, "fast_path_decision");
    auto &logger = *config->logger;
    logger.debug("Trying to see if fast path is available after {} file changes", changedFiles.size());
    if (config->disableFastPath) {
        logger.debug("Taking slow path because fast path is disabled.");
        prodCategoryCounterInc("lsp.slow_path_reason", "fast_path_disabled");
        return false;
    }

    if (changedFiles.size() > config->opts.lspMaxFilesOnFastPath) {
        logger.debug("Taking slow path because too many files changed ({} files > {} files)", changedFiles.size(),
                     config->opts.lspMaxFilesOnFastPath);
        prodCategoryCounterInc("lsp.slow_path_reason", "too_many_files");
        return false;
    }

    for (auto &f : changedFiles) {
        auto fref = initialGS->findFileByPath(f->path());
        if (!fref.exists()) {
            logger.debug("Taking slow path because {} is a new file", f->path());
            prodCategoryCounterInc("lsp.slow_path_reason", "new_file");
            return false;
        }

        const auto &oldFile = getOldFile(fref, *initialGS, evictedFiles);
        // We don't yet have a content hash that works for package files yet. Instead, we check if the package file
        // source text has changed at all. If it does, we take the slow path.
        // Only relevant in `--stripe-packages` mode. This prevents LSP editing features like autocomplete from
        // working in `__package.rb` since every edit causes a slow path.
        // TODO(jvilk): We could use `PackageInfo` as a `__package.rb` hash -- but we would have to stash it
        // somewhere. Currently, we discard them after `packager` runs.
        if (this->config->opts.stripePackages && oldFile.isPackage() && oldFile.source() != f->source()) {
            logger.debug("Taking slow path because {} is a package file", f->path());
            prodCategoryCounterInc("lsp.slow_path_reason", "package_file");
            return false;
        }
        ENFORCE(oldFile.getFileHash() != nullptr);
        ENFORCE(f->getFileHash() != nullptr);
        const auto &oldHash = *oldFile.getFileHash();
        const auto &newHash = *f->getFileHash();
        ENFORCE(oldHash.localSymbolTableHashes.hierarchyHash != core::LocalSymbolTableHashes::HASH_STATE_NOT_COMPUTED);
        if (newHash.localSymbolTableHashes.isInvalidParse()) {
            logger.debug("Taking slow path because {} has a syntax error", f->path());
            prodCategoryCounterInc("lsp.slow_path_reason", "syntax_error");
            return false;
        }

        if (!newHash.localSymbolTableHashes.isInvalidParse() &&
            newHash.localSymbolTableHashes.hierarchyHash != oldHash.localSymbolTableHashes.hierarchyHash) {
            logger.debug("Taking slow path because {} has changed localSymbolTableHashes", f->path());
            prodCategoryCounterInc("lsp.slow_path_reason", "changed_definition");
            // Also record some information about what might have changed.
            const bool classesDiffer =
                newHash.localSymbolTableHashes.classModuleHash != oldHash.localSymbolTableHashes.classModuleHash;
            const bool typeArgumentsDiffer =
                newHash.localSymbolTableHashes.typeArgumentHash != oldHash.localSymbolTableHashes.typeArgumentHash;
            const bool typeMembersDiffer =
                newHash.localSymbolTableHashes.typeMemberHash != oldHash.localSymbolTableHashes.typeMemberHash;
            const bool fieldsDiffer =
                newHash.localSymbolTableHashes.fieldHash != oldHash.localSymbolTableHashes.fieldHash;
            const bool methodsDiffer =
                newHash.localSymbolTableHashes.methodHash != oldHash.localSymbolTableHashes.methodHash;
            const uint32_t differCount = int(classesDiffer) + int(typeArgumentsDiffer) + int(typeMembersDiffer) +
                                         int(fieldsDiffer) + int(methodsDiffer);
            if (classesDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "classmodule");
            }
            if (typeArgumentsDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "typeargument");
            }
            if (typeMembersDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "typemember");
            }
            if (fieldsDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "field");
            }
            if (methodsDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "method");
            }
            if (differCount == 1) {
                if (classesDiffer) {
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlyclassmodule");
                } else if (typeArgumentsDiffer) {
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlytypeargument");
                } else if (typeMembersDiffer) {
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlytypemembers");
                } else if (fieldsDiffer) {
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlyfields");
                } else {
                    ENFORCE(methodsDiffer);
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlymethods");
                }
            }
            return false;
        }
    }

    // Technically we could say "yes, we can take the fast path" here (because we've detected that
    // the hierarchy hash has not changed for any file).
    //
    // But because of how the fast path is currently always run in a blocking "Typechecking in
    // foreground..." operation, we also compute how many downstream files (outside of the changed
    // files) would need to be typechecked on the fast path so we can compare that number against
    // `lspMaxFilesOnFastPath` as well.

    // TODO(jez) Currently we compute the full set of information that we would need for the sake of
    // whether to take the fast path twice--once here and once again in runFastPath on the
    // typechecking thread.
    //
    // As an optimization, we might want to try to store that information on the update itself, so
    // that the typechecking thread can simply read it instead of having to compute it.
    auto result = LSPFileUpdates::fastPathFilesToTypecheck(*initialGS, *config, changedFiles, evictedFiles);
    auto filesToTypecheck = result.changedFiles.size() + result.extraFiles.size();
    if (filesToTypecheck > config->opts.lspMaxFilesOnFastPath) {
        logger.debug(
            "Taking slow path because too many extra files would be typechecked on the fast path ({} files > {} files)",
            filesToTypecheck, config->opts.lspMaxFilesOnFastPath);
        prodCategoryCounterInc("lsp.slow_path_reason", "too_many_extra_files");
        return false;
    }

    logger.debug("Taking fast path");
    return true;
}

bool LSPIndexer::canTakeFastPath(const LSPFileUpdates &edit,
                                 const UnorderedMap<core::FileRef, shared_ptr<core::File>> &evictedFiles) const {
    auto &logger = *config->logger;
    // Path taken after the first time an update has been encountered. Hack since we can't roll back new files just yet.
    if (edit.hasNewFiles) {
        logger.debug("Taking slow path because update has a new file");
        prodCategoryCounterInc("lsp.slow_path_reason", "new_file");
        return false;
    }
    return canTakeFastPathInternal(edit.updatedFiles, evictedFiles);
}

bool LSPIndexer::canTakeFastPath(const vector<shared_ptr<core::File>> &changedFiles) const {
    static UnorderedMap<core::FileRef, shared_ptr<core::File>> emptyMap;

    // Avoid expensively computing file hashes if there are too many files and it's likely that we'd
    // do a lot of hashing just to realize that something changed, requiring a slowpath anyways.
    if (changedFiles.size() > config->opts.lspMaxFilesOnFastPath) {
        config->logger->debug("Taking slow path because too many files changed ({} files > {} files)",
                              changedFiles.size(), config->opts.lspMaxFilesOnFastPath);
        prodCategoryCounterInc("lsp.slow_path_reason", "too_many_files");
        return false;
    }

    // Ensure all files have computed hashes.
    computeFileHashes(changedFiles);
    return canTakeFastPathInternal(changedFiles, emptyMap);
}

void LSPIndexer::transferInitializeState(InitializedTask &task) {
    task.setGlobalState(std::move(this->initialGS));
    task.setKeyValueStore(std::move(this->kvstore));
}

void LSPIndexer::initialize(IndexerInitializationTask &task, std::unique_ptr<core::GlobalState> initialGS) {
    if (initialized) {
        Exception::raise("Indexer is already initialized; cannot initialize a second time.");
    }
    initialized = true;

    this->initialGS = std::move(initialGS);
}

LSPFileUpdates LSPIndexer::commitEdit(SorbetWorkspaceEditParams &edit, WorkerPool &workers) {
    Timer timeit(config->logger, "LSPIndexer::commitEdit");
    LSPFileUpdates update;
    update.epoch = edit.epoch;
    update.editCount = edit.mergeCount + 1;
    update.updatedFiles = move(edit.updates);
    update.cancellationExpected = edit.sorbetCancellationExpected;
    update.preemptionsExpected = edit.sorbetPreemptionsExpected;
    // _Wait_ to compute `canTakeFastPath` until after we compute hashes.

    UnorderedMap<core::FileRef, shared_ptr<core::File>> newlyEvictedFiles;
    // Update globalStateHashes. Keep track of file IDs for these files, along with old hashes for these files.
    vector<core::FileRef> frefs;
    {
        core::UnfreezeFileTable fileTableAccess(*initialGS);
        for (auto &file : update.updatedFiles) {
            auto fref = initialGS->findFileByPath(file->path());
            if (fref.exists()) {
                newlyEvictedFiles[fref] = initialGS->getFiles()[fref.id()];
                initialGS->replaceFile(fref, file);
            } else {
                // This file update adds a new file to GlobalState.
                update.hasNewFiles = true;
                fref = initialGS->enterFile(file);
                fref.data(*initialGS).strictLevel = pipeline::decideStrictLevel(*initialGS, fref, config->opts);
            }
            frefs.emplace_back(fref);
        }
    }

    // Index changes in initialGS. pipeline::index sorts output by file id, but we need to reorder to match the order of
    // other fields.
    UnorderedMap<core::FileRef, int> fileToPos;
    {
        int i = -1;
        for (auto fref : frefs) {
            // We should have ensured before reaching here that there are no duplicates.
            ENFORCE(!fileToPos.contains(fref));
            i++;
            fileToPos[fref] = i;
        }
    }

    {
        // Explicitly null. It does not make sense to use kvstore for short-lived editor edits.
        unique_ptr<const OwnedKeyValueStore> kvstore;
        // Create a throwaway error queue. commitEdit may be called on two different threads, and we can't anticipate
        // which one it will be.
        initialGS->errorQueue = make_shared<core::ErrorQueue>(
            initialGS->errorQueue->logger, initialGS->errorQueue->tracer, make_shared<core::NullFlusher>());
        auto trees = hashing::Hashing::indexAndComputeFileHashes(initialGS, config->opts, *config->logger, frefs,
                                                                 workers, kvstore);
        update.updatedFileIndexes.resize(trees.size());
        for (auto &ast : trees) {
            const int i = fileToPos[ast.file];
            update.updatedFileIndexes[i] = move(ast);
        }
    }

    // _Now_ that we've computed file hashes, we can make a fast path determination.
    update.canTakeFastPath = canTakeFastPath(update, newlyEvictedFiles);

    auto runningSlowPath = initialGS->epochManager->getStatus();
    if (runningSlowPath.slowPathRunning) {
        // A cancelable slow path is currently running. Check if we can cancel.
        // Invariant: `pendingTypecheckUpdates` should contain the edits currently being typechecked on the slow path.
        // runningSlowPath.epoch should be in the interval (pendingTypecheckUpdates.epoch - editCount,
        // pendingTypecheckUpdates.epoch]
        ENFORCE(runningSlowPath.epoch <= pendingTypecheckUpdates.epoch);
        ENFORCE(runningSlowPath.epoch > (pendingTypecheckUpdates.epoch - pendingTypecheckUpdates.editCount));

        // Cancel if the new update will take the slow path anyway.
        if (!update.canTakeFastPath && initialGS->epochManager->tryCancelSlowPath(update.epoch)) {
            // Cancelation succeeded! Merge the updates from the cancelled run into the current update.
            update.mergeOlder(pendingTypecheckUpdates);
            // The two updates together could end up taking the fast path.
            update.canTakeFastPath = canTakeFastPath(update, evictedFiles);
            update.canceledSlowPath = true;
            mergeEvictedFiles(evictedFiles, newlyEvictedFiles);
        }
    }

    ENFORCE(update.updatedFiles.size() == update.updatedFileIndexes.size());

    if (update.canceledSlowPath) {
        // Merge diagnostic latency timers; this edit contains the previous slow path.
        edit.diagnosticLatencyTimers.insert(edit.diagnosticLatencyTimers.end(),
                                            make_move_iterator(pendingTypecheckDiagnosticLatencyTimers.begin()),
                                            make_move_iterator(pendingTypecheckDiagnosticLatencyTimers.end()));
        clearAndReplaceTimers(pendingTypecheckDiagnosticLatencyTimers, edit.diagnosticLatencyTimers);
    } else if (!update.canTakeFastPath) {
        // Replace diagnostic latency timers; this is a new slow path that did not cancel the previous slow path.
        clearAndReplaceTimers(pendingTypecheckDiagnosticLatencyTimers, edit.diagnosticLatencyTimers);
    }

    if (update.canTakeFastPath) {
        // Edit takes the fast path. Merge with this edit so we can reverse it if the slow path gets canceled.
        auto merged = update.copy();
        merged.mergeOlder(pendingTypecheckUpdates);
        pendingTypecheckUpdates = move(merged);
        if (!update.canceledSlowPath) {
            // If a slow path is running, this update preempted.
            pendingTypecheckUpdates.committedEditCount += update.editCount;
        }
        mergeEvictedFiles(evictedFiles, newlyEvictedFiles);
    } else {
        // Completely replace `pendingTypecheckUpdates` if this was a slow path update.
        update.updatedGS = initialGS->deepCopy();
        pendingTypecheckUpdates = update.copy();
    }

    // newlyEvictedFiles contains the changes from this edit + changes from the pending typecheck, if applicable.
    evictedFiles = std::move(newlyEvictedFiles);

    // Don't copy over these (test-only) properties, as they only apply to the original request.
    pendingTypecheckUpdates.cancellationExpected = false;
    pendingTypecheckUpdates.preemptionsExpected = 0;

    return update;
}

LSPFileUpdates LSPIndexer::commitEdit(SorbetWorkspaceEditParams &edit) {
    ENFORCE(edit.updates.size() <= config->opts.lspMaxFilesOnFastPath, "Too many files to index serially");
    return commitEdit(edit, *emptyWorkers);
}

core::FileRef LSPIndexer::uri2FileRef(string_view uri) const {
    return config->uri2FileRef(*initialGS, uri);
}

const core::File &LSPIndexer::getFile(core::FileRef fref) const {
    ENFORCE(fref.exists());
    return fref.data(*initialGS);
}

} // namespace sorbet::realmain::lsp
