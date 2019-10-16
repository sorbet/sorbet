#include "main/lsp/LSPTypechecker.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "ast/treemap/treemap.h"
#include "common/typecase.h"
#include "core/Unfreeze.h"
#include "main/lsp/DefLocSaver.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LocalVarSaver.h"
#include "main/lsp/ShowOperation.h"
#include "main/pipeline/pipeline.h"

namespace sorbet::realmain::lsp {
using namespace std;

LSPTypechecker::LSPTypechecker(const shared_ptr<spd::logger> &logger, WorkerPool &workers, LSPConfiguration config)
    : wasShutdown(false), typecheckerThreadId(this_thread::get_id()), logger(logger), workers(workers), config(config) {
}

void LSPTypechecker::initialize(LSPFileUpdates updates, LSPOutput &output) {
    runOnTypecheckerThread([&]() -> void {
        globalStateHashes = move(updates.updatedFileHashes);
        indexed = move(updates.updatedFileIndexes);
        // Initialization typecheck is not cancelable.
        auto run = runSlowPath(move(updates), /* cancelable */ false, output);
        ENFORCE(!run.canceled);
        ENFORCE(run.newGS.has_value());
        gs = move(run.newGS.value());
        pushDiagnostics(move(run), output);
        config.initialized = true;
    });
}

bool LSPTypechecker::typecheck(LSPFileUpdates updates, LSPOutput &output) {
    bool finished = true;
    // TODO: Make this asynchronous.
    runOnTypecheckerThread([&]() -> void {
        auto run = runTypechecking(move(updates), output);
        finished = !run.canceled;
        commitTypecheckRun(move(run), output);
    });
    return finished;
}

void LSPTypechecker::enterCriticalSection(std::function<void(const LSPTypecheckerOps &)> lambda) {
    runOnTypecheckerThread([&]() -> void { lambda(LSPTypecheckerOps(*this, *gs)); });
}

void LSPTypechecker::shutdown() {
    absl::MutexLock lock(&mtx);
    wasShutdown = true;
}

unique_ptr<Joinable> LSPTypechecker::runTypechecker() {
    ENFORCE(this_thread::get_id() == typecheckerThreadId);
    return runInAThread("lspPreprocess", [&]() -> void {
        typecheckerThreadId = this_thread::get_id();
        optional<function<void()>> toRun = nullopt;
        {
            absl::MutexLock lock(&mtx);
            mtx.Await(absl::Condition(
                +[](LSPTypechecker *typechecker) -> bool {
                    typechecker->mtx.AssertHeld(); // Satisfy thread annotations.
                    return typechecker->wasShutdown || typechecker->lambda.has_value();
                },
                this));
            if (wasShutdown) {
                lambda = nullopt;
                return;
            }
            toRun = move(lambda);
            lambda = nullopt;
        }
        ENFORCE(toRun.has_value());
        toRun.value()();
        toRun = nullopt;
    });
}

void LSPTypechecker::runOnTypecheckerThread(function<void()> lambda) {
    if (this_thread::get_id() == typecheckerThreadId) {
        return lambda();
    } else {
        // TODO: Support async tasks. For now, everything blocks.
        absl::Notification lambdaComplete;
        {
            absl::MutexLock lock(&mtx);
            ENFORCE(!this->lambda.has_value());
            this->lambda = [&lambdaComplete, &lambda]() -> void {
                lambda();
                lambdaComplete.Notify();
            };
        }
        // Block until lambda runs.
        lambdaComplete.WaitForNotification();
    }
}

TypecheckRun LSPTypechecker::runTypechecking(LSPFileUpdates updates, LSPOutput &output) const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId,
            "runTypechecking can only be called from the typechecker thread.");
    // We assume gs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(gs->lspTypecheckCount > 0,
            "Tried to run fast path with a GlobalState object that never had inferencer and resolver runs.");

    if (!updates.canTakeFastPath) {
        return runSlowPath(move(updates), true, output);
    }

    Timer timeit(logger, "fast_path");
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
                fref.data(*gs).strictLevel = pipeline::decideStrictLevel(*gs, fref, config.opts);
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
            logger->debug("Added {} to update set as used a changed method", !ref.exists() ? "" : ref.data(*gs).path());
            subset.emplace_back(ref);
        }
    }
    // Remove any duplicate files.
    fast_sort(subset);
    subset.resize(std::distance(subset.begin(), std::unique(subset.begin(), subset.end())));

    prodCategoryCounterInc("lsp.updates", "fastpath");
    logger->debug("Taking fast path");
    ENFORCE(gs->errorQueue->isEmpty());
    vector<ast::ParsedFile> updatedIndexed;
    for (auto &f : subset) {
        unique_ptr<KeyValueStore> kvstore; // nullptr
        // TODO: Thread through kvstore.
        ENFORCE(this->kvstore == nullptr);
        auto t = pipeline::indexOne(config.opts, *gs, f, kvstore);
        updatedIndexed.emplace_back(ast::ParsedFile{t.tree->deepCopy(), t.file});
        updates.updatedFinalGSFileIndexes.push_back(move(t));
    }

    ENFORCE(gs->lspQuery.isEmpty());
    auto resolved = pipeline::incrementalResolve(*gs, move(updatedIndexed), config.opts);
    pipeline::typecheck(gs, move(resolved), config.opts, workers);
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

