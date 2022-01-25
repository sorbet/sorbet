#include "main/lsp/LSPTypechecker.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "ast/treemap/treemap.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/sort.h"
#include "core/ErrorCollector.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/DefLocSaver.h"
#include "main/lsp/ErrorFlusherLSP.h"
#include "main/lsp/ErrorReporter.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LocalVarSaver.h"
#include "main/lsp/QueryCollector.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/UndoState.h"
#include "main/lsp/json_types.h"
#include "main/pipeline/pipeline.h"

namespace sorbet::realmain::lsp {
using namespace std;
namespace {
void sendTypecheckInfo(const LSPConfiguration &config, const core::GlobalState &gs, SorbetTypecheckRunStatus status,
                       bool isFastPath, std::vector<core::FileRef> filesTypechecked) {
    if (config.getClientConfig().enableTypecheckInfo) {
        auto sorbetTypecheckInfo =
            make_unique<SorbetTypecheckRunInfo>(status, isFastPath, config.frefsToPaths(gs, filesTypechecked));
        config.output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::SorbetTypecheckRunInfo, move(sorbetTypecheckInfo))));
    }
}

// In debug builds, asserts that we have not accidentally taken the fast path after a change to the set of
// methods in a file.
bool validateMethodHashesHaveSameMethods(const std::vector<std::pair<core::NameHash, uint32_t>> &a,
                                         const std::vector<std::pair<core::NameHash, uint32_t>> &b) {
    if (a.size() != b.size()) {
        return false;
    }

    pair<core::NameHash, uint32_t> previousHash; // Initializes to <0, 0>.
    auto bIt = b.begin();
    for (const auto &methodA : a) {
        const auto &methodB = *bIt;
        if (methodA.first != methodB.first) {
            return false;
        }

        // Enforce that hashes are sorted in ascending order.
        if (methodA < previousHash) {
            return false;
        }

        previousHash = methodA;
        bIt++;
    }

    return true;
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

void LSPTypechecker::initialize(LSPFileUpdates updates, WorkerPool &workers) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    ENFORCE(!this->initialized);
    // We should always initialize with epoch 0.
    ENFORCE(updates.epoch == 0);
    this->initialized = true;
    indexed = move(updates.updatedFileIndexes);
    // Initialization typecheck is not cancelable.
    // TODO(jvilk): Make it preemptible.
    auto committed = false;
    {
        const bool isIncremental = false;
        ErrorEpoch epoch(*errorReporter, updates.epoch, isIncremental, {});
        committed = runSlowPath(move(updates), workers, /* cancelable */ false);
        epoch.committed = committed;
    }
    ENFORCE(committed);
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
            auto fastPathDecision = updates.canTakeFastPath;
            // Retypecheck all of the files that previously had errors.
            updates.mergeOlder(getNoopUpdate(oldFilesWithErrors));
            // The merge operation resets `fastPathDecision`, but we know that retypechecking unchanged files
            // has no influence on the fast path decision.
            updates.canTakeFastPath = fastPathDecision;
        } else {
            config->logger->debug("[Typechecker] Error: UndoState is missing for update that canceled slow path!");
        }
    }

    vector<core::FileRef> filesTypechecked;
    bool committed = true;
    const bool isFastPath = updates.canTakeFastPath;
    sendTypecheckInfo(*config, *gs, SorbetTypecheckRunStatus::Started, isFastPath, {});
    {
        ErrorEpoch epoch(*errorReporter, updates.epoch, isFastPath, move(diagnosticLatencyTimers));

        if (isFastPath) {
            filesTypechecked =
                runFastPath(updates, workers, make_shared<ErrorFlusherLSP>(updates.epoch, errorReporter));
            commitFileUpdates(updates, /* cancelable */ false);
            prodCategoryCounterInc("lsp.updates", "fastpath");
        } else {
            committed = runSlowPath(move(updates), workers, /* cancelable */ true);
        }
        epoch.committed = committed;
    }

    sendTypecheckInfo(*config, *gs, committed ? SorbetTypecheckRunStatus::Ended : SorbetTypecheckRunStatus::Cancelled,
                      isFastPath, move(filesTypechecked));
    return committed;
}

