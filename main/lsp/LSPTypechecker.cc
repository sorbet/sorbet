#include "main/lsp/LSPTypechecker.h"
#include "LSPFileUpdates.h"
#include "absl/synchronization/mutex.h"
#include "ast/treemap/treemap.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "core/ErrorCollector.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "core/sig_finder/sig_finder.h"
#include "hashing/hashing.h"
#include "local_vars/local_vars.h"
#include "main/cache/cache.h"
#include "main/lsp/DefLocSaver.h"
#include "main/lsp/DiagnosticSeverity.h"
#include "main/lsp/ErrorFlusherLSP.h"
#include "main/lsp/ErrorReporter.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPPreprocessor.h"
#include "main/lsp/LocalVarSaver.h"
#include "main/lsp/QueryCollector.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/UndoState.h"
#include "main/lsp/json_types.h"
#include "main/lsp/notifications/indexer_initialization.h"
#include "main/pipeline/pipeline.h"

namespace sorbet::realmain::lsp {
using namespace std;
namespace {

void sendTypecheckInfo(const LSPConfiguration &config, const core::GlobalState &gs, SorbetTypecheckRunStatus status,
                       TypecheckingPath typecheckingPath, vector<core::FileRef> filesTypechecked) {
    if (config.getClientConfig().enableTypecheckInfo) {
        auto sorbetTypecheckInfo =
            make_unique<SorbetTypecheckRunInfo>(status, typecheckingPath, config.frefsToPaths(gs, filesTypechecked));
        config.output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::SorbetTypecheckRunInfo, move(sorbetTypecheckInfo))));
    }
}

vector<ast::ParsedFile> sortParsedFiles(const core::GlobalState &gs, ErrorReporter &errorReporter,
                                        vector<ast::ParsedFile> parsedFiles) {
    fast_sort(parsedFiles, [&](const auto &lhs, const auto &rhs) -> bool {
        auto lhsEpoch = max(errorReporter.lastDiagnosticEpochForFile(lhs.file), lhs.file.data(gs).epoch);
        auto rhsEpoch = max(errorReporter.lastDiagnosticEpochForFile(rhs.file), rhs.file.data(gs).epoch);

        return lhsEpoch > rhsEpoch;
    });

    return parsedFiles;
}
} // namespace

LSPTypechecker::LSPTypechecker(shared_ptr<const LSPConfiguration> config,
                               shared_ptr<core::lsp::PreemptionTaskManager> preemptManager)
    : typecheckerThreadId(this_thread::get_id()), config(move(config)), preemptManager(move(preemptManager)),
      errorReporter(make_shared<ErrorReporter>(this->config)) {}

LSPTypechecker::~LSPTypechecker() {}

void LSPTypechecker::initialize(TaskQueue &queue, unique_ptr<core::GlobalState> initialGS,
                                unique_ptr<KeyValueStore> kvstore, WorkerPool &workers,
                                const LSPConfiguration &currentConfig) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    ENFORCE(!this->initialized);

    LSPFileUpdates updates;

    // We should always initialize with epoch 0.
    updates.epoch = 0;
    updates.typecheckingPath = TypecheckingPath::Slow;
    this->gs = std::move(initialGS);

    // Initialize settings that come through from the client.
    this->gs->trackUntyped = currentConfig.getClientConfig().initialEnableHighlightUntyped;

    this->gs->highlightUntypedDiagnosticSeverity =
        convertDiagnosticSeverity(currentConfig.getClientConfig().initialHighlightUntypedDiagnosticSeverity);

    // Initialization typecheck is not cancelable.
    // TODO(jvilk): Make it preemptible.
    {
        const bool isIncremental = false;
        ErrorEpoch epoch(*errorReporter, updates.epoch, isIncremental, {});
        auto errorFlusher = make_shared<ErrorFlusherLSP>(updates.epoch, errorReporter);
        auto committed = runSlowPath(updates, cache::ownIfUnchanged(*this->gs, std::move(kvstore)), workers,
                                     errorFlusher, SlowPathMode::Init);
        ENFORCE(committed);
        epoch.committed = true;
    }

    // Unblock the indexer now that its state is fully initialized.
    {
        // We copy our file table over to the indexer to ensure that we start with a consistent view of the workspace:
        // the indexer uses presence in the file table to determine if a file is new or not.
        auto files = gs->getFiles().subspan(1);
        vector<shared_ptr<core::File>> indexerFiles{files.begin(), files.end()};

        absl::MutexLock lck{queue.getMutex()};

        // ensure that the next task we process initializes the indexer
        auto initTask = make_unique<IndexerInitializationTask>(*config, std::move(indexerFiles));
        queue.tasks().push_front(std::move(initTask));
    }

    config->logger->error("Resuming");
}

