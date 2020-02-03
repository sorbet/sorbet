#include "main/lsp/LSPTypechecker.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "ast/treemap/treemap.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "core/Unfreeze.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/DefLocSaver.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LocalVarFinder.h"
#include "main/lsp/LocalVarSaver.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/UndoState.h"
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
} // namespace

LSPTypechecker::LSPTypechecker(std::shared_ptr<const LSPConfiguration> config,
                               shared_ptr<core::lsp::PreemptionTaskManager> preemptManager)
    : typecheckerThreadId(this_thread::get_id()), config(move(config)), preemptManager(move(preemptManager)) {}

LSPTypechecker::~LSPTypechecker() {}

void LSPTypechecker::initialize(LSPFileUpdates updates, WorkerPool &workers) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    ENFORCE(!this->initialized);
    // We should always initialize with epoch 0.
    ENFORCE(updates.epoch == 0);
    this->initialized = true;
    globalStateHashes = move(updates.updatedFileHashes);
    indexed = move(updates.updatedFileIndexes);
    // Initialize to all zeroes.
    diagnosticEpochs = vector<u4>(globalStateHashes.size(), 0);
    // Initialization typecheck is not cancelable.
    // TODO(jvilk): Make it preemptible.
    auto committed = runSlowPath(move(updates), workers, /* cancelable */ false);
    ENFORCE(committed);
    ENFORCE(globalStateHashes.size() == indexed.size());
}

bool LSPTypechecker::typecheck(LSPFileUpdates updates, WorkerPool &workers) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    ENFORCE(this->initialized);
    if (updates.canceledSlowPath) {
        // This update canceled the last slow path, so we should have undo state to restore to go to the point _before_
        // that slow path. This should always be the case, but let's not crash release builds.
        ENFORCE(cancellationUndoState != nullptr);
        if (cancellationUndoState != nullptr) {
            // This is the typecheck that caused us to cancel the previous slow path. Un-commit all typechecker changes.
            auto oldFilesWithErrors =
                cancellationUndoState->restore(gs, indexed, indexedFinalGS, globalStateHashes, filesThatHaveErrors);
            cancellationUndoState = nullptr;
            // Retypecheck all of the files that previously had errors.
            updates.mergeOlder(getNoopUpdate(oldFilesWithErrors));
        } else {
            config->logger->debug("[Typechecker] Error: UndoState is missing for update that canceled slow path!");
        }
    }

    vector<core::FileRef> filesTypechecked;
    bool committed = true;
    const bool isFastPath = updates.canTakeFastPath;
    sendTypecheckInfo(*config, *gs, SorbetTypecheckRunStatus::Started, isFastPath, {});
    if (isFastPath) {
        auto run = runFastPath(move(updates), workers);
        filesTypechecked = run.filesTypechecked;
        commitTypecheckRun(move(run));
    } else {
        committed = runSlowPath(move(updates), workers, /* cancelable */ true);
    }
    sendTypecheckInfo(*config, *gs, committed ? SorbetTypecheckRunStatus::Ended : SorbetTypecheckRunStatus::Cancelled,
                      isFastPath, move(filesTypechecked));
    return committed;
}