TypecheckRun LSPTypechecker::runSlowPath(LSPFileUpdates updates, bool isCancelable, LSPOutput &output) const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId,
            "runSlowPath can only be called from the typechecker thread.");

    ShowOperation slowPathOp(output, config, "SlowPath", "Typechecking...");
    Timer timeit(logger, "slow_path");
    ENFORCE(!updates.canTakeFastPath || config.disableFastPath);
    ENFORCE(updates.updatedGS.has_value());
    if (!updates.updatedGS.has_value()) {
        Exception::raise("runSlowPath called with an update that lacks an updated global state.");
    }
    logger->debug("Taking slow path");

    UnorderedSet<int> updatedFiles;
    vector<ast::ParsedFile> indexedCopies;
    vector<core::FileRef> affectedFiles;
    auto finalGS = move(updates.updatedGS.value());
    finalGS->errorQueue->changeOwnerToCurrentThread();
    // Note: Commits can only be canceled if this edit is cancelable, LSP is running across multiple threads, and the
    // cancelation feature is enabled.
    const bool committed = finalGS->tryCommitEpoch(updates.versionEnd, isCancelable, [&]() -> void {
        // Index the updated files using finalGS.
        {
            core::UnfreezeFileTable fileTableAccess(*finalGS);
            for (auto &file : updates.updatedFiles) {
                auto pair = updateFile(move(finalGS), file, config.opts);
                finalGS = move(pair.first);
                auto &ast = pair.second;
                if (ast.tree) {
                    indexedCopies.emplace_back(ast::ParsedFile{ast.tree->deepCopy(), ast.file});
                    updatedFiles.insert(ast.file.id());
                }
                updates.updatedFinalGSFileIndexes.push_back(move(ast));
            }
        }

        // Copy the indexes of unchanged files.
        for (const auto &tree : indexed) {
            // Note: indexed entries for payload files don't have any contents.
            if (tree.tree && !updatedFiles.contains(tree.file.id())) {
                indexedCopies.emplace_back(ast::ParsedFile{tree.tree->deepCopy(), tree.file});
            }
        }
        if (finalGS->wasTypecheckingCanceled()) {
            return;
        }

        ENFORCE(finalGS->lspQuery.isEmpty());
        if (finalGS->sleepInSlowPath) {
            Timer::timedSleep(3000ms, *logger, "slow_path.resolve.sleep");
        }
        auto maybeResolved =
            pipeline::resolve(finalGS, move(indexedCopies), config.opts, workers, config.skipConfigatron);
        if (!maybeResolved.hasResult()) {
            return;
        }

        auto &resolved = maybeResolved.result();
        for (auto &tree : resolved) {
            ENFORCE(tree.file.exists());
            affectedFiles.push_back(tree.file);
        }
        if (finalGS->sleepInSlowPath) {
            Timer::timedSleep(3000ms, *logger, "slow_path.typecheck.sleep");
        }
        pipeline::typecheck(finalGS, move(resolved), config.opts, workers);
    });

    auto out = finalGS->errorQueue->drainWithQueryResponses();
    finalGS->lspTypecheckCount++;
    finalGS->lspQuery = core::lsp::Query::noQuery();

    if (committed) {
        prodCategoryCounterInc("lsp.updates", "slowpath");
        return TypecheckRun(move(out.first), move(affectedFiles), move(updates), false, move(finalGS));
    } else {
        prodCategoryCounterInc("lsp.updates", "slowpath_canceled");
        // Drain any enqueued errors from aborted typechecking run.
        gs->errorQueue->drainWithQueryResponses();
        return TypecheckRun::makeCanceled();
    }
}

