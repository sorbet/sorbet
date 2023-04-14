#include "main/lsp/LSPTypechecker.h"
#include "LSPFileUpdates.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "ast/treemap/treemap.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/sort/sort.h"
#include "core/ErrorCollector.h"
#include "core/ErrorQueue.h"
#include "core/NullFlusher.h"
#include "core/Unfreeze.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "hashing/hashing.h"
#include "main/cache/cache.h"
#include "main/lsp/DefLocSaver.h"
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
#include "main/lsp/notifications/sorbet_resume.h"
#include "main/pipeline/pipeline.h"
#include "main/sig_finder/sig_finder.h"

namespace sorbet::realmain::lsp {
using namespace std;
namespace {

void sendTypecheckInfo(const LSPConfiguration &config, const core::GlobalState &gs, SorbetTypecheckRunStatus status,
                       TypecheckingPath typecheckingPath, std::vector<core::FileRef> filesTypechecked) {
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

LSPTypechecker::LSPTypechecker(std::shared_ptr<const LSPConfiguration> config,
                               shared_ptr<core::lsp::PreemptionTaskManager> preemptManager)
    : typecheckerThreadId(this_thread::get_id()), config(move(config)), preemptManager(move(preemptManager)),
      errorReporter(make_shared<ErrorReporter>(this->config)) {}

LSPTypechecker::~LSPTypechecker() {}

void LSPTypechecker::initialize(TaskQueue &queue, std::unique_ptr<core::GlobalState> initialGS,
                                std::unique_ptr<KeyValueStore> kvstore, WorkerPool &workers,
                                const LSPConfiguration &currentConfig) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    ENFORCE(!this->initialized);

    LSPFileUpdates updates;

    // Initialize the global state for the indexer
    {
        initialGS->trackUntyped = currentConfig.getClientConfig().enableHighlightUntyped;
        // Temporarily replace error queue, as it asserts that the same thread that created it uses it and we're
        // going to use it on typechecker thread for this one operation.
        auto savedErrorQueue = initialGS->errorQueue;
        initialGS->errorQueue = make_shared<core::ErrorQueue>(savedErrorQueue->logger, savedErrorQueue->tracer,
                                                              make_shared<core::NullFlusher>());

        vector<ast::ParsedFile> indexed;
        Timer timeit(config->logger, "initial_index");
        ShowOperation op(*config, ShowOperation::Kind::Indexing);
        vector<core::FileRef> inputFiles;
        unique_ptr<const OwnedKeyValueStore> ownedKvstore = cache::ownIfUnchanged(*initialGS, move(kvstore));
        {
            Timer timeit(config->logger, "reIndexFromFileSystem");
            inputFiles = pipeline::reserveFiles(initialGS, config->opts.inputFileNames);
            indexed.resize(initialGS->filesUsed());

            auto asts = hashing::Hashing::indexAndComputeFileHashes(initialGS, config->opts, *config->logger,
                                                                    inputFiles, workers, ownedKvstore);
            // asts are in fref order, but we (currently) don't index and compute file hashes for payload files, so
            // vector index != FileRef ID. Fix that by slotting them into `indexed`.
            for (auto &ast : asts) {
                int id = ast.file.id();
                ENFORCE_NO_TIMER(id < indexed.size());
                indexed[id] = move(ast);
            }
        }

        cache::maybeCacheGlobalStateAndFiles(OwnedKeyValueStore::abort(move(ownedKvstore)), config->opts, *initialGS,
                                             workers, indexed);

        ENFORCE_NO_TIMER(indexed.size() == initialGS->filesUsed());

        updates.epoch = 0;
        updates.typecheckingPath = TypecheckingPath::Slow;
        updates.updatedFileIndexes = move(indexed);
        updates.updatedGS = initialGS->deepCopy();

        // Restore error queue, as initialGS will be used on the LSPLoop thread from now on.
        initialGS->errorQueue = move(savedErrorQueue);
    }

    // We should always initialize with epoch 0.
    this->initialized = true;
    this->indexed = move(updates.updatedFileIndexes);
    // Initialization typecheck is not cancelable.
    // TODO(jvilk): Make it preemptible.
    auto committed = false;
    {
        const bool isIncremental = false;
        ErrorEpoch epoch(*errorReporter, updates.epoch, isIncremental, {});
        auto errorFlusher = make_shared<ErrorFlusherLSP>(updates.epoch, errorReporter);
        committed = runSlowPath(move(updates), workers, errorFlusher, /* cancelable */ false);
        epoch.committed = committed;
    }
    ENFORCE(committed);

    // Unblock the indexer now that its state is fully initialized.
    {
        absl::MutexLock lck{queue.getMutex()};

        // ensure that the next task we process initializes the indexer
        auto initTask = std::make_unique<IndexerInitializationTask>(*config, std::move(initialGS));
        queue.tasks().push_front(std::move(initTask));
    }

    config->logger->error("Resuming");
}

bool LSPTypechecker::typecheck(LSPFileUpdates updates, WorkerPool &workers,
                               vector<unique_ptr<Timer>> diagnosticLatencyTimers) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    ENFORCE(this->initialized);
    if (updates.canceledSlowPath) {
        // This update canceled the last slow path, so we should have undo state to restore to go to the point _before_
        // that slow path. This should always be the case, but let's not crash release builds.
        ENFORCE(cancellationUndoState != nullptr);
        if (cancellationUndoState != nullptr) {
            // Restore the previous globalState
            cancellationUndoState->restore(gs, indexed, indexedFinalGS);

            // Prune the new files from list of files to be re-typechecked
            vector<core::FileRef> oldFilesWithErrors;
            uint32_t maxFileId = gs->getFiles().size();
            for (auto &file : errorReporter->filesWithErrorsSince(cancellationUndoState->epoch)) {
                if (file.id() < maxFileId) {
                    oldFilesWithErrors.push_back(file);
                }
            }

            cancellationUndoState = nullptr;
            auto fastPathDecision = updates.typecheckingPath;
            // Retypecheck all of the files that previously had errors.
            updates.mergeOlder(getNoopUpdate(oldFilesWithErrors));
            // The merge operation resets `fastPathDecision`, but we know that retypechecking unchanged files
            // has no influence on the fast path decision.
            updates.typecheckingPath = fastPathDecision;
        } else {
            config->logger->debug("[Typechecker] Error: UndoState is missing for update that canceled slow path!");
        }
    }

