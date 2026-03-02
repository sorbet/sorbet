#include "main/lsp/LSPIndexer.h"
#include "LSPFileUpdates.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "core/ErrorQueue.h"
#include "core/FileHash.h"
#include "core/NullFlusher.h"
#include "core/Unfreeze.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "hashing/hashing.h"
#include "main/cache/cache.h"
#include "main/lsp/DiagnosticSeverity.h"
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

LSPIndexer::LSPIndexer(shared_ptr<const LSPConfiguration> config, unique_ptr<core::GlobalState> gs,
                       unique_ptr<KeyValueStore> kvstore)
    : config(config), gs(move(gs)), kvstore(move(kvstore)), emptyWorkers(WorkerPool::create(0, *config->logger)) {}

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

// This function was previously called canTakeFastPath, but we changed it in ancitipation of adding
// incremental mode(s) that lied between the original fast and slow path. Leaving this comment here
// because old habits die hard and I still can only remember the name "canTakeFastPath"
LSPIndexer::TypecheckingPathResult
LSPIndexer::getTypecheckingPathInternal(const vector<shared_ptr<core::File>> &changedFiles,
                                        const UnorderedMap<core::FileRef, shared_ptr<core::File>> &evictedFiles) const {
    TypecheckingPathResult result;

    Timer timeit(config->logger, "fast_path_decision");
    auto &logger = *config->logger;
    logger.debug("Trying to see if fast path is available after {} file changes", changedFiles.size());
    if (config->disableFastPath) {
        logger.debug("Taking slow path because fast path is disabled.");
        prodCategoryCounterInc("lsp.slow_path_reason", "fast_path_disabled");
        timeit.setTag("path_chosen", "slow");
        return result;
    }

    if (changedFiles.size() > config->opts.lspMaxFilesOnFastPath) {
        logger.debug("Taking slow path because too many files changed ({} files > {} files)", changedFiles.size(),
                     config->opts.lspMaxFilesOnFastPath);
        prodCategoryCounterInc("lsp.slow_path_reason", "too_many_files");
        timeit.setTag("path_chosen", "slow");
        return result;
    }

    for (auto &f : changedFiles) {
        auto fref = gs->findFileByPath(f->path());
        if (!fref.exists()) {
            logger.debug("Taking slow path because {} is a new file", f->path());
            prodCategoryCounterInc("lsp.slow_path_reason", "new_file");
            timeit.setTag("path_chosen", "slow");
            return result;
        }

        const auto &oldFile = getOldFile(fref, *gs, evictedFiles);
        // We don't yet have a content hash that works for package files yet. Instead, we check if the package file
        // source text has changed at all. If it does, we take the slow path.
        // Only relevant in `--sorbet-packages` mode. This prevents LSP editing features like autocomplete from
        // working in `__package.rb` since every edit causes a slow path.
        // Note: We don't use File::isPackage because we have not necessarily set the packager options on initialGS yet
        if (this->config->opts.cacheSensitiveOptions.sorbetPackages && oldFile.hasPackageRbPath() &&
            oldFile.source() != f->source()) {
            logger.debug("Taking slow path because {} is a package file", f->path());
            prodCategoryCounterInc("lsp.slow_path_reason", "package_file");
            timeit.setTag("path_chosen", "slow");
            return result;
        }
        ENFORCE(oldFile.getFileHash() != nullptr);
        ENFORCE(f->getFileHash() != nullptr);
        const auto &oldHash = *oldFile.getFileHash();
        const auto &newHash = *f->getFileHash();
        ENFORCE(oldHash.localSymbolTableHashes.hierarchyHash != core::LocalSymbolTableHashes::HASH_STATE_NOT_COMPUTED);
        if (newHash.localSymbolTableHashes.isInvalidParse()) {
            logger.debug("Taking slow path because {} has a syntax error", f->path());
            prodCategoryCounterInc("lsp.slow_path_reason", "syntax_error");
            timeit.setTag("path_chosen", "slow");
            return result;
        }

        if (!newHash.localSymbolTableHashes.isInvalidParse() &&
            newHash.localSymbolTableHashes.hierarchyHash != oldHash.localSymbolTableHashes.hierarchyHash) {
            logger.debug("Taking slow path because {} has changed localSymbolTableHashes", f->path());
            prodCategoryCounterInc("lsp.slow_path_reason", "changed_definition");
            // Also record some information about what might have changed.
            const bool classesDiffer =
                newHash.localSymbolTableHashes.classModuleHash != oldHash.localSymbolTableHashes.classModuleHash;
            const bool typeMembersDiffer =
                newHash.localSymbolTableHashes.typeMemberHash != oldHash.localSymbolTableHashes.typeMemberHash;
            const bool fieldsDiffer =
                newHash.localSymbolTableHashes.fieldHash != oldHash.localSymbolTableHashes.fieldHash;
            const bool staticFieldsDiffer =
                newHash.localSymbolTableHashes.staticFieldHash != oldHash.localSymbolTableHashes.staticFieldHash;
            const bool classAliasesDiffer =
                newHash.localSymbolTableHashes.classAliasHash != oldHash.localSymbolTableHashes.classAliasHash;
            const bool methodsDiffer =
                newHash.localSymbolTableHashes.methodHash != oldHash.localSymbolTableHashes.methodHash;
            const uint32_t differCount = int(classesDiffer) + int(typeMembersDiffer) + int(fieldsDiffer) +
                                         int(staticFieldsDiffer) + int(classAliasesDiffer) + int(methodsDiffer);
            if (classesDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "classmodule");
            }
            if (typeMembersDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "typemember");
            }
            if (fieldsDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "icvar");
            }
            if (staticFieldsDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "staticfield");
            }
            if (classAliasesDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "classalias");
            }
            if (methodsDiffer) {
                prodCategoryCounterInc("lsp.slow_path_changed_def", "method");
            }
            if (differCount == 1) {
                if (classesDiffer) {
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlyclassmodule");
                } else if (typeMembersDiffer) {
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlytypemembers");
                } else if (fieldsDiffer) {
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlyicvars");
                } else if (staticFieldsDiffer) {
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlystaticfields");
                } else if (classAliasesDiffer) {
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlyclassaliases");
                } else {
                    ENFORCE(methodsDiffer);
                    prodCategoryCounterInc("lsp.slow_path_changed_def", "onlymethods");
                }
            }
            timeit.setTag("path_chosen", "slow");
            return result;
        }
    }

    // Technically we could say "yes, we can take the fast path" here (because we've detected that
    // the hierarchy hash has not changed for any file).
    //
    // But because of how the fast path is currently always run in a blocking "Typechecking in
    // foreground..." operation, we also compute how many downstream files (outside of the changed
    // files) would need to be typechecked on the fast path so we can compare that number against
    // `lspMaxFilesOnFastPath` as well.
    result.files = LSPFileUpdates::fastPathFilesToTypecheck(*gs, *config, changedFiles, evictedFiles);
    if (result.files.totalChanged > config->opts.lspMaxFilesOnFastPath) {
        logger.debug(
            "Taking slow path because too many extra files would be typechecked on the fast path ({} files > {} files)",
            result.files.totalChanged, config->opts.lspMaxFilesOnFastPath);
        prodCategoryCounterInc("lsp.slow_path_reason", "too_many_extra_files");
        timeit.setTag("path_chosen", "slow");
        return result;
    }

    result.path = TypecheckingPath::Fast;
    logger.debug("Taking fast path");

    // Using the incremental name indicates that we changed a symbol that affects an unrelated file, and thus traversed
    // the file table to determine if we can still take the fast path.
    if (result.files.useIncrementalNamer) {
        timeit.setTag("path_chosen", "fast+incremental_namer");
    } else {
        timeit.setTag("path_chosen", "fast");
    }

    return result;
}