void LSPTypechecker::pushDiagnostics(TypecheckRun run, LSPOutput &output) {
    ENFORCE(!run.canceled);
    const auto &filesTypechecked = run.filesTypechecked;
    vector<core::FileRef> errorFilesInNewRun;
    UnorderedMap<core::FileRef, vector<std::unique_ptr<core::Error>>> errorsAccumulated;

    if (config.enableTypecheckInfo) {
        vector<string> pathsTypechecked;
        for (auto &f : filesTypechecked) {
            pathsTypechecked.emplace_back(f.data(*gs).path());
        }
        auto sorbetTypecheckInfo = make_unique<SorbetTypecheckRunInfo>(run.tookFastPath, move(pathsTypechecked));
        output.write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::SorbetTypecheckRunInfo, move(sorbetTypecheckInfo))));
    }

    for (auto &e : run.errors) {
        if (e->isSilenced) {
            continue;
        }
        auto file = e->loc.file();
        errorsAccumulated[file].emplace_back(std::move(e));
    }

    for (auto &accumulated : errorsAccumulated) {
        errorFilesInNewRun.push_back(accumulated.first);
    }

    vector<core::FileRef> filesToUpdateErrorListFor = errorFilesInNewRun;

    UnorderedSet<core::FileRef> filesTypecheckedAsSet;
    filesTypecheckedAsSet.insert(filesTypechecked.begin(), filesTypechecked.end());

    for (auto f : this->filesThatHaveErrors) {
        if (filesTypecheckedAsSet.find(f) != filesTypecheckedAsSet.end()) {
            // we've retypechecked this file. We can override the fact it has an error
            // thus, we will update the error list for this file on client
            filesToUpdateErrorListFor.push_back(f);
        } else {
            // we're not typecking this file, we need to remember that it had error
            errorFilesInNewRun.push_back(f);
        }
    }

    fast_sort(filesToUpdateErrorListFor);
    filesToUpdateErrorListFor.erase(unique(filesToUpdateErrorListFor.begin(), filesToUpdateErrorListFor.end()),
                                    filesToUpdateErrorListFor.end());

    fast_sort(errorFilesInNewRun);
    errorFilesInNewRun.erase(unique(errorFilesInNewRun.begin(), errorFilesInNewRun.end()), errorFilesInNewRun.end());

    this->filesThatHaveErrors = errorFilesInNewRun;

    for (auto file : filesToUpdateErrorListFor) {
        if (file.exists()) {
            string uri;
            { // uri
                if (file.data(*gs).sourceType == core::File::Type::Payload) {
                    uri = string(file.data(*gs).path());
                } else {
                    uri = config.fileRef2Uri(*gs, file);
                }
            }

            vector<unique_ptr<Diagnostic>> diagnostics;
            {
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
                                    auto location = config.loc2Location(*gs, errorLine.loc);
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
                                    absl::StrCat(config.opts.errorUrlBase, e->what.code),
                                    make_unique<Range>(make_unique<Position>(0, 0), make_unique<Position>(0, 0))),
                                "Click for more information on this error."));
                            diagnostic->relatedInformation = move(relatedInformation);
                        });
                        diagnostics.push_back(move(diagnostic));
                    }
                }
            }

            output.write(make_unique<LSPMessage>(
                make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentPublishDiagnostics,
                                                 make_unique<PublishDiagnosticsParams>(uri, move(diagnostics)))));
        }
    }
    return;
}