bool LSPTypechecker::typecheck(unique_ptr<LSPFileUpdates> updates, WorkerPool &workers,
                               vector<unique_ptr<Timer>> diagnosticLatencyTimers) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    ENFORCE(this->initialized);
    if (updates->canceledSlowPath) {
        // This update canceled the last slow path, so we should have undo state to restore to go to the point _before_
        // that slow path. This should always be the case, but let's not crash release builds.
        ENFORCE(cancellationUndoState != nullptr);
        if (cancellationUndoState != nullptr) {
            // Require that the undo state was restored when the slow path was canceled -- we only need it to be present
            // for the epoch at this point.
            ENFORCE(cancellationUndoState->getEvictedGs() == nullptr);

            // Prune the new files from list of files to be re-typechecked
            vector<core::FileRef> oldFilesWithErrors;
            uint32_t maxFileId = gs->getFiles().size();
            for (auto &file : errorReporter->filesWithErrorsSince(cancellationUndoState->epoch)) {
                if (file.id() < maxFileId) {
                    oldFilesWithErrors.push_back(file);
                }
            }

            cancellationUndoState = nullptr;
            auto fastPathDecision = updates->typecheckingPath;
            // Retypecheck all of the files that previously had errors.
            updates->mergeOlder(*getNoopUpdate(oldFilesWithErrors));
            // The merge operation resets `fastPathDecision`, but we know that retypechecking unchanged files
            // has no influence on the fast path decision.
            updates->typecheckingPath = fastPathDecision;
        } else {
            config->logger->debug("[Typechecker] Error: UndoState is missing for update that canceled slow path!");
        }
    }

    vector<core::FileRef> filesTypechecked;
    bool committed = true;
    const bool isFastPath = updates->typecheckingPath == TypecheckingPath::Fast;
    sendTypecheckInfo(*config, *gs, SorbetTypecheckRunStatus::Started, updates->typecheckingPath, {});
    {
        ErrorEpoch epoch(*errorReporter, updates->epoch, isFastPath, move(diagnosticLatencyTimers));

        auto errorFlusher = make_shared<ErrorFlusherLSP>(updates->epoch, errorReporter);
        if (isFastPath) {
            bool isNoopUpdateForRetypecheck = false;
            filesTypechecked = runFastPath(*updates, workers, errorFlusher, isNoopUpdateForRetypecheck);

            ENFORCE(updates->updatedFiles.empty());

            for (auto &ast : updates->updatedFinalGSFileIndexes) {
                this->indexedFinalGS[ast.file.id()] = move(ast);
            }

            prodCategoryCounterInc("lsp.updates", "fastpath");
        } else {
            committed = runSlowPath(*updates, this->getKvStore(), workers, errorFlusher, SlowPathMode::Cancelable);
        }
        epoch.committed = committed;
    }

    sendTypecheckInfo(*config, *gs, committed ? SorbetTypecheckRunStatus::Ended : SorbetTypecheckRunStatus::Cancelled,
                      updates->typecheckingPath, move(filesTypechecked));
    return committed;
}