TypecheckRun LSPTypechecker::runFastPath(LSPFileUpdates updates, WorkerPool &workers) const {
    // The error queue should not have any errors in it from previous operations.
    ENFORCE(gs->errorQueue->queueIsEmptyApprox());
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

    Timer timeit(config->logger, "fast_path");
    vector<core::FileRef> subset;
    vector<core::NameHash> changedHashes;
    {
        const auto &hashes = updates.updatedFileHashes;
        ENFORCE(updates.updatedFiles.size() == hashes.size());

        int i = -1;
        for (auto &f : updates.updatedFiles) {
            ++i;
            auto fref = gs->findFileByPath(f->path());
            // We don't support new files on the fast path. This enforce failing indicates a bug in our fast/slow
            // path logic in LSPPreprocessor.
            ENFORCE(fref.exists());
            if (fref.exists()) {
                // Update to existing file on fast path
                auto &oldHash = globalStateHashes[fref.id()];
                for (auto &p : hashes[i].definitions.methodHashes) {
                    auto fnd = oldHash.definitions.methodHashes.find(p.first);
                    ENFORCE(fnd != oldHash.definitions.methodHashes.end(), "definitionHash should have failed");
                    if (fnd->second != p.second) {
                        changedHashes.emplace_back(p.first);
                    }
                }
                gs = core::GlobalState::replaceFile(move(gs), fref, f);
                // If file doesn't have a typed: sigil, then we need to ensure it's typechecked using typed: false.
                fref.data(*gs).strictLevel = pipeline::decideStrictLevel(*gs, fref, config->opts);
                subset.emplace_back(fref);
            }
        }
        core::NameHash::sortAndDedupe(changedHashes);
    }

    int i = -1;
    for (auto &oldHash : globalStateHashes) {
        i++;
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

    prodCategoryCounterInc("lsp.updates", "fastpath");
    config->logger->debug("Taking fast path");
    ENFORCE(gs->errorQueue->isEmpty());
    vector<ast::ParsedFile> updatedIndexed;
    for (auto &f : subset) {
        unique_ptr<KeyValueStore> kvstore; // nullptr
        // TODO: Thread through kvstore.
        ENFORCE(this->kvstore == nullptr);
        // TODO(jvilk): We don't need to re-index files that didn't change.
        auto t = pipeline::indexOne(config->opts, *gs, f, kvstore);
        updatedIndexed.emplace_back(ast::ParsedFile{t.tree->deepCopy(), t.file});
        updates.updatedFinalGSFileIndexes.push_back(move(t));
    }

    ENFORCE(gs->lspQuery.isEmpty());
    auto resolved = pipeline::incrementalResolve(*gs, move(updatedIndexed), config->opts);
    pipeline::typecheck(gs, move(resolved), config->opts, workers);
    auto out = gs->errorQueue->drainWithQueryResponses();
    gs->lspTypecheckCount++;
    return TypecheckRun(move(out.first), move(subset), move(updates), true);
}

namespace {
pair<unique_ptr<core::GlobalState>, ast::ParsedFile>
updateFile(unique_ptr<core::GlobalState> gs, const shared_ptr<core::File> &file, const options::Options &opts) {
    core::FileRef fref = gs->findFileByPath(file->path());
    if (fref.exists()) {
        gs = core::GlobalState::replaceFile(move(gs), fref, file);
    } else {
        fref = gs->enterFile(file);
    }
    fref.data(*gs).strictLevel = pipeline::decideStrictLevel(*gs, fref, opts);
    std::unique_ptr<KeyValueStore> kvstore; // nullptr
    return make_pair(move(gs), pipeline::indexOne(opts, *gs, fref, kvstore));
}
} // namespace

bool LSPTypechecker::runSlowPath(LSPFileUpdates updates, WorkerPool &workers, bool cancelable) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId,
            "runSlowPath can only be called from the typechecker thread.");

    auto &logger = config->logger;
    unique_ptr<ShowOperation> slowPathOp = make_unique<ShowOperation>(*config, "SlowPathBlocking", "Typechecking...");
    Timer timeit(logger, "slow_path");
    ENFORCE(!updates.canTakeFastPath || config->disableFastPath);
    ENFORCE(updates.updatedGS.has_value());
    if (!updates.updatedGS.has_value()) {
        Exception::raise("runSlowPath called with an update that lacks an updated global state.");
    }
    logger->debug("Taking slow path");

    vector<core::FileRef> affectedFiles;
    auto finalGS = move(updates.updatedGS.value());
    // Replace error queue with one that is owned by this thread.
    finalGS->errorQueue = make_shared<core::ErrorQueue>(finalGS->errorQueue->logger, finalGS->errorQueue->tracer);
    finalGS->errorQueue->ignoreFlushes = true;
    auto &epochManager = *finalGS->epochManager;
    const u4 epoch = updates.epoch;
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
                    indexedCopies.emplace_back(ast::ParsedFile{ast.tree->deepCopy(), ast.file});
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
        for (const auto &tree : indexed) {
            // Note: indexed entries for payload files don't have any contents.
            if (tree.tree && !updatedFiles.contains(tree.file.id())) {
                indexedCopies.emplace_back(ast::ParsedFile{tree.tree->deepCopy(), tree.file});
            }
        }
        if (epochManager.wasTypecheckingCanceled()) {
            return;
        }

        ENFORCE(gs->lspQuery.isEmpty());
        if (gs->sleepInSlowPath) {
            Timer::timedSleep(3000ms, *logger, "slow_path.resolve.sleep");
        }
        auto maybeResolved = pipeline::resolve(gs, move(indexedCopies), config->opts, workers, config->skipConfigatron);
        if (!maybeResolved.hasResult()) {
            return;
        }

        auto &resolved = maybeResolved.result();
        for (auto &tree : resolved) {
            ENFORCE(tree.file.exists());
            affectedFiles.push_back(tree.file);
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
            slowPathOp = make_unique<ShowOperation>(*config, "SlowPathNonBlocking", "Typechecking in background");
        }

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

        pipeline::typecheck(gs, move(resolved), config->opts, workers, cancelable, preemptManager);
    });

    // Note: This is important to do even if the slow path was canceled. It clears out any typechecking errors from the
    // aborted typechecking run.
    // Note 2: `gs` now holds the value of `finalGS`.
    auto out = gs->errorQueue->drainWithQueryResponses();
    gs->lspQuery = core::lsp::Query::noQuery();

    if (committed) {
        prodCategoryCounterInc("lsp.updates", "slowpath");
        // No need to keep around cancelation state!
        cancellationUndoState = nullptr;
        // Send diagnostics to client (we already committed file updates earlier).
        pushDiagnostics(updates.epoch, move(affectedFiles), move(out.first));
        logger->debug("[Typechecker] Typecheck run for epoch {} successfully finished.", updates.epoch);
    } else {
        prodCategoryCounterInc("lsp.updates", "slowpath_canceled");
        // Update responsible will use state in `cancellationUndoState` to restore typechecker to the point before
        // this slow path.
        ENFORCE(cancelable);
        logger->debug("[Typechecker] Typecheck run for epoch {} was canceled.", updates.epoch);
    }
    return committed;
}