void LSPTypechecker::commitTypecheckRun(TypecheckRun run, LSPOutput &output) {
    if (run.canceled) {
        logger->debug("[Typechecker] Typecheck run for edits {} thru {} was canceled.", run.updates.versionStart,
                      run.updates.versionEnd);
        return;
    }

    Timer timeit(logger, "commitTypecheckRun");
    auto &updates = run.updates;

    // Clear out state associated with old finalGS.
    if (!run.tookFastPath) {
        indexedFinalGS.clear();
    }

    int i = -1;
    ENFORCE(updates.updatedFileIndexes.size() == updates.updatedFileHashes.size() &&
            updates.updatedFileHashes.size() == updates.updatedFiles.size());
    for (auto &ast : updates.updatedFileIndexes) {
        i++;
        const int id = ast.file.id();
        if (id >= indexed.size()) {
            indexed.resize(id + 1);
        }
        if (id >= globalStateHashes.size()) {
            globalStateHashes.resize(id + 1);
        }
        indexed[id] = move(ast);
        globalStateHashes[id] = move(updates.updatedFileHashes[i]);
    }

    for (auto &ast : updates.updatedFinalGSFileIndexes) {
        indexedFinalGS[ast.file.id()] = move(ast);
    }

    if (run.newGS.has_value()) {
        gs = move(run.newGS.value());
    }

    return pushDiagnostics(move(run), output);
}

unique_ptr<core::GlobalState> LSPTypechecker::destroyAndReturnGlobalState() {
    return move(gs);
}

LSPTypecheckerOps::LSPTypecheckerOps(LSPTypechecker &typechecker, const core::GlobalState &gs)
    : typechecker(typechecker), gs(gs) {}

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

LSPQueryResult LSPTypechecker::query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery) const {
    // We assume gs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(gs->lspTypecheckCount > 0,
            "Tried to run a query with a GlobalState object that never had inferencer and resolver runs.");

    Timer timeit(logger, "query");
    prodCategoryCounterInc("lsp.updates", "query");
    ENFORCE(gs->errorQueue->isEmpty());
    vector<ast::ParsedFile> updatedIndexed;
    for (auto &f : filesForQuery) {
        const int id = f.id();
        const auto it = indexedFinalGS.find(id);
        const auto &parsedFile = it == indexedFinalGS.end() ? indexed[id] : it->second;
        if (parsedFile.tree) {
            updatedIndexed.emplace_back(ast::ParsedFile{parsedFile.tree->deepCopy(), parsedFile.file});
        }
    }

    ENFORCE(gs->lspQuery.isEmpty());
    gs->lspQuery = q;
    auto resolved = pipeline::incrementalResolve(*gs, move(updatedIndexed), config.opts);
    tryApplyDefLocSaver(*gs, resolved);
    tryApplyLocalVarSaver(*gs, resolved);
    pipeline::typecheck(gs, move(resolved), config.opts, workers);
    auto out = gs->errorQueue->drainWithQueryResponses();
    gs->lspTypecheckCount++;
    gs->lspQuery = core::lsp::Query::noQuery();
    return LSPQueryResult{move(out.second)};
}

LSPQueryResult LSPTypecheckerOps::query(const core::lsp::Query &q,
                                        const std::vector<core::FileRef> &filesForQuery) const {
    return typechecker.query(q, filesForQuery);
}

TypecheckRun LSPTypecheckerOps::fastPathTypecheck(LSPFileUpdates updates, LSPOutput &output) const {
    if (!updates.canTakeFastPath) {
        Exception::raise("Tried to typecheck slow path updates on fast path.");
    }
    return typechecker.runTypechecking(move(updates), output);
}

const ast::ParsedFile &LSPTypecheckerOps::getIndex(core::FileRef fref) const {
    const auto id = fref.id();
    auto treeFinalGS = typechecker.indexedFinalGS.find(id);
    if (treeFinalGS != typechecker.indexedFinalGS.end()) {
        return treeFinalGS->second;
    }
    ENFORCE(id < typechecker.indexed.size());
    return typechecker.indexed[id];
}

const std::vector<core::FileHash> &LSPTypecheckerOps::getFileHashes() const {
    return typechecker.globalStateHashes;
}

TypecheckRun::TypecheckRun(vector<unique_ptr<core::Error>> errors, vector<core::FileRef> filesTypechecked,
                           LSPFileUpdates updates, bool tookFastPath,
                           std::optional<std::unique_ptr<core::GlobalState>> newGS)
    : errors(move(errors)), filesTypechecked(move(filesTypechecked)), updates(move(updates)),
      tookFastPath(tookFastPath), newGS(move(newGS)) {}

TypecheckRun TypecheckRun::makeCanceled() {
    TypecheckRun run;
    run.canceled = true;
    return run;
}

} // namespace sorbet::realmain::lsp