vector<core::FileRef> LSPTypechecker::runFastPath(LSPFileUpdates &updates, WorkerPool &workers,
                                                  shared_ptr<core::ErrorFlusher> errorFlusher,
                                                  bool isNoopUpdateForRetypecheck) const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    ENFORCE(this->initialized);
    // We assume gs has been through the slow path, which is guaranteed by the initialization path not being cancelable.
    ENFORCE(gs->lspTypecheckCount > 0,
            "Tried to run fast path with a GlobalState object that never had inferencer and resolver runs.");
    // This property is set to 'true' in tests only if the update is expected to take the slow path and get cancelled.
    ENFORCE(!updates.cancellationExpected);
    ENFORCE(updates.preemptionsExpected == 0);
    // This path only works for fast path updates.
    ENFORCE(updates.typecheckingPath == TypecheckingPath::Fast);

    Timer timeit(config->logger, "fast_path");

    auto shouldRunIncrementalNamer = updates.fastPathUseIncrementalNamer;
    if (shouldRunIncrementalNamer) {
        timeit.setTag("path_chosen", "fast+incremental");
    } else {
        ENFORCE(updates.fastPathExtraFiles.empty());
        timeit.setTag("path_chosen", "fast");
    }

    // Replace error queue with one that is owned by this thread.
    gs->errorQueue = make_shared<core::ErrorQueue>(gs->errorQueue->logger, gs->errorQueue->tracer, errorFlusher);

    vector<core::FileRef> toTypecheck;
    toTypecheck.reserve(updates.fastPathExtraFiles.size() + updates.updatedFiles.size());
    for (auto &path : updates.fastPathExtraFiles) {
        auto fref = gs->findFileByPath(path);
        ENFORCE(fref.exists());
        toTypecheck.emplace_back(fref);
    }

    config->logger->debug("Added {} files that were not part of the edit to the update set", toTypecheck.size());
    UnorderedMap<core::FileRef, shared_ptr<const core::FileHash>> oldFoundHashesForFiles;
    for (auto &file : updates.updatedFiles) {
        auto fref = gs->findFileByPath(file->path());
        ENFORCE(fref.exists(), "New files are not supported in the fast path");

        auto oldFile = gs->replaceFile(fref, std::move(file));

        if (shouldRunIncrementalNamer) {
            // Only set oldFoundHashesForFiles if we're processing a real edit.
            //
            // This means that no-op edits (and thus calls to LSPTypechecker::retypecheck) don't
            // blow away methods only to redefine them with different IDs. retypecheck powers things
            // like hover and codeAction and is thus very common.
            //
            // A previous approach here was to check whether result.changedSymbolNameHashes is empty.
            // But the incremental namer is actually the *only* way Sorbet correctly handles fast
            // path edits to files with type_member's (previously, type_member-related errors would
            // disappear when making e.g. a fast path edit inside a method body).
            //
            // This does mean that runFastPath for retypecheck will fail to report those type member
            // errors (because no-op edits will not run incremental namer), but that's fine because
            // GlobalState doesn't change for no-op edits, and retypecheck already drops all errors.
            oldFoundHashesForFiles.emplace(fref, oldFile->getFileHash());
        }

        // If file doesn't have a typed: sigil, then we need to ensure it's typechecked using typed: false.
        fref.data(*gs).strictLevel = pipeline::decideStrictLevel(*gs, fref, config->opts);

        toTypecheck.emplace_back(fref);
    }

    updates.updatedFiles.clear();

    if (shouldRunIncrementalNamer && gs->packageDB().enabled()) {
        vector<core::FileRef> packageFiles;

        for (auto fref : toTypecheck) {
            // Only need to re-run packager if we're going to delete constants and have to re-define
            // their visibility, which only happens if we're running incrementalNamer.
            // NOTE: Using `gs` to access package information here assumes that edits to __package.rb
            // files don't take the fast path. We'll want (or maybe need) to revisit this when we start
            // making edits to `__package.rb` take fast paths.
            if (!(fref.data(*gs).isPackage(*gs))) {
                auto pkg = gs->packageDB().getPackageNameForFile(fref);
                if (pkg.exists()) {
                    // Since even no-op (e.g. whitespace-only) edits will cause constants to be deleted
                    // and re-added, we have to add the __package.rb files to set of files to retypecheck
                    // so that we can re-run PropagateVisibility to set export bits for any constants.
                    // TODO(jez) Make PropagateVisibility a function of the packageDB so that we
                    // don't have to completely retypecheck the `__package.rb` file again just to
                    // make sure one file's constants have the right visibility flags.
                    auto packageFref = gs->packageDB().getPackageInfo(pkg).fullLoc().file();
                    if (!packageFref.exists()) {
                        continue;
                    }

                    packageFiles.emplace_back(packageFref);
                    if (!oldFoundHashesForFiles.contains(fref)) {
                        oldFoundHashesForFiles.emplace(fref, fref.data(*gs).getFileHash());
                    }
                }
            }
        }

        toTypecheck.insert(toTypecheck.end(), packageFiles.begin(), packageFiles.end());
    }

    fast_sort(toTypecheck);
    toTypecheck.erase(std::unique(toTypecheck.begin(), toTypecheck.end()), toTypecheck.end());

    optional<ShowOperation> op;
    if (toTypecheck.size() > config->opts.lspMaxFilesOnFastPath / 2) {
        op.emplace(*config, ShowOperation::Kind::FastPath);
    }
    ENFORCE(gs->errorQueue->isEmpty());
    vector<ast::ParsedFile> updatedIndexed;
    for (core::FileRef fref : toTypecheck) {
        auto t = pipeline::indexOne(config->opts, *gs, fref);

        // As the fast-path might pull in unrelated files in the incremental case, we make sure to only cache files that
        // are explicitly open in the editor.
        if (fref.data(*gs).isOpenInClient()) {
            updates.updatedFinalGSFileIndexes.push_back(ast::ParsedFile{t.tree.deepCopy(), t.file});
        }

        updatedIndexed.emplace_back(std::move(t));

        // See earlier in the method for an explanation of the isNoopUpdateForRetypecheck check here.
        if (shouldRunIncrementalNamer && !oldFoundHashesForFiles.contains(fref)) {
            // This is an extra file that we need to typecheck which was not part of the original
            // edited files, so whatever it happens to have in foundMethodHashes is still "old"
            // (but we can't use `move` to steal it like before, because we're not replacing the
            // whole file).
            oldFoundHashesForFiles.emplace(fref, fref.data(*gs).getFileHash());
        }
    }

    ENFORCE(gs->lspQuery.isEmpty());
    auto resolved = shouldRunIncrementalNamer
                        ? pipeline::incrementalResolve(*gs, move(updatedIndexed), std::move(oldFoundHashesForFiles),
                                                       config->opts, workers)
                        : pipeline::incrementalResolve(*gs, move(updatedIndexed), nullopt, config->opts, workers);
    auto sorted = sortParsedFiles(*gs, *errorReporter, move(resolved));
    const auto presorted = true;
    const auto cancelable = false;
    pipeline::typecheck(*gs, move(sorted), config->opts, workers, cancelable, nullopt, presorted);
    gs->lspTypecheckCount++;

    auto duration = timeit.setEndTime();
    std::string files;
    if (duration.usec > 1'000'000) {
        auto span = absl::MakeSpan(toTypecheck);
        const auto numFilesToShow = 10;
        files = fmt::format("{}", fmt::map_join(span.size() > numFilesToShow ? span.subspan(0, numFilesToShow) : span,
                                                ", ", [&](const auto fref) { return fref.data(*gs).path(); }));
        if (span.size() > numFilesToShow) {
            files += ", ...";
        }
    } else {
        files = "...";
    }
    auto isPreemption = gs->epochManager->getStatus().slowPathRunning;
    config->logger->debug(
        "Running fast path over num_files={} incrementalNamer={} preemption={} duration={} files=[{}]",
        toTypecheck.size(), shouldRunIncrementalNamer, isPreemption, duration.usec, files);

    return toTypecheck;
}