void LSPTypechecker::pushDiagnostics(u4 epoch, vector<core::FileRef> filesTypechecked,
                                     vector<std::unique_ptr<core::Error>> errors) {
    config->logger->debug("[Typechecker] Sending diagnostics for epoch {}", epoch);
    vector<core::FileRef> errorFilesInNewRun;
    UnorderedMap<core::FileRef, vector<std::unique_ptr<core::Error>>> errorsAccumulated;

    // Update epochs of all files that were typechecked, since we've recalculated the set of diagnostics in these files.
    for (auto f : filesTypechecked) {
        // N.B.: Overflow could theoretically happen. It would take an absurdly long time for someone to make
        // 4294967295 edits in one session. One way to handle that case: Have a special overflow request that blocks
        // preemption and resets all versions to 0.
        if (diagnosticEpochs[f.id()] < epoch) {
            diagnosticEpochs[f.id()] = epoch;
        }
    }

    for (auto &e : errors) {
        if (e->isSilenced) {
            continue;
        }
        auto file = e->loc.file();
        errorsAccumulated[file].emplace_back(std::move(e));
    }

    for (auto &accumulated : errorsAccumulated) {
        // Ignore errors from files that have been typechecked on newer versions (e.g. because they preempted the slow
        // path). We also ignore errors from files that don't exist (which does, in fact, happen).
        // TODO(jvilk): See overflow comment above.
        if (accumulated.first.exists() && diagnosticEpochs[accumulated.first.id()] <= epoch) {
            errorFilesInNewRun.push_back(accumulated.first);
        }
    }

    vector<core::FileRef> filesToUpdateErrorListFor = errorFilesInNewRun;

    UnorderedSet<core::FileRef> filesTypecheckedAsSet;
    filesTypecheckedAsSet.insert(filesTypechecked.begin(), filesTypechecked.end());

    for (auto f : this->filesThatHaveErrors) {
        // TODO(jvilk): Overflow warning applies here, too.
        if (filesTypecheckedAsSet.find(f) != filesTypecheckedAsSet.end() && diagnosticEpochs[f.id()] <= epoch) {
            // We've retypechecked this file, it hasn't been typechecked with newer edits, and it might not have errors.
            // Ensure that we update the error list for this file on client.
            filesToUpdateErrorListFor.push_back(f);
        } else {
            // We either did not retypecheck this file, _or_ it has since been typechecked with newer edits.
            // We need to remember that it had errors from previous typecheck runs.
            errorFilesInNewRun.push_back(f);
        }
    }

    // The previous loop may have introduced dupes; remove them.
    fast_sort(filesToUpdateErrorListFor);
    filesToUpdateErrorListFor.erase(unique(filesToUpdateErrorListFor.begin(), filesToUpdateErrorListFor.end()),
                                    filesToUpdateErrorListFor.end());

    fast_sort(errorFilesInNewRun);
    errorFilesInNewRun.erase(unique(errorFilesInNewRun.begin(), errorFilesInNewRun.end()), errorFilesInNewRun.end());

    this->filesThatHaveErrors = errorFilesInNewRun;

    for (auto file : filesToUpdateErrorListFor) {
        if (!file.exists()) {
            continue;
        }
        ENFORCE(diagnosticEpochs[file.id()] <= epoch);
        ENFORCE(file.data(*gs).epoch <= epoch);
        const string uri = config->fileRef2Uri(*gs, file);
        vector<unique_ptr<Diagnostic>> diagnostics;
        // diagnostics
        if (errorsAccumulated.find(file) != errorsAccumulated.end()) {
            for (auto &e : errorsAccumulated[file]) {
                auto range = Range::fromLoc(*gs, e->loc);
                if (range == nullptr) {
                    continue;
                }
                auto diagnostic = make_unique<Diagnostic>(std::move(range), e->header);
                diagnostic->code = e->what.code;
                diagnostic->severity = DiagnosticSeverity::Error;

                typecase(e.get(), [&](core::Error *ce) {
                    vector<unique_ptr<DiagnosticRelatedInformation>> relatedInformation;
                    for (auto &section : ce->sections) {
                        string sectionHeader = section.header;

                        for (auto &errorLine : section.messages) {
                            string message;
                            if (errorLine.formattedMessage.length() > 0) {
                                message = errorLine.formattedMessage;
                            } else {
                                message = sectionHeader;
                            }
                            auto location = config->loc2Location(*gs, errorLine.loc);
                            if (location == nullptr) {
                                continue;
                            }
                            relatedInformation.push_back(
                                make_unique<DiagnosticRelatedInformation>(std::move(location), message));
                        }
                    }
                    // Add link to error documentation.
                    relatedInformation.push_back(make_unique<DiagnosticRelatedInformation>(
                        make_unique<Location>(
                            absl::StrCat(config->opts.errorUrlBase, e->what.code),
                            make_unique<Range>(make_unique<Position>(0, 0), make_unique<Position>(0, 0))),
                        "Click for more information on this error."));
                    diagnostic->relatedInformation = move(relatedInformation);
                });
                diagnostics.push_back(move(diagnostic));
            }
        }

        auto params = make_unique<PublishDiagnosticsParams>(uri, move(diagnostics));
        config->output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentPublishDiagnostics, move(params))));
    }
}