    vector<core::FileRef> filesTypechecked;
    bool committed = true;
    const bool isFastPath = updates.typecheckingPath == TypecheckingPath::Fast;
    sendTypecheckInfo(*config, *gs, SorbetTypecheckRunStatus::Started, updates.typecheckingPath, {});
    {
        ErrorEpoch epoch(*errorReporter, updates.epoch, isFastPath, move(diagnosticLatencyTimers));

        auto errorFlusher = make_shared<ErrorFlusherLSP>(updates.epoch, errorReporter);
        if (isFastPath) {
            bool isNoopUpdateForRetypecheck = false;
            filesTypechecked = runFastPath(updates, workers, errorFlusher, isNoopUpdateForRetypecheck);
            commitFileUpdates(updates, /* cancelable */ false);
            prodCategoryCounterInc("lsp.updates", "fastpath");
        } else {
            committed = runSlowPath(move(updates), workers, errorFlusher, /* cancelable */ true);
        }
        epoch.committed = committed;
    }

    sendTypecheckInfo(*config, *gs, committed ? SorbetTypecheckRunStatus::Ended : SorbetTypecheckRunStatus::Cancelled,
                      updates.typecheckingPath, move(filesTypechecked));
    return committed;
}

vector<core::FileRef> LSPTypechecker::runFastPath(LSPFileUpdates &updates, WorkerPool &workers,
                                                  shared_ptr<core::ErrorFlusher> errorFlusher,
                                                  bool isNoopUpdateForRetypecheck) const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    ENFORCE(this->initialized);
    // We assume gs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(gs->lspTypecheckCount > 0,
            "Tried to run fast path with a GlobalState object that never had inferencer and resolver runs.");
    // This property is set to 'true' in tests only if the update is expected to take the slow path and get cancelled.
    ENFORCE(!updates.cancellationExpected);
    ENFORCE(updates.preemptionsExpected == 0);
    // This path only works for fast path updates.
    ENFORCE(updates.typecheckingPath == TypecheckingPath::Fast);

    Timer timeit(config->logger, "fast_path");
    // Replace error queue with one that is owned by this thread.
    gs->errorQueue = make_shared<core::ErrorQueue>(gs->errorQueue->logger, gs->errorQueue->tracer, errorFlusher);
    auto result = updates.fastPathFilesToTypecheck(*gs, *config);
    config->logger->debug("Added {} files that were not part of the edit to the update set", result.extraFiles.size());
    UnorderedMap<core::FileRef, core::FoundDefHashes> oldFoundHashesForFiles;
    auto toTypecheck = move(result.extraFiles);
    auto shouldRunIncrementalNamer = !result.changedSymbolNameHashes.empty();
    for (auto [fref, idx] : result.changedFiles) {
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

            // Okay to `move` here (steals component of getFileHash) because we're about to use
            // replaceFile to clobber fref.data(gs) anyways.
            oldFoundHashesForFiles.emplace(fref, move(fref.data(*gs).getFileHash()->foundHashes));
        }

        gs->replaceFile(fref, updates.updatedFiles[idx]);
        // If file doesn't have a typed: sigil, then we need to ensure it's typechecked using typed: false.
        fref.data(*gs).strictLevel = pipeline::decideStrictLevel(*gs, fref, config->opts);

        toTypecheck.emplace_back(fref);
    }

    UnorderedSet<core::FileRef> packageFiles;

    if (shouldRunIncrementalNamer) {
        for (auto fref : toTypecheck) {
            // Only need to re-run packager if we're going to delete constants and have to re-define
            // their visibility, which only happens if we're running incrementalNamer.
            // NOTE: Using `gs` to access package information here assumes that edits to __package.rb
            // files don't take the fast path. We'll want (or maybe need) to revisit this when we start
            // making edits to `__package.rb` take fast paths.
            if (!(fref.data(*gs).isPackage())) {
                auto &pkg = gs->packageDB().getPackageForFile(*gs, fref);
                if (pkg.exists()) {
                    // Since even no-op (e.g. whitespace-only) edits will cause constants to be deleted
                    // and re-added, we have to add the __package.rb files to set of files to retypecheck
                    // so that we can re-run PropagateVisibility to set export bits for any constants.
                    auto packageFref = pkg.fullLoc().file();
                    if (!packageFref.exists()) {
                        continue;
                    }

                    packageFiles.emplace(packageFref);
                }
            }
        }
    }

    for (auto packageFref : packageFiles) {
        if (result.changedFiles.find(packageFref) == result.changedFiles.end()) {
            toTypecheck.emplace_back(packageFref);
        }
    }

    fast_sort(toTypecheck);

    config->logger->debug("Running fast path over num_files={}", toTypecheck.size());
    unique_ptr<ShowOperation> op;
    if (toTypecheck.size() > config->opts.lspMaxFilesOnFastPath / 2) {
        op = make_unique<ShowOperation>(*config, ShowOperation::Kind::FastPath);
    }
    ENFORCE(gs->errorQueue->isEmpty());
    vector<ast::ParsedFile> updatedIndexed;
    for (auto &f : toTypecheck) {
        // TODO(jvilk): We don't need to re-index files that didn't change.
        // (`updates` has already-indexed trees, but they've been indexed with initialGS, not the
        // `*gs` that we'll be typechecking with. We could do an ast::Substitute here if we had
        // access to `initialGS`, but that's owned by the indexer thread, not this thread.)
        auto t = pipeline::indexOne(config->opts, *gs, f);
        updatedIndexed.emplace_back(ast::ParsedFile{t.tree.deepCopy(), t.file});
        updates.updatedFinalGSFileIndexes.push_back(move(t));

        // See earlier in the method for an explanation of the isNoopUpdateForRetypecheck check here.
        if (shouldRunIncrementalNamer && oldFoundHashesForFiles.find(f) == oldFoundHashesForFiles.end()) {
            // This is an extra file that we need to typecheck which was not part of the original
            // edited files, so whatever it happens to have in foundMethodHashes is still "old"
            // (but we can't use `move` to steal it like before, because we're not replacing the
            // whole file).
            oldFoundHashesForFiles.emplace(f, f.data(*gs).getFileHash()->foundHashes);
        }
    }

    ENFORCE(gs->lspQuery.isEmpty());
    auto resolved =
        shouldRunIncrementalNamer
            ? pipeline::incrementalResolve(*gs, move(updatedIndexed), std::move(oldFoundHashesForFiles), config->opts)
            : pipeline::incrementalResolve(*gs, move(updatedIndexed), nullopt, config->opts);
    auto sorted = sortParsedFiles(*gs, *errorReporter, move(resolved));
    const auto presorted = true;
    const auto cancelable = false;
    pipeline::typecheck(*gs, move(sorted), config->opts, workers, cancelable, std::nullopt, presorted);
    gs->lspTypecheckCount++;

    return toTypecheck;
}