bool LSPTypechecker::runSlowPath(LSPFileUpdates &updates, unique_ptr<const OwnedKeyValueStore> ownedKvstore,
                                 WorkerPool &workers, shared_ptr<core::ErrorFlusher> errorFlusher,
                                 LSPTypechecker::SlowPathMode mode) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId,
            "runSlowPath can only be called from the typechecker thread.");

    const bool cancelable = mode == SlowPathMode::Cancelable;

    auto &logger = config->logger;
    auto slowPathOp = make_optional<ShowOperation>(*config, ShowOperation::Kind::SlowPathBlocking);
    Timer timeit(logger, "slow_path");
    ENFORCE(updates.typecheckingPath != TypecheckingPath::Fast || config->disableFastPath);
    logger->debug("Taking slow path");

    UnorderedSet<core::FileRef> openFiles;
    ENFORCE(this->cancellationUndoState == nullptr);
    if (cancelable) {
        auto savedGS = std::exchange(this->gs, pipeline::copyForSlowPath(*this->gs, this->config->opts));

        // Seed open files with the previous set from `indexedFinalGS`
        for (auto &entry : this->indexedFinalGS) {
            openFiles.insert(entry.first);
        }

        this->cancellationUndoState =
            make_unique<UndoState>(std::move(savedGS), std::move(this->indexedFinalGS), updates.epoch);
    }

    const uint32_t epoch = updates.epoch;
    auto &epochManager = *this->gs->epochManager;
    // Note: Commits can only be canceled if this edit is cancelable, LSP is running across multiple threads, and the
    // cancelation feature is enabled.
    auto committed = epochManager.tryCommitEpoch(*this->gs, epoch, cancelable, preemptManager, [&]() -> void {
        vector<ast::ParsedFile> indexed, nonPackagedIndexed;

        {
            // Replace error queue with one that is owned by this thread.
            this->gs->errorQueue =
                make_shared<core::ErrorQueue>(this->gs->errorQueue->logger, this->gs->errorQueue->tracer, errorFlusher);

            optional<Timer> timeit;
            ShowOperation op(*config, ShowOperation::Kind::Indexing);

            switch (mode) {
                // Initialization fetches the list of files to index from the options
                case SlowPathMode::Init: {
                    ENFORCE(!this->initialized);
                    timeit.emplace(this->config->logger, "initial_index");

                    this->workspaceFiles = pipeline::reserveFiles(*this->gs, config->opts.inputFileNames);
                    break;
                }

                // Reindexing on the slow path derives the list of inputs files from the file table
                case SlowPathMode::Cancelable: {
                    timeit.emplace(this->config->logger, "slow_path_reindex");

                    // Determine which files we need to copy into the open files cache (indexedFinalGS), and update the
                    // file table to point to the updated files.
                    if (!updates.updatedFiles.empty()) {
                        core::UnfreezeFileTable updateFileTable{*this->gs};

                        for (auto &file : updates.updatedFiles) {
                            auto fref = this->gs->findFileByPath(file->path());
                            if (!fref.exists()) {
                                fref = this->gs->enterFile(std::move(file));
                            } else {
                                this->gs->replaceFile(fref, std::move(file));
                            }

                            // Not all files present in the update set will be from open files--some could be watchman
                            // update events, and others will be the result of a `textDocument/didClose` notification.
                            // As a result, we may need to remove entries from the set that was eagerly cloned from
                            // `indexedFinalGS`
                            if (fref.data(*this->gs).isOpenInClient()) {
                                openFiles.insert(fref);
                            } else {
                                openFiles.erase(fref);
                            }
                        }

                        updates.updatedFiles.clear();
                    }

                    this->workspaceFiles.clear();
                    this->workspaceFiles.reserve(this->gs->filesUsed());

                    // Rebuild the set of filerefs we're going to index. We're explicitly skipping the `0` file, as
                    // that's always a nullptr.
                    auto ix = 0;
                    for (const auto &file : this->gs->getFiles().subspan(1)) {
                        ++ix;

                        ENFORCE(file != nullptr);

                        switch (file->sourceType) {
                            case core::File::Type::NotYetRead:
                            case core::File::Type::Normal:
                                this->workspaceFiles.emplace_back(ix);
                                break;

                            case core::File::Type::PayloadGeneration:
                            case core::File::Type::Payload:
                            case core::File::Type::TombStone:
                                break;
                        }
                    }

                    break;
                }
            }

            ENFORCE(updates.updatedFiles.empty());
            ENFORCE(gs->lspQuery.isEmpty());

            // Test-only hook: Stall for as long as `slowPathBlocked` is set.
            {
                absl::MutexLock lck(&slowPathBlockedMutex);
                slowPathBlockedMutex.Await(absl::Condition(
                    +[](bool *slowPathBlocked) -> bool { return !*slowPathBlocked; }, &slowPathBlocked));
            }

            if (gs->sleepInSlowPathSeconds.has_value()) {
                auto sleepDuration = gs->sleepInSlowPathSeconds.value();
                for (int i = 0; i < sleepDuration * 10; i++) {
                    Timer::timedSleep(100ms, *logger, "slow_path.resolve.sleep");
                    if (epochManager.wasTypecheckingCanceled()) {
                        break;
                    }
                }
            }

            {
                Timer timeit(config->logger, "reIndexFromFileSystem");

                auto workspaceFilesSpan = absl::MakeSpan(this->workspaceFiles);
                if (this->config->opts.cacheSensitiveOptions.sorbetPackages) {
                    auto numPackageFiles = pipeline::partitionPackageFiles(*this->gs, workspaceFilesSpan);
                    auto inputPackageFiles = workspaceFilesSpan.first(numPackageFiles);
                    workspaceFilesSpan = workspaceFilesSpan.subspan(numPackageFiles);

                    {
                        auto result = hashing::Hashing::indexAndComputeFileHashes(
                            *this->gs, this->config->opts, *this->config->logger, inputPackageFiles, workers,
                            ownedKvstore, cancelable);
                        if (!result.hasResult()) {
                            return;
                        }
                        indexed = std::move(result.result());
                    }

                    // Only write the cache during initialization to avoid unbounded growth.
                    if (mode == SlowPathMode::Init) {
                        // Cache these before any packager rewrites, so that the cache is still
                        // usable regardless of whether `--sorbet-packages` was passed.
                        // Want to keep the kvstore around so we can still write to it later.
                        ownedKvstore = cache::ownIfUnchanged(
                            *this->gs,
                            cache::maybeCacheGlobalStateAndFiles(OwnedKeyValueStore::abort(std::move(ownedKvstore)),
                                                                 this->config->opts, *this->gs, workers, indexed));
                    }
                    this->cacheUpdatedFiles(indexed, openFiles);

                    // Only need to compute FoundDefHashes when running to compute a FileHash
                    auto foundHashes = nullptr;

                    // First namer run: only the __package.rb files. This populates the packageDB
                    auto cancelled =
                        pipeline::name(*this->gs, absl::MakeSpan(indexed), this->config->opts, workers, foundHashes);
                    if (cancelled) {
                        ast::ParsedFilesOrCancelled::cancel(move(indexed), workers);
                        return;
                    }

                    pipeline::buildPackageDB(*this->gs, absl::MakeSpan(indexed), workspaceFilesSpan, this->config->opts,
                                             workers);
                }

                {
                    auto result = hashing::Hashing::indexAndComputeFileHashes(*this->gs, this->config->opts,
                                                                              *this->config->logger, workspaceFilesSpan,
                                                                              workers, ownedKvstore, cancelable);
                    if (!result.hasResult()) {
                        ast::ParsedFilesOrCancelled::cancel(std::move(indexed), workers);
                        return;
                    }
                    nonPackagedIndexed = std::move(result.result());
                    this->cacheUpdatedFiles(nonPackagedIndexed, openFiles);
                }

                // Only write the cache during initialization to avoid unbounded growth.
                if (mode == SlowPathMode::Init) {
                    // Cache these before any packager rewrites, so that the cache is still usable
                    // regardless of whether `--sorbet-packages` was passed.
                    ownedKvstore = cache::ownIfUnchanged(
                        *this->gs, cache::maybeCacheGlobalStateAndFiles(
                                       OwnedKeyValueStore::abort(std::move(ownedKvstore)), this->config->opts,
                                       *this->gs, workers, nonPackagedIndexed));

                    // Close and copy the global kvstore, so that we have unique access for the rest of the session.
                    this->sessionCache =
                        cache::SessionCache::make(std::move(ownedKvstore), *this->config->logger, this->config->opts);

                    this->initialized = true;
                } else {
                    // We don't write in the cancelable slow path, and all our read operations have completed.
                    OwnedKeyValueStore::abort(std::move(ownedKvstore));
                }

                // Second namer run: all the other files (the packageDB shouldn't change)
                auto foundHashes = nullptr;
                auto canceled = pipeline::name(*gs, absl::Span<ast::ParsedFile>(nonPackagedIndexed), config->opts,
                                               workers, foundHashes);
                if (canceled) {
                    ast::ParsedFilesOrCancelled::cancel(move(indexed), workers);
                    ast::ParsedFilesOrCancelled::cancel(move(nonPackagedIndexed), workers);
                    return;
                }
                pipeline::validatePackagedFiles(*this->gs, absl::MakeSpan(nonPackagedIndexed), this->config->opts,
                                                workers);
            }
        } // Indexing+namer is done at this point

        if (epochManager.wasTypecheckingCanceled()) {
            return;
        }

        pipeline::unpartitionPackageFiles(indexed, std::move(nonPackagedIndexed));
        // TODO(jez) At this point, it's not correct to call it `indexed` anymore: we've run namer too

        auto maybeResolved = pipeline::resolve(*gs, move(indexed), config->opts, workers);
        if (!maybeResolved.hasResult()) {
            return;
        }

        if (gs->sleepInSlowPathSeconds.has_value()) {
            auto sleepDuration = gs->sleepInSlowPathSeconds.value();
            for (int i = 0; i < sleepDuration * 10; i++) {
                Timer::timedSleep(100ms, *logger, "slow_path.resolve.sleep");
                if (epochManager.wasTypecheckingCanceled()) {
                    break;
                }
            }
        }

        // Inform the fast path that this global state is OK for typechecking as resolution has completed.
        gs->lspTypecheckCount++;
        // TODO(jvilk): Remove conditional once initial typecheck is preemptible.
        if (cancelable) {
            // Inform users that Sorbet should be responsive now.
            // Explicitly end previous operation before beginning next operation.
            slowPathOp.emplace(*config, ShowOperation::Kind::SlowPathNonBlocking);
        }
        // Report how long the slow path blocks preemption.
        timeit.clone("slow_path.blocking_time");

        // [Test only] Wait for a preemption if one is expected.
        while (updates.preemptionsExpected > 0) {
            auto loopStartTime = Timer::clock_gettime_coarse();
            auto coarseThreshold = Timer::get_clock_threshold_coarse();
            while (!preemptManager->tryRunScheduledPreemptionTask(*gs)) {
                auto curTime = Timer::clock_gettime_coarse();
                if (curTime.usec - loopStartTime.usec > 20'000'000) {
                    Exception::raise("Slow path timed out waiting for preemption edit");
                }
                Timer::timedSleep(coarseThreshold, *logger, "slow_path.expected_preemption.sleep");
            }
            updates.preemptionsExpected--;
        }

        // [Test only] Wait for a cancellation if one is expected.
        if (updates.cancellationExpected) {
            auto loopStartTime = Timer::clock_gettime_coarse();
            auto coarseThreshold = Timer::get_clock_threshold_coarse();
            while (!epochManager.wasTypecheckingCanceled()) {
                auto curTime = Timer::clock_gettime_coarse();
                if (curTime.usec - loopStartTime.usec > 20'000'000) {
                    Exception::raise("Slow path timed out waiting for cancellation edit");
                }
                Timer::timedSleep(coarseThreshold, *logger, "slow_path.expected_cancellation.sleep");
            }
            return;
        }

        auto sorted = sortParsedFiles(*gs, *errorReporter, move(maybeResolved.result()));
        const auto presorted = true;
        pipeline::typecheck(*gs, move(sorted), config->opts, workers, cancelable, preemptManager, presorted);
    });

    gs->lspQuery = core::lsp::Query::noQuery();

    if (committed) {
        prodCategoryCounterInc("lsp.updates", "slowpath");
        timeit.setTag("canceled", "false");
        // No need to keep around cancelation state!
        cancellationUndoState = nullptr;
        logger->debug("[Typechecker] Typecheck run for epoch {} successfully finished.", updates.epoch);
    } else {
        prodCategoryCounterInc("lsp.updates", "slowpath_canceled");
        timeit.setTag("canceled", "true");
        ENFORCE(cancelable);
        // Eagerly restore the state to how it was before this slow path, so that we're not holding the old state for an
        // arbitrarily long time. The next update will be responsible for freeing the underlying UndoState after it
        // makes use of the epoch field to determine additional files to include in the edit.
        cancellationUndoState->restore(this->gs, this->indexedFinalGS);
        logger->debug("[Typechecker] Typecheck run for epoch {} was canceled.", updates.epoch);
    }

    return committed;
}