void LSPTypechecker::commitFileUpdates(LSPFileUpdates &updates, bool couldBeCanceled) {
    // The fast path cannot be canceled.
    ENFORCE(!(updates.canTakeFastPath && couldBeCanceled));
    if (couldBeCanceled) {
        ENFORCE(updates.updatedGS.has_value());
        cancellationUndoState =
            make_unique<UndoState>(*config, move(gs), std::move(indexedFinalGS), filesThatHaveErrors);
    }

    // Clear out state associated with old finalGS.
    if (!updates.canTakeFastPath) {
        indexedFinalGS.clear();
    }

    int i = -1;
    ENFORCE(updates.updatedFileIndexes.size() == updates.updatedFileHashes.size() &&
            updates.updatedFileHashes.size() == updates.updatedFiles.size());
    ENFORCE(globalStateHashes.size() == indexed.size() && globalStateHashes.size() == diagnosticEpochs.size());
    for (auto &ast : updates.updatedFileIndexes) {
        i++;
        const int id = ast.file.id();
        if (id >= indexed.size()) {
            indexed.resize(id + 1);
            globalStateHashes.resize(id + 1);
            // No diagnostics sent yet; initialize new IDs to 0.
            diagnosticEpochs.resize(id + 1, 0);
        }
        if (cancellationUndoState != nullptr) {
            // Move the evicted values before they get replaced.
            cancellationUndoState->recordEvictedState(move(indexed[id]), move(globalStateHashes[id]));
        }
        indexed[id] = move(ast);
        globalStateHashes[id] = move(updates.updatedFileHashes[i]);
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

void LSPTypechecker::commitTypecheckRun(TypecheckRun run) {
    Timer timeit(config->logger, "commitTypecheckRun");
    commitFileUpdates(run.updates, false);
    pushDiagnostics(run.updates.epoch, move(run.filesTypechecked), move(run.errors));
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
        core::Context ctx(gs, core::Symbols::root());
        t.tree = ast::TreeMap::apply(ctx, localVarSaver, move(t.tree));
    }
}

void tryApplyDefLocSaver(const core::GlobalState &gs, vector<ast::ParsedFile> &indexedCopies) {
    if (gs.lspQuery.kind != core::lsp::Query::Kind::LOC && gs.lspQuery.kind != core::lsp::Query::Kind::SYMBOL) {
        return;
    }
    for (auto &t : indexedCopies) {
        DefLocSaver defLocSaver;
        core::Context ctx(gs, core::Symbols::root());
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

    Timer timeit(config->logger, "query");
    prodCategoryCounterInc("lsp.updates", "query");
    ENFORCE(gs->errorQueue->isEmpty());
    ENFORCE(gs->lspQuery.isEmpty());
    gs->lspQuery = q;
    auto resolved = getResolved(filesForQuery);
    tryApplyDefLocSaver(*gs, resolved);
    tryApplyLocalVarSaver(*gs, resolved);
    pipeline::typecheck(gs, move(resolved), config->opts, workers);
    auto out = gs->errorQueue->drainWithQueryResponses();
    gs->lspTypecheckCount++;
    gs->lspQuery = core::lsp::Query::noQuery();
    return LSPQueryResult{move(out.second)};
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
        noop.updatedFileIndexes.push_back({index.tree->deepCopy(), index.file});
        noop.updatedFiles.push_back(gs->getFiles()[fref.id()]);
        noop.updatedFileHashes.push_back(globalStateHashes[fref.id()]);
    }
    return noop;
}

TypecheckRun LSPTypechecker::retypecheck(vector<core::FileRef> frefs, WorkerPool &workers) const {
    LSPFileUpdates updates = getNoopUpdate(move(frefs));
    return runFastPath(move(updates), workers);
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
            updatedIndexed.emplace_back(ast::ParsedFile{indexed.tree->deepCopy(), indexed.file});
        }
    }
    return pipeline::incrementalResolve(*gs, move(updatedIndexed), config->opts);
}