namespace {
ast::ParsedFile updateFile(core::GlobalState &gs, const shared_ptr<core::File> &file, const options::Options &opts) {
    core::FileRef fref = gs.findFileByPath(file->path());
    if (fref.exists()) {
        gs.replaceFile(fref, file);
    } else {
        fref = gs.enterFile(file);
    }
    fref.data(gs).strictLevel = pipeline::decideStrictLevel(gs, fref, opts);
    return pipeline::indexOne(opts, gs, fref);
}
} // namespace

bool LSPTypechecker::copyIndexed(WorkerPool &workers, const UnorderedSet<int> &ignore,
                                 vector<ast::ParsedFile> &out) const {
    auto &logger = *config->logger;
    Timer timeit(logger, "slow_path.copy_indexes");
    shared_ptr<ConcurrentBoundedQueue<int>> fileq = make_shared<ConcurrentBoundedQueue<int>>(indexed.size());
    for (int i = 0; i < indexed.size(); i++) {
        fileq->push(i, 1);
    }

    const auto &epochManager = *gs->epochManager;
    shared_ptr<BlockingBoundedQueue<vector<ast::ParsedFile>>> resultq =
        make_shared<BlockingBoundedQueue<vector<ast::ParsedFile>>>(indexed.size());
    workers.multiplexJob("copyParsedFiles", [fileq, resultq, &indexed = this->indexed, &ignore, &epochManager]() {
        vector<ast::ParsedFile> threadResult;
        int processedByThread = 0;
        int job;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;

                    // Stop if typechecking was canceled.
                    if (!epochManager.wasTypecheckingCanceled()) {
                        const auto &tree = indexed[job];
                        // Note: indexed entries for payload files don't have any contents.
                        if (tree.tree && !ignore.contains(tree.file.id())) {
                            threadResult.emplace_back(ast::ParsedFile{tree.tree.deepCopy(), tree.file});
                        }
                    }
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });
    {
        vector<ast::ParsedFile> threadResult;
        out.reserve(indexed.size());
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger); !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger)) {
            if (result.gotItem()) {
                for (auto &copy : threadResult) {
                    out.push_back(move(copy));
                }
            }
        }
    }
    return !epochManager.wasTypecheckingCanceled();
}