void LSPTypechecker::cacheUpdatedFiles(absl::Span<const ast::ParsedFile> indexed,
                                       const UnorderedSet<core::FileRef> &openFiles) {
    auto &logger = *config->logger;
    Timer timeit(logger, "slow_path.cache_open_files");

    for (auto &ast : indexed) {
        if (openFiles.contains(ast.file)) {
            this->indexedFinalGS[ast.file.id()] = ast::ParsedFile{ast.tree.deepCopy(), ast.file};
        }
    }
}

unique_ptr<core::GlobalState> LSPTypechecker::destroy() {
    this->sessionCache.reset();
    return move(gs);
}

namespace {
void tryApplyLocalVarSaver(const core::GlobalState &gs, vector<ast::ParsedFile> &indexedCopies) {
    auto lspQuery = get_if<core::lsp::Query::Var>(&gs.lspQuery.query);
    if (lspQuery == nullptr) {
        return;
    }
    for (auto &t : indexedCopies) {
        optional<resolver::ParsedSig> signature;
        auto ctx = core::Context(gs, core::Symbols::root(), t.file);
        if (t.file == lspQuery->enclosingLoc.file()) {
            // For a VAR query, gs.lspQuery.loc is the enclosing MethodDef's loc, which we can use
            // to find the signature before that MethodDef.
            auto queryLoc = lspQuery->enclosingLoc.copyWithZeroLength();
            auto result = sig_finder::SigFinder::findSignature(ctx, t.tree, queryLoc);
            if (result.has_value()) {
                signature = move(result->sig);
            }
        }
        LocalVarSaver localVarSaver(ctx.locAt(t.tree.loc()), move(signature), lspQuery->variable);
        ast::ConstTreeWalk::apply(ctx, localVarSaver, t.tree);
    }
}

void tryApplyDefLocSaver(const core::GlobalState &gs, vector<ast::ParsedFile> &indexedCopies) {
    if (holds_alternative<core::lsp::Query::Loc>(gs.lspQuery.query) &&
        holds_alternative<core::lsp::Query::Symbol>(gs.lspQuery.query)) {
        return;
    }
    for (auto &t : indexedCopies) {
        DefLocSaver defLocSaver;
        core::Context ctx(gs, core::Symbols::root(), t.file);
        ast::TreeWalk::apply(ctx, defLocSaver, t.tree);
    }
}
} // namespace