vector<core::FileRef> LSPTypechecker::runFastPath(LSPFileUpdates &updates, WorkerPool &workers,
                                                  shared_ptr<core::ErrorFlusher> errorFlusher) const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    ENFORCE(this->initialized);
    // We assume gs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(gs->lspTypecheckCount > 0,
            "Tried to run fast path with a GlobalState object that never had inferencer and resolver runs.");
    // This property is set to 'true' in tests only if the update is expected to take the slow path and get cancelled.
    ENFORCE(!updates.cancellationExpected);
    ENFORCE(updates.preemptionsExpected == 0);
    // This path only works for fast path updates.
    ENFORCE(updates.canTakeFastPath);

    Timer timeit("fast_path");
    vector<core::FileRef> subset;
    vector<core::NameHash> changedHashes;
    // Replace error queue with one that is owned by this thread.
    gs->errorQueue = make_shared<core::ErrorQueue>(gs->errorQueue->logger, gs->errorQueue->tracer, errorFlusher);
    {
        vector<pair<core::NameHash, uint32_t>> changedMethodHashes;
        for (auto &f : updates.updatedFiles) {
            auto fref = gs->findFileByPath(f->path());
            // We don't support new files on the fast path. This enforce failing indicates a bug in our fast/slow
            // path logic in LSPPreprocessor.
            ENFORCE(fref.exists());
            ENFORCE(f->getFileHash() != nullptr);
            if (f->sourceType == core::File::Type::Package) {
                // Only relevant in --stripe-packages mode. Package declarations do not have method
                // hashes. Instead we rely on recomputing packages if any __package.rb source
                // changes.
                continue;
            }
            if (fref.exists()) {
                // Update to existing file on fast path
                ENFORCE(fref.data(*gs).getFileHash() != nullptr);
                const auto &oldMethodHashes = fref.data(*gs).getFileHash()->definitions.methodHashes;
                const auto &newMethodHashes = f->getFileHash()->definitions.methodHashes;

                // Both oldHash and newHash should have the same methods, since this is the fast path!
                ENFORCE(validateMethodHashesHaveSameMethods(oldMethodHashes, newMethodHashes),
                        "definitionHash should have failed");

                // Find which hashes changed. Note: methodHashes are sorted, so set_difference should work.
                // This will insert two entries into `changedMethodHashes` for each changed method, but they will get
                // deduped later.
                set_difference(oldMethodHashes.begin(), oldMethodHashes.end(), newMethodHashes.begin(),
                               newMethodHashes.end(), inserter(changedMethodHashes, changedMethodHashes.begin()));

                gs->replaceFile(fref, f);
                // If file doesn't have a typed: sigil, then we need to ensure it's typechecked using typed: false.
                fref.data(*gs).strictLevel = pipeline::decideStrictLevel(*gs, fref, config->opts);
                subset.emplace_back(fref);
            }
        }

        changedHashes.reserve(changedMethodHashes.size());
        for (auto &changedMethodHash : changedMethodHashes) {
            changedHashes.push_back(changedMethodHash.first);
        }
        core::NameHash::sortAndDedupe(changedHashes);
    }

    int i = -1;
    // N.B.: We'll iterate over the changed files, too, but it's benign if we re-add them since we dedupe `subset`.
    for (auto &oldFile : gs->getFiles()) {
        i++;
        if (oldFile == nullptr) {
            continue;
        }

        if (oldFile->sourceType == core::File::Type::Package) {
            continue; // See note above about --stripe-packages.
        }

        ENFORCE(oldFile->getFileHash() != nullptr);
        const auto &oldHash = *oldFile->getFileHash();
        vector<core::NameHash> intersection;
        std::set_intersection(changedHashes.begin(), changedHashes.end(), oldHash.usages.sends.begin(),
                              oldHash.usages.sends.end(), std::back_inserter(intersection));
        if (!intersection.empty()) {
            auto ref = core::FileRef(i);
            config->logger->debug("Added {} to update set as used a changed method",
                                  !ref.exists() ? "" : ref.data(*gs).path());
            subset.emplace_back(ref);
        }
    }
    // Remove any duplicate files.
    fast_sort(subset);
    subset.resize(std::distance(subset.begin(), std::unique(subset.begin(), subset.end())));

    config->logger->debug("Taking fast path");
    ENFORCE(gs->errorQueue->isEmpty());
    vector<ast::ParsedFile> updatedIndexed;
    for (auto &f : subset) {
        // TODO(jvilk): We don't need to re-index files that didn't change.
        auto t = pipeline::indexOne(config->opts, *gs, f);
        updatedIndexed.emplace_back(ast::ParsedFile{t.tree.deepCopy(), t.file});
        updates.updatedFinalGSFileIndexes.push_back(move(t));
    }

    ENFORCE(gs->lspQuery.isEmpty());
    auto resolved = pipeline::incrementalResolve(*gs, move(updatedIndexed), config->opts);
    auto sorted = sortParsedFiles(*gs, *errorReporter, move(resolved));
    pipeline::typecheck(gs, move(sorted), config->opts, workers, /*presorted*/ true);
    gs->lspTypecheckCount++;

    return subset;
}