bool LSPTypechecker::runSlowPath(LSPFileUpdates updates, WorkerPool &workers,
                                 shared_ptr<core::ErrorFlusher> errorFlusher, bool cancelable) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId,
            "runSlowPath can only be called from the typechecker thread.");

    auto &logger = config->logger;
    unique_ptr<ShowOperation> slowPathOp = make_unique<ShowOperation>(*config, ShowOperation::Kind::SlowPathBlocking);
    Timer timeit(logger, "slow_path");
    ENFORCE(updates.typecheckingPath != TypecheckingPath::Fast || config->disableFastPath);
    ENFORCE(updates.updatedGS.has_value());
    if (!updates.updatedGS.has_value()) {
        Exception::raise("runSlowPath called with an update that lacks an updated global state.");
    }
    logger->debug("Taking slow path");

    auto finalGS = move(updates.updatedGS.value());
    const uint32_t epoch = updates.epoch;
    // Replace error queue with one that is owned by this thread.
    finalGS->errorQueue =
        make_shared<core::ErrorQueue>(finalGS->errorQueue->logger, finalGS->errorQueue->tracer, errorFlusher);
    auto &epochManager = *finalGS->epochManager;
    // Note: Commits can only be canceled if this edit is cancelable, LSP is running across multiple threads, and the
    // cancelation feature is enabled.
    const bool committed = epochManager.tryCommitEpoch(*finalGS, epoch, cancelable, preemptManager, [&]() -> void {
        UnorderedSet<int> updatedFiles;
        vector<ast::ParsedFile> indexedCopies;

        // Index the updated files using finalGS.
        {
            auto &gs = *finalGS;
            core::UnfreezeFileTable fileTableAccess(gs);
            for (auto &file : updates.updatedFiles) {
                auto parsedFile = updateFile(gs, file, config->opts);
                if (parsedFile.tree) {
                    indexedCopies.emplace_back(ast::ParsedFile{parsedFile.tree.deepCopy(), parsedFile.file});
                    updatedFiles.insert(parsedFile.file.id());
                }
                updates.updatedFinalGSFileIndexes.push_back(move(parsedFile));
            }
        }

        // Before making preemption or cancelation possible, pre-commit the changes from this slow path so that
        // preempted queries can use them and the code after this lambda can assume that this step happened.
        updates.updatedGS = move(finalGS);
        commitFileUpdates(updates, cancelable);
        // We use `gs` rather than the moved `finalGS` from this point forward.

        // Copy the indexes of unchanged files.
        if (!copyIndexed(workers, updatedFiles, indexedCopies)) {
            // Canceled.
            return;
        }

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
        // Only need to compute FoundDefHashes when running to compute a FileHash
        auto foundHashes = nullptr;
        auto maybeResolved = pipeline::resolve(gs, move(indexedCopies), config->opts, workers, foundHashes);
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
            slowPathOp = nullptr;
            slowPathOp = make_unique<ShowOperation>(*config, ShowOperation::Kind::SlowPathNonBlocking);
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

    // Note: `gs` now holds the value of `finalGS`.
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
        // Update responsible will use state in `cancellationUndoState` to restore typechecker to the point before
        // this slow path.
        ENFORCE(cancelable);
        logger->debug("[Typechecker] Typecheck run for epoch {} was canceled.", updates.epoch);
    }
    return committed;
}