LSPQueryResult LSPTypechecker::query(const core::lsp::Query &q, const vector<core::FileRef> &filesForQuery,
                                     WorkerPool &workers) const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    // We assume gs has been through the slow path, which is guaranteed by the initialization path not being cancelable.
    ENFORCE(gs->lspTypecheckCount > 0,
            "Tried to run a query with a GlobalState object that never had inferencer and resolver runs.");

    // Replace error queue with one that is owned by this thread.
    auto queryCollector = make_shared<QueryCollector>();
    gs->errorQueue = make_shared<core::ErrorQueue>(gs->errorQueue->logger, gs->errorQueue->tracer, queryCollector);

    Timer timeit(config->logger, "query");
    prodCategoryCounterInc("lsp.updates", "query");
    ENFORCE(gs->errorQueue->isEmpty());
    ENFORCE(gs->lspQuery.isEmpty());
    gs->lspQuery = q;
    auto resolved = getResolved(filesForQuery, workers);
    tryApplyDefLocSaver(*gs, resolved);
    tryApplyLocalVarSaver(*gs, resolved);

    const auto cancelable = true;
    pipeline::typecheck(*gs, move(resolved), config->opts, workers, cancelable);
    gs->lspTypecheckCount++;
    gs->lspQuery = core::lsp::Query::noQuery();
    return LSPQueryResult{queryCollector->drainQueryResponses(), nullptr};
}