TypecheckingPath
LSPIndexer::getTypecheckingPath(LSPFileUpdates &edit,
                                const UnorderedMap<core::FileRef, shared_ptr<core::File>> &evictedFiles) const {
    auto &logger = *config->logger;
    // Path taken after the first time an update has been encountered. Hack since we can't roll back new files just yet.
    if (edit.hasNewFiles) {
        logger.debug("Taking slow path because update has a new file");
        prodCategoryCounterInc("lsp.slow_path_reason", "new_file");
        return TypecheckingPath::Slow;
    }

    auto [path, result] = getTypecheckingPathInternal(edit.updatedFiles, evictedFiles);
    switch (path) {
        case TypecheckingPath::Fast: {
            edit.fastPathUseIncrementalNamer = result.useIncrementalNamer;
            edit.fastPathExtraFiles = std::move(result.extraFiles);
            break;
        }

        case TypecheckingPath::Slow:
            break;
    }

    return path;
}

TypecheckingPath LSPIndexer::getTypecheckingPath(const vector<shared_ptr<core::File>> &changedFiles) const {
    static UnorderedMap<core::FileRef, shared_ptr<core::File>> emptyMap;

    // Avoid expensively computing file hashes if there are too many files and it's likely that we'd
    // do a lot of hashing just to realize that something changed, requiring a slowpath anyways.
    if (changedFiles.size() > config->opts.lspMaxFilesOnFastPath) {
        config->logger->debug("Taking slow path because too many files changed ({} files > {} files)",
                              changedFiles.size(), config->opts.lspMaxFilesOnFastPath);
        prodCategoryCounterInc("lsp.slow_path_reason", "too_many_files");
        return TypecheckingPath::Slow;
    }

    // Ensure all files have computed hashes.
    computeFileHashes(changedFiles);

    auto [path, result] = getTypecheckingPathInternal(changedFiles, emptyMap);
    return path;
}