void LSPTypechecker::commitFileUpdates(LSPFileUpdates &updates, bool couldBeCanceled) {
    // The fast path cannot be canceled.
    ENFORCE(!(updates.typecheckingPath == TypecheckingPath::Fast && couldBeCanceled));
    if (couldBeCanceled) {
        ENFORCE(updates.updatedGS.has_value());
        cancellationUndoState = make_unique<UndoState>(move(gs), std::move(indexedFinalGS), updates.epoch);
    }

    // Clear out state associated with old finalGS.
    if (updates.typecheckingPath != TypecheckingPath::Fast) {
        indexedFinalGS.clear();
    }

    int i = -1;
    ENFORCE(updates.updatedFileIndexes.size() == updates.updatedFiles.size());
    for (auto &ast : updates.updatedFileIndexes) {
        i++;
        const int id = ast.file.id();
        if (id >= indexed.size()) {
            indexed.resize(id + 1);
        }
        if (cancellationUndoState != nullptr) {
            // Move the evicted values before they get replaced.
            cancellationUndoState->recordEvictedState(move(indexed[id]));
        }
        indexed[id] = move(ast);
    }

    for (auto &ast : updates.updatedFinalGSFileIndexes) {
        indexedFinalGS[ast.file.id()] = move(ast);
    }

    if (updates.updatedGS.has_value()) {
        ENFORCE(updates.typecheckingPath != TypecheckingPath::Fast);
        gs = move(updates.updatedGS.value());
    } else {
        ENFORCE(updates.typecheckingPath == TypecheckingPath::Fast);
    }
}