const std::vector<core::FileHash> &LSPTypechecker::getFileHashes() const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId, "Typechecker can only be used from the typechecker thread.");
    return globalStateHashes;
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

vector<core::FileHash> LSPTypechecker::computeFileHashes(const LSPConfiguration &config,
                                                         const vector<shared_ptr<core::File>> &files,
                                                         WorkerPool &workers) {
    Timer timeit(config.logger, "computeFileHashes");
    vector<core::FileHash> res(files.size());
    shared_ptr<ConcurrentBoundedQueue<int>> fileq = make_shared<ConcurrentBoundedQueue<int>>(files.size());
    for (int i = 0; i < files.size(); i++) {
        auto copy = i;
        fileq->push(move(copy), 1);
    }

    auto &logger = *config.logger;
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

TypecheckRun::TypecheckRun(vector<unique_ptr<core::Error>> errors, vector<core::FileRef> filesTypechecked,
                           LSPFileUpdates updates, bool tookFastPath,
                           std::optional<std::unique_ptr<core::GlobalState>> newGS)
    : errors(move(errors)), filesTypechecked(move(filesTypechecked)), updates(move(updates)),
      tookFastPath(tookFastPath), newGS(move(newGS)) {}

LSPTypecheckerDelegate::LSPTypecheckerDelegate(WorkerPool &workers, LSPTypechecker &typechecker)
    : typechecker(typechecker), workers(workers) {}

void LSPTypecheckerDelegate::typecheckOnFastPath(LSPFileUpdates updates) {
    if (!updates.canTakeFastPath) {
        Exception::raise("Tried to typecheck a slow path edit on the fast path.");
    }
    auto committed = typechecker.typecheck(move(updates), workers);
    // Fast path edits can't be canceled.
    ENFORCE(committed);
}

TypecheckRun LSPTypecheckerDelegate::retypecheck(std::vector<core::FileRef> frefs) const {
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

const std::vector<core::FileHash> &LSPTypecheckerDelegate::getFileHashes() const {
    return typechecker.getFileHashes();
}

const core::GlobalState &LSPTypecheckerDelegate::state() const {
    return typechecker.state();
}

} // namespace sorbet::realmain::lsp