unique_ptr<LSPFileUpdates> LSPTypechecker::getNoopUpdate(absl::Span<const core::FileRef> frefs) const {
    auto result = make_unique<LSPFileUpdates>();
    auto &noop = *result;
    noop.typecheckingPath = TypecheckingPath::Fast;
    // Epoch isn't important for this update.
    noop.epoch = 0;
    for (auto fref : frefs) {
        ENFORCE(fref.exists());
        noop.updatedFiles.push_back(gs->getFiles()[fref.id()]);
    }
    return result;
}

vector<unique_ptr<core::Error>> LSPTypechecker::retypecheck(vector<core::FileRef> frefs, WorkerPool &workers) const {
    auto updates = getNoopUpdate(frefs);
    auto errorCollector = make_shared<core::ErrorCollector>();
    bool isNoopUpdateForRetypecheck = true;
    runFastPath(*updates, workers, errorCollector, isNoopUpdateForRetypecheck);

    return errorCollector->drainErrors();
}

ast::ExpressionPtr LSPTypechecker::getLocalVarTrees(core::FileRef fref) const {
    auto preserveConcreteSyntax = true;
    auto afterDesugar = pipeline::desugarOne(config->opts, *gs, fref, preserveConcreteSyntax);
    return local_vars::LocalVars::run(*gs, {move(afterDesugar), fref}).tree;
}

unique_ptr<OwnedKeyValueStore> LSPTypechecker::getKvStore() const {
    if (this->sessionCache == nullptr) {
        return nullptr;
    }

    auto kvstore = this->sessionCache->open(this->config->logger, this->config->opts);
    if (kvstore == nullptr) {
        return nullptr;
    }

    return make_unique<OwnedKeyValueStore>(std::move(kvstore));
}

vector<ast::ParsedFile> LSPTypechecker::getResolved(absl::Span<const core::FileRef> frefs, WorkerPool &workers) const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    vector<ast::ParsedFile> updatedIndexed;

    vector<core::FileRef> toIndex;
    toIndex.reserve(frefs.size());
    for (auto fref : frefs) {
        const auto id = fref.id();
        auto treeFinalGS = this->indexedFinalGS.find(id);
        if (treeFinalGS != this->indexedFinalGS.end()) {
            auto &indexed = treeFinalGS->second;
            if (indexed.tree) {
                updatedIndexed.emplace_back(ast::ParsedFile{indexed.tree.deepCopy(), indexed.file});
            }
        } else {
            ENFORCE(id < this->gs->filesUsed());
            toIndex.emplace_back(fref);
        }
    }

    if (!toIndex.empty()) {
        auto cancelable = false;
        auto kvstore = this->getKvStore();
        auto result = pipeline::index(*this->gs, toIndex, this->config->opts, workers, std::move(kvstore), cancelable);
        ENFORCE(result.hasResult());
        auto indexed = std::move(result.result());
        indexed.erase(
            std::remove_if(indexed.begin(), indexed.end(), [](auto &indexed) { return indexed.tree == nullptr; }),
            indexed.end());
        absl::c_move(std::move(indexed), std::back_inserter(updatedIndexed));
    }

    // There are two incrementalResolve modes: one when running for the purpose of processing a file update,
    // and one for running an LSP query on an already-resolved file.
    // In getResolved, we want the LSP query behavior, not the file update behavior, which we get by passing nullopt.
    auto foundHashesForFiles = nullopt;

    return pipeline::incrementalResolve(*gs, move(updatedIndexed), move(foundHashesForFiles), config->opts, workers);
}