unique_ptr<core::GlobalState> LSPTypechecker::destroy() {
    return move(gs);
}

namespace {
void tryApplyLocalVarSaver(const core::GlobalState &gs, vector<ast::ParsedFile> &indexedCopies) {
    if (gs.lspQuery.kind != core::lsp::Query::Kind::VAR) {
        return;
    }
    for (auto &t : indexedCopies) {
        optional<resolver::ParsedSig> signature;
        auto ctx = core::Context(gs, core::Symbols::root(), t.file);
        if (t.file == gs.lspQuery.loc.file()) {
            // For a VAR query, gs.lspQuery.loc is the enclosing MethodDef's loc, which we can use
            // to find the signature before that MethodDef.
            auto queryLoc = gs.lspQuery.loc.copyWithZeroLength();
            signature = sig_finder::SigFinder::findSignature(ctx, t.tree, queryLoc);
        }
        LocalVarSaver localVarSaver(ctx.locAt(t.tree.loc()), move(signature));
        ast::TreeWalk::apply(ctx, localVarSaver, t.tree);
    }
}

void tryApplyDefLocSaver(const core::GlobalState &gs, vector<ast::ParsedFile> &indexedCopies) {
    if (gs.lspQuery.kind != core::lsp::Query::Kind::LOC && gs.lspQuery.kind != core::lsp::Query::Kind::SYMBOL) {
        return;
    }
    for (auto &t : indexedCopies) {
        DefLocSaver defLocSaver;
        core::Context ctx(gs, core::Symbols::root(), t.file);
        ast::TreeWalk::apply(ctx, defLocSaver, t.tree);
    }
}
} // namespace

LSPQueryResult LSPTypechecker::query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery,
                                     WorkerPool &workers) const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    // We assume gs is a copy of initialGS, which has had the inferencer & resolver run.
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
    auto resolved = getResolved(filesForQuery);
    tryApplyDefLocSaver(*gs, resolved);
    tryApplyLocalVarSaver(*gs, resolved);

    const auto cancelable = true;
    pipeline::typecheck(*gs, move(resolved), config->opts, workers, cancelable);
    gs->lspTypecheckCount++;
    gs->lspQuery = core::lsp::Query::noQuery();
    return LSPQueryResult{queryCollector->drainQueryResponses(), nullptr};
}

LSPFileUpdates LSPTypechecker::getNoopUpdate(std::vector<core::FileRef> frefs) const {
    LSPFileUpdates noop;
    noop.typecheckingPath = TypecheckingPath::Fast;
    // Epoch isn't important for this update.
    noop.epoch = 0;
    for (auto fref : frefs) {
        ENFORCE(fref.exists());
        ENFORCE(fref.id() < indexed.size());
        auto &index = indexed[fref.id()];
        // Note: `index.tree` can be null if the file is a stdlib file.
        noop.updatedFileIndexes.push_back({(index.tree ? index.tree.deepCopy() : nullptr), index.file});
        noop.updatedFiles.push_back(gs->getFiles()[fref.id()]);
    }
    return noop;
}