void LSPIndexer::transferInitializeState(InitializedTask &task) {
    ENFORCE(!this->config->opts.genPackages);
    // Copying the global state here means that we snapshot before any files have been loaded. That means that the
    // indexer and typechecker's file tables will almost immediately diverge, but that's not an issue as we don't share
    // `core::FileRef` values between the two.
    auto enableGenPackages = false;
    auto enableGenPackagesAllowRelaxingTestVisibility = false;
    auto typecheckerGS = std::exchange(
        this->gs, this->gs->copyForLSPTypechecker(
                      this->config->opts.cacheSensitiveOptions.sorbetPackages,
                      this->config->opts.extraPackageFilesDirectoryUnderscorePrefixes,
                      this->config->opts.extraPackageFilesDirectorySlashDeprecatedPrefixes,
                      this->config->opts.extraPackageFilesDirectorySlashPrefixes,
                      this->config->opts.packageSkipRBIExportEnforcementDirs,
                      this->config->opts.allowRelaxedPackagerChecksFor, this->config->opts.updateVisibilityFor,
                      this->config->opts.packagerLayers, this->config->opts.sorbetPackagesHint, enableGenPackages,
                      enableGenPackagesAllowRelaxingTestVisibility, this->config->opts.testPackages));

    task.setGlobalState(std::move(typecheckerGS));
    task.setKeyValueStore(std::move(this->kvstore));
}

void LSPIndexer::initialize(IndexerInitializationTask &task, vector<shared_ptr<core::File>> &&files) {
    if (initialized) {
        Exception::raise("Indexer is already initialized; cannot initialize a second time.");
    }

    {
        core::UnfreezeFileTable unfreezeFiles{*this->gs};

        for (auto &file : files) {
            auto fref = this->gs->findFileByPath(file->path());
            if (fref.exists()) {
                this->gs->replaceFile(fref, std::move(file));
            } else {
                this->gs->enterFile(std::move(file));
            }
        }
    }

    initialized = true;
}