ast::ParsedFile LSPTypechecker::getResolved(core::FileRef fref, WorkerPool &workers) const {
    vector<ast::ParsedFile> trees = this->getResolved(std::initializer_list<core::FileRef>{fref}, workers);

    if (trees.empty()) {
        // This case can happen if the associated file ref was to a payload file, in which case the tree will be
        // `nullptr` in `this->indexed` and not added to the vector produced by the multi-file version of
        // `this-fgetResolved`.
        return ast::ParsedFile{nullptr, fref};
    } else {
        ENFORCE(trees.size() == 1);
        return std::move(trees.front());
    }
}

const core::GlobalState &LSPTypechecker::state() const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    return *gs;
}

void LSPTypechecker::changeThread() {
    auto newId = this_thread::get_id();
    ENFORCE(newId != typecheckerThreadId);
    typecheckerThreadId = newId;
}

void LSPTypechecker::setSlowPathBlocked(bool blocked) {
    absl::MutexLock lck(&slowPathBlockedMutex);
    slowPathBlocked = blocked;
}

void LSPTypechecker::updateConfigAndGsFromOptions(const DidChangeConfigurationParams &options) const {
    this->gs->trackUntyped =
        LSPClientConfiguration::parseEnableHighlightUntyped(*options.settings, this->gs->trackUntyped);

    if (options.settings->highlightUntypedDiagnosticSeverity.has_value()) {
        this->gs->highlightUntypedDiagnosticSeverity =
            convertDiagnosticSeverity(options.settings->highlightUntypedDiagnosticSeverity.value());
    }

    if (options.settings->enableTypecheckInfo.has_value() ||
        options.settings->enableTypedFalseCompletionNudges.has_value() ||
        options.settings->supportsOperationNotifications.has_value() ||
        options.settings->supportsSorbetURIs.has_value()) {
        auto msg =
            "Currently `highlightUntyped` is the only updateable setting using the workspace/didChangeConfiguration "
            "notification";
        auto params = make_unique<ShowMessageParams>(MessageType::Warning, msg);
        config->output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::WindowShowMessage, move(params))));
    }
}

LSPTypecheckerDelegate::LSPTypecheckerDelegate(TaskQueue &queue, WorkerPool &workers, LSPTypechecker &typechecker)
    : typechecker(typechecker), queue{queue}, workers(workers) {}

void LSPTypecheckerDelegate::initialize(InitializedTask &task, unique_ptr<core::GlobalState> gs,
                                        unique_ptr<KeyValueStore> kvstore, const LSPConfiguration &currentConfig) {
    return typechecker.initialize(this->queue, std::move(gs), std::move(kvstore), this->workers, currentConfig);
}

void LSPTypecheckerDelegate::resumeTaskQueue(InitializedTask &task) {
    absl::MutexLock lck{this->queue.getMutex()};
    ENFORCE(this->queue.isPaused());
    this->queue.resume();
}

void LSPTypecheckerDelegate::typecheckOnFastPath(unique_ptr<LSPFileUpdates> updates,
                                                 vector<unique_ptr<Timer>> diagnosticLatencyTimers) {
    if (updates->typecheckingPath != TypecheckingPath::Fast) {
        Exception::raise("Tried to typecheck a slow path edit on the fast path.");
    }
    auto committed = typechecker.typecheck(std::move(updates), workers, move(diagnosticLatencyTimers));
    // Fast path edits can't be canceled.
    ENFORCE(committed);
}

vector<unique_ptr<core::Error>> LSPTypecheckerDelegate::retypecheck(vector<core::FileRef> frefs) const {
    return typechecker.retypecheck(frefs, workers);
}

LSPQueryResult LSPTypecheckerDelegate::query(const core::lsp::Query &q,
                                             const vector<core::FileRef> &filesForQuery) const {
    return typechecker.query(q, filesForQuery, workers);
}

vector<ast::ParsedFile> LSPTypecheckerDelegate::getResolved(absl::Span<const core::FileRef> frefs) const {
    return typechecker.getResolved(frefs, workers);
}

ast::ParsedFile LSPTypecheckerDelegate::getResolved(core::FileRef fref) const {
    return typechecker.getResolved(fref, workers);
}

ast::ExpressionPtr LSPTypecheckerDelegate::getLocalVarTrees(core::FileRef fref) const {
    return typechecker.getLocalVarTrees(fref);
}

const core::GlobalState &LSPTypecheckerDelegate::state() const {
    return typechecker.state();
}
void LSPTypecheckerDelegate::updateConfigAndGsFromOptions(const DidChangeConfigurationParams &options) const {
    typechecker.updateConfigAndGsFromOptions(options);
}

unique_ptr<LSPFileUpdates> LSPTypecheckerDelegate::getNoopUpdate(absl::Span<const core::FileRef> frefs) const {
    return typechecker.getNoopUpdate(frefs);
}

} // namespace sorbet::realmain::lsp