std::vector<std::unique_ptr<core::Error>> LSPTypechecker::retypecheck(vector<core::FileRef> frefs,
                                                                      WorkerPool &workers) const {
    LSPFileUpdates updates = getNoopUpdate(move(frefs));
    auto errorCollector = make_shared<core::ErrorCollector>();
    bool isNoopUpdateForRetypecheck = true;
    runFastPath(updates, workers, errorCollector, isNoopUpdateForRetypecheck);

    return errorCollector->drainErrors();
}

const ast::ParsedFile &LSPTypechecker::getIndexed(core::FileRef fref) const {
    const auto id = fref.id();
    auto treeFinalGS = indexedFinalGS.find(id);
    if (treeFinalGS != indexedFinalGS.end()) {
        return treeFinalGS->second;
    }
    ENFORCE(id < indexed.size());
    return indexed[id];
}

vector<ast::ParsedFile> LSPTypechecker::getResolved(const vector<core::FileRef> &frefs) const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    vector<ast::ParsedFile> updatedIndexed;

    for (auto fref : frefs) {
        auto &indexed = getIndexed(fref);
        if (indexed.tree) {
            updatedIndexed.emplace_back(ast::ParsedFile{indexed.tree.deepCopy(), indexed.file});
        }
    }

    // There are two incrementalResolve modes: one when running for the purpose of processing a file update,
    // and one for running an LSP query on an already-resolved file.
    // In getResolved, we want the LSP query behavior, not the file update behavior, which we get by passing nullopt.
    auto foundHashesForFiles = nullopt;

    return pipeline::incrementalResolve(*gs, move(updatedIndexed), move(foundHashesForFiles), config->opts);
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

LSPTypecheckerDelegate::LSPTypecheckerDelegate(TaskQueue &queue, WorkerPool &workers, LSPTypechecker &typechecker)
    : typechecker(typechecker), queue{queue}, workers(workers) {}

void LSPTypecheckerDelegate::initialize(InitializedTask &task, std::unique_ptr<core::GlobalState> gs,
                                        std::unique_ptr<KeyValueStore> kvstore, const LSPConfiguration &currentConfig) {
    return typechecker.initialize(this->queue, std::move(gs), std::move(kvstore), this->workers, currentConfig);
}

void LSPTypecheckerDelegate::resumeTaskQueue(InitializedTask &task) {
    absl::MutexLock lck{this->queue.getMutex()};
    ENFORCE(this->queue.isPaused());
    this->queue.resume();
}

void LSPTypecheckerDelegate::typecheckOnFastPath(LSPFileUpdates updates,
                                                 vector<unique_ptr<Timer>> diagnosticLatencyTimers) {
    if (updates.typecheckingPath != TypecheckingPath::Fast) {
        Exception::raise("Tried to typecheck a slow path edit on the fast path.");
    }
    auto committed = typechecker.typecheck(move(updates), workers, move(diagnosticLatencyTimers));
    // Fast path edits can't be canceled.
    ENFORCE(committed);
}

std::vector<std::unique_ptr<core::Error>> LSPTypecheckerDelegate::retypecheck(std::vector<core::FileRef> frefs) const {
    return typechecker.retypecheck(frefs, workers);
}

LSPQueryResult LSPTypecheckerDelegate::query(const core::lsp::Query &q,
                                             const std::vector<core::FileRef> &filesForQuery) const {
    return typechecker.query(q, filesForQuery, workers);
}

const ast::ParsedFile &LSPTypecheckerDelegate::getIndexed(core::FileRef fref) const {
    return typechecker.getIndexed(fref);
}

std::vector<ast::ParsedFile> LSPTypecheckerDelegate::getResolved(const std::vector<core::FileRef> &frefs) const {
    return typechecker.getResolved(frefs);
}

const core::GlobalState &LSPTypecheckerDelegate::state() const {
    return typechecker.state();
}

} // namespace sorbet::realmain::lsp