unique_ptr<LSPFileUpdates> LSPIndexer::commitEdit(SorbetWorkspaceEditParams &edit, WorkerPool &workers) {
    Timer timeit(config->logger, "LSPIndexer::commitEdit");
    auto result = make_unique<LSPFileUpdates>();
    auto &update = *result;
    update.epoch = edit.epoch;
    update.editCount = edit.mergeCount + 1;
    update.updatedFiles = move(edit.updates);
    update.cancellationExpected = edit.sorbetCancellationExpected;
    update.preemptionsExpected = edit.sorbetPreemptionsExpected;
    // _Wait_ to compute `getTypecheckingPath` until after we compute hashes.

    UnorderedMap<core::FileRef, shared_ptr<core::File>> newlyEvictedFiles;
    // Update globalStateHashes. Keep track of file IDs for these files, along with old hashes for these files.
    vector<core::FileRef> frefs;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        for (auto &file : update.updatedFiles) {
            auto fref = gs->findFileByPath(file->path());
            if (fref.exists()) {
                newlyEvictedFiles[fref] = gs->getFiles()[fref.id()];
                gs->replaceFile(fref, file);
            } else {
                // This file update adds a new file to GlobalState.
                update.hasNewFiles = true;
                fref = gs->enterFile(file);
                fref.data(*gs).strictLevel = pipeline::decideStrictLevel(*gs, fref, config->opts);
            }
            frefs.emplace_back(fref);
        }
    }

    // Index changes in gs. pipeline::index sorts output by file id, but we need to reorder to match the order of
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
        gs->errorQueue = make_shared<core::ErrorQueue>(gs->errorQueue->logger, gs->errorQueue->tracer,
                                                       make_shared<core::NullFlusher>());
        auto trees = hashing::Hashing::indexAndComputeFileHashes(*gs, config->opts, *config->logger,
                                                                 absl::Span<core::FileRef>(frefs), workers, kvstore);
        ENFORCE(trees.hasResult(), "The indexer thread doesn't support cancellation");
    }

    // _Now_ that we've computed file hashes, we can make a fast path determination.
    update.typecheckingPath = getTypecheckingPath(update, newlyEvictedFiles);

    auto runningSlowPath = gs->epochManager->getStatus();
    if (runningSlowPath.slowPathRunning) {
        // A cancelable slow path is currently running. Check if we can cancel.
        // Invariant: `pendingTypecheckUpdates` should contain the edits currently being typechecked on the slow path.
        // runningSlowPath.epoch should be in the interval (pendingTypecheckUpdates.epoch - editCount,
        // pendingTypecheckUpdates.epoch]
        ENFORCE(runningSlowPath.epoch <= pendingTypecheckUpdates.epoch);
        ENFORCE(runningSlowPath.epoch > (pendingTypecheckUpdates.epoch - pendingTypecheckUpdates.editCount));

        // Cancel if the new update will take the slow path anyway.
        if (update.typecheckingPath != TypecheckingPath::Fast && gs->epochManager->tryCancelSlowPath(update.epoch)) {
            // Cancelation succeeded! Merge the updates from the cancelled run into the current update.
            update.mergeOlder(pendingTypecheckUpdates);
            mergeEvictedFiles(evictedFiles, newlyEvictedFiles);
            // The two updates together could end up taking the fast path.
            update.typecheckingPath = getTypecheckingPath(update, newlyEvictedFiles);
            update.canceledSlowPath = true;
        }
    }

    if (update.canceledSlowPath) {
        // Merge diagnostic latency timers; this edit contains the previous slow path.
        edit.diagnosticLatencyTimers.insert(edit.diagnosticLatencyTimers.end(),
                                            make_move_iterator(pendingTypecheckDiagnosticLatencyTimers.begin()),
                                            make_move_iterator(pendingTypecheckDiagnosticLatencyTimers.end()));
        clearAndReplaceTimers(pendingTypecheckDiagnosticLatencyTimers, edit.diagnosticLatencyTimers);
    } else if (update.typecheckingPath != TypecheckingPath::Fast) {
        // Replace diagnostic latency timers; this is a new slow path that did not cancel the previous slow path.
        clearAndReplaceTimers(pendingTypecheckDiagnosticLatencyTimers, edit.diagnosticLatencyTimers);
    }

    switch (update.typecheckingPath) {
        case TypecheckingPath::Fast: {
            // Edit takes the fast path. Merge with this edit so we can reverse it if the slow path gets canceled.
            auto merged = update.copy();
            merged.mergeOlder(pendingTypecheckUpdates);
            pendingTypecheckUpdates = move(merged);
            if (!update.canceledSlowPath) {
                // If a slow path is running, this update preempted.
                pendingTypecheckUpdates.committedEditCount += update.editCount;
            }
            mergeEvictedFiles(evictedFiles, newlyEvictedFiles);
            break;
        }
        case TypecheckingPath::Slow: {
            // Completely replace `pendingTypecheckUpdates` if this was a slow path update.
            pendingTypecheckUpdates = update.copy();
            break;
        }
    }

    // newlyEvictedFiles contains the changes from this edit + changes from the pending typecheck, if applicable.
    evictedFiles = std::move(newlyEvictedFiles);

    // Don't copy over these (test-only) properties, as they only apply to the original request.
    pendingTypecheckUpdates.cancellationExpected = false;
    pendingTypecheckUpdates.preemptionsExpected = 0;

    return result;
}

unique_ptr<LSPFileUpdates> LSPIndexer::commitEdit(SorbetWorkspaceEditParams &edit) {
    ENFORCE(edit.updates.size() <= config->opts.lspMaxFilesOnFastPath, "Too many files to index serially");
    return commitEdit(edit, *emptyWorkers);
}

core::FileRef LSPIndexer::uri2FileRef(string_view uri) const {
    return config->uri2FileRef(*gs, uri);
}

const core::File &LSPIndexer::getFile(core::FileRef fref) const {
    ENFORCE(fref.exists());
    return fref.data(*gs);
}

void LSPIndexer::updateConfigAndGsFromOptions(const DidChangeConfigurationParams &options) const {
    gs->trackUntyped = LSPClientConfiguration::parseEnableHighlightUntyped(*options.settings, gs->trackUntyped);

    // Errors are flushed from the typechecker thread, so this should not matter, but we may as well
    // set it just in case.
    if (options.settings->highlightUntypedDiagnosticSeverity.has_value()) {
        gs->highlightUntypedDiagnosticSeverity =
            convertDiagnosticSeverity(options.settings->highlightUntypedDiagnosticSeverity.value());
    }
}

} // namespace sorbet::realmain::lsp