namespace {
pair<unique_ptr<core::GlobalState>, ast::ParsedFile>
updateFile(unique_ptr<core::GlobalState> gs, const shared_ptr<core::File> &file, const options::Options &opts) {
    core::FileRef fref = gs->findFileByPath(file->path());
    if (fref.exists()) {
        gs->replaceFile(fref, file);
    } else {
        fref = gs->enterFile(file);
    }
    fref.data(*gs).strictLevel = pipeline::decideStrictLevel(*gs, fref, opts);
    return make_pair(move(gs), pipeline::indexOne(opts, *gs, fref));
}
} // namespace

bool LSPTypechecker::copyIndexed(WorkerPool &workers, const UnorderedSet<int> &ignore,
                                 vector<ast::ParsedFile> &out) const {
    auto &logger = *config->logger;
    Timer timeit("slow_path.copy_indexes");
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

bool LSPTypechecker::runSlowPath(LSPFileUpdates updates, WorkerPool &workers, bool cancelable) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId,
            "runSlowPath can only be called from the typechecker thread.");

    auto &logger = config->logger;
    unique_ptr<ShowOperation> slowPathOp = make_unique<ShowOperation>(*config, ShowOperation::Kind::SlowPathBlocking);
    Timer timeit("slow_path");
    ENFORCE(!updates.canTakeFastPath || config->disableFastPath);
    ENFORCE(updates.updatedGS.has_value());
    if (!updates.updatedGS.has_value()) {
        Exception::raise("runSlowPath called with an update that lacks an updated global state.");
    }
    logger->debug("Taking slow path");

    auto finalGS = move(updates.updatedGS.value());
    const uint32_t epoch = updates.epoch;
    // Replace error queue with one that is owned by this thread.
    finalGS->errorQueue = make_shared<core::ErrorQueue>(finalGS->errorQueue->logger, finalGS->errorQueue->tracer,
                                                        make_shared<ErrorFlusherLSP>(epoch, errorReporter));
    auto &epochManager = *finalGS->epochManager;
    // Note: Commits can only be canceled if this edit is cancelable, LSP is running across multiple threads, and the
    // cancelation feature is enabled.
    const bool committed = epochManager.tryCommitEpoch(*finalGS, epoch, cancelable, preemptManager, [&]() -> void {
        UnorderedSet<int> updatedFiles;
        vector<ast::ParsedFile> indexedCopies;

        // Index the updated files using finalGS.
        {
            core::UnfreezeFileTable fileTableAccess(*finalGS);
            for (auto &file : updates.updatedFiles) {
                auto pair = updateFile(move(finalGS), file, config->opts);
                finalGS = move(pair.first);
                auto &ast = pair.second;
                if (ast.tree) {
                    indexedCopies.emplace_back(ast::ParsedFile{ast.tree.deepCopy(), ast.file});
                    updatedFiles.insert(ast.file.id());
                }
                updates.updatedFinalGSFileIndexes.push_back(move(ast));
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
        if (gs->sleepInSlowPath) {
            Timer::timedSleep(3000ms, *logger, "slow_path.resolve.sleep");
        }
        auto maybeResolved = pipeline::resolve(gs, move(indexedCopies), config->opts, workers);
        if (!maybeResolved.hasResult()) {
            return;
        }

        auto &resolved = maybeResolved.result();
        for (auto &tree : resolved) {
            ENFORCE(tree.file.exists());
        }
        if (gs->sleepInSlowPath) {
            Timer::timedSleep(3000ms, *logger, "slow_path.typecheck.sleep");
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
            while (!preemptManager->tryRunScheduledPreemptionTask(*gs)) {
                Timer::timedSleep(1ms, *logger, "slow_path.expected_preemption.sleep");
            }
            updates.preemptionsExpected--;
        }

        // [Test only] Wait for a cancellation if one is expected.
        if (updates.cancellationExpected) {
            while (!epochManager.wasTypecheckingCanceled()) {
                Timer::timedSleep(1ms, *logger, "slow_path.expected_cancellation.sleep");
            }
            return;
        }

        auto sorted = sortParsedFiles(*gs, *errorReporter, move(resolved));
        pipeline::typecheck(gs, move(sorted), config->opts, workers, cancelable, preemptManager, /*presorted*/ true);
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
    ENFORCE(!(updates.canTakeFastPath && couldBeCanceled));
    if (couldBeCanceled) {
        ENFORCE(updates.updatedGS.has_value());
        cancellationUndoState = make_unique<UndoState>(move(gs), std::move(indexedFinalGS), updates.epoch);
    }

    // Clear out state associated with old finalGS.
    if (!updates.canTakeFastPath) {
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
        ENFORCE(!updates.canTakeFastPath);
        gs = move(updates.updatedGS.value());
    } else {
        ENFORCE(updates.canTakeFastPath);
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
        LocalVarSaver localVarSaver;
        core::Context ctx(gs, core::Symbols::root(), t.file);
        t.tree = ast::TreeMap::apply(ctx, localVarSaver, move(t.tree));
    }
}

void tryApplyDefLocSaver(const core::GlobalState &gs, vector<ast::ParsedFile> &indexedCopies) {
    if (gs.lspQuery.kind != core::lsp::Query::Kind::LOC && gs.lspQuery.kind != core::lsp::Query::Kind::SYMBOL) {
        return;
    }
    for (auto &t : indexedCopies) {
        DefLocSaver defLocSaver;
        core::Context ctx(gs, core::Symbols::root(), t.file);
        t.tree = ast::TreeMap::apply(ctx, defLocSaver, move(t.tree));
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

    Timer timeit("query");
    prodCategoryCounterInc("lsp.updates", "query");
    ENFORCE(gs->errorQueue->isEmpty());
    ENFORCE(gs->lspQuery.isEmpty());
    gs->lspQuery = q;
    auto resolved = getResolved(filesForQuery);
    tryApplyDefLocSaver(*gs, resolved);
    tryApplyLocalVarSaver(*gs, resolved);

    pipeline::typecheck(gs, move(resolved), config->opts, workers, /*presorted*/ true);
    gs->lspTypecheckCount++;
    gs->lspQuery = core::lsp::Query::noQuery();
    return LSPQueryResult{queryCollector->drainQueryResponses(), nullptr};
}

LSPFileUpdates LSPTypechecker::getNoopUpdate(std::vector<core::FileRef> frefs) const {
    LSPFileUpdates noop;
    noop.canTakeFastPath = true;
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
    runFastPath(updates, workers, errorCollector);

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
    return pipeline::incrementalResolve(*gs, move(updatedIndexed), config->opts);
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

LSPTypecheckerDelegate::LSPTypecheckerDelegate(WorkerPool &workers, LSPTypechecker &typechecker)
    : typechecker(typechecker), workers(workers) {}

void LSPTypecheckerDelegate::typecheckOnFastPath(LSPFileUpdates updates,
                                                 vector<unique_ptr<Timer>> diagnosticLatencyTimers) {
    if (!updates.canTakeFastPath) {
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
