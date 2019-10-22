#include "main/lsp/LSPTypechecker.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "ast/treemap/treemap.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "core/Unfreeze.h"
#include "json_types.h"
#include "main/lsp/DefLocSaver.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LocalVarFinder.h"
#include "main/lsp/LocalVarSaver.h"
#include "main/lsp/ShowOperation.h"
#include "main/pipeline/pipeline.h"

namespace sorbet::realmain::lsp {
using namespace std;

namespace {
vector<string> frefsToPaths(const core::GlobalState &gs, const vector<core::FileRef> &refs) {
    vector<string> paths;
    paths.resize(refs.size());
    std::transform(refs.begin(), refs.end(), paths.begin(),
                   [&gs](const auto &ref) -> string { return string(ref.data(gs).path()); });
    return paths;
}
} // namespace

LSPTypecheckerUndoState::LSPTypecheckerUndoState(u4 version, unique_ptr<core::GlobalState> oldGS,
                                                 UnorderedMap<int, ast::ParsedFile> oldIndexedFinalGS,
                                                 vector<core::FileRef> oldFilesThatHaveErrors)
    : version(version), gs(move(oldGS)), indexedFinalGS(std::move(oldIndexedFinalGS)),
      filesThatHaveErrors(move(oldFilesThatHaveErrors)) {}

vector<core::FileRef> LSPTypechecker::restore(LSPTypecheckerUndoState &undoState) {
    // Remove any new files.
    indexed.resize(undoState.indexed.size());
    globalStateHashes.resize(undoState.indexed.size());

    // Replace indexed trees and file hashes for any files that have been typechecked on the new final GS.
    for (auto &entry : indexedFinalGS) {
        // It might be from a new file we removed above, so check if it is.
        if (entry.first <= undoState.indexed.size()) {
            indexed[entry.first] = move(undoState.indexed[entry.first]);
            globalStateHashes[entry.first] = move(undoState.globalStateHashes[entry.first]);
        }
    }
    indexedFinalGS = std::move(undoState.indexedFinalGS);

    // Clear errors for files that are in the new set of files with errors but not the old set.
    // TODO: Update with the reverse when we switch to tombstoning files.
    vector<string> newPathsThatHaveErrors = frefsToPaths(*gs, filesThatHaveErrors);
    vector<string> oldPathsThatHaveErrors = frefsToPaths(*undoState.gs, undoState.filesThatHaveErrors);
    fast_sort(newPathsThatHaveErrors);
    fast_sort(oldPathsThatHaveErrors);
    vector<string> clearErrorsFor;
    std::set_difference(newPathsThatHaveErrors.begin(), newPathsThatHaveErrors.end(), oldPathsThatHaveErrors.begin(),
                        oldPathsThatHaveErrors.end(), std::inserter(clearErrorsFor, clearErrorsFor.begin()));
    for (auto &file : clearErrorsFor) {
        vector<unique_ptr<Diagnostic>> diagnostics;
        config->output->write(make_unique<LSPMessage>(make_unique<NotificationMessage>(
            "2.0", LSPMethod::TextDocumentPublishDiagnostics,
            make_unique<PublishDiagnosticsParams>(config->localName2Remote(file), move(diagnostics)))));
    }

    // Finally, restore the old global state.
    gs = move(undoState.gs);
    return undoState.filesThatHaveErrors;
}

void LSPTypecheckerUndoState::recordEvictedState(ast::ParsedFile replacedIndexTree, core::FileHash replacedStateHash) {
    const auto id = replacedIndexTree.file.id();
    // The first time a file gets evicted, it's an index tree from the old global state.
    // Subsequent times it is evicting old index trees from the new global state, and we don't care.
    // Also, ignore updates to new files (id >= size of file table)
    if (id < gs->getFiles().size() && !indexed.contains(id)) {
        indexed[id] = move(replacedIndexTree);
        globalStateHashes[id] = move(replacedStateHash);
    }
}

LSPTypechecker::LSPTypechecker(const std::shared_ptr<const LSPConfiguration> &config)
    : typecheckerThreadId(this_thread::get_id()), config(config) {}

void LSPTypechecker::initialize(LSPFileUpdates updates) {
    globalStateHashes = move(updates.updatedFileHashes);
    indexed = move(updates.updatedFileIndexes);
    ENFORCE(globalStateHashes.size() == indexed.size());
    // Initialization typecheck is not cancelable.
    auto committed = runSlowPath(move(updates), false);
    ENFORCE(committed);
}

bool LSPTypechecker::typecheck(LSPFileUpdates updates, bool cancelableAndPreemptible) {
    vector<core::FileRef> addToTypecheck;
    if (updates.canceledSlowPath) {
        // This update canceled the last slow path, so we should have undo state to restore to go to the point _before_
        // that slow path. This should always be the case, but let's not crash release builds.
        ENFORCE(cancellationUndoState.has_value());
        if (cancellationUndoState.has_value()) {
            // This is the typecheck that caused us to cancel the previous slow path. Un-commit all typechecker changes.
            addToTypecheck = restore(cancellationUndoState.value());
            cancellationUndoState = nullopt;
        }
    }

    if (updates.canTakeFastPath) {
        // Retypecheck all files that formerly had errors.
        for (auto fref : addToTypecheck) {
            auto &index = getIndexed(fref);
            updates.updatedFileIndexes.push_back({index.tree->deepCopy(), index.file});
            updates.updatedFiles.push_back(gs->getFiles()[fref.id()]);
            updates.updatedFileHashes.push_back(globalStateHashes[fref.id()]);
        }
        auto run = runFastPath(move(updates));
        auto committed = !run.canceled;
        commitTypecheckRun(move(run));
        return committed;
    } else {
        // No need to add any files to typecheck, as it'll retypecheck all files.
        // TODO: In future, have it prioritize old files with errors.
        return runSlowPath(move(updates), cancelableAndPreemptible);
    }
}

TypecheckRun LSPTypechecker::runFastPath(LSPFileUpdates updates) const {
    ENFORCE(this_thread::get_id() == typecheckerThreadId,
            "runTypechecking can only be called from the typechecker thread.");
    // We assume gs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(gs->lspTypecheckCount.load() > 0,
            "Tried to run fast path with a GlobalState object that never had inferencer and resolver runs.");

    if (!updates.canTakeFastPath) {
        Exception::raise("Attempted to incrementally typecheck changes that must take the slow path.");
    }

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
        auto t = pipeline::indexOne(config->opts, *gs, f, kvstore);
        updatedIndexed.emplace_back(ast::ParsedFile{t.tree->deepCopy(), t.file});
        updates.updatedFinalGSFileIndexes.push_back(move(t));
    }

    ENFORCE(gs->lspQuery.isEmpty());
    auto resolved = pipeline::incrementalResolve(*gs, move(updatedIndexed), config->opts);
    // Disable multithreading for fast path since it can preempt a slow path that is already monopolizing all workers.
    auto workers = WorkerPool::create(0, gs->tracer());
    pipeline::typecheck(gs, move(resolved), config->opts, *workers);
    auto out = gs->errorQueue->drainWithQueryResponses();
    gs->lspTypecheckCount.fetch_add(1);
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

bool LSPTypechecker::runSlowPath(LSPFileUpdates updates, bool cancelableAndPreemptible) {
    ENFORCE(this_thread::get_id() == typecheckerThreadId,
            "runSlowPath can only be called from the typechecker thread.");

    auto &logger = config->logger;
    ShowOperation slowPathOp(*config, "SlowPath", "Typechecking...");
    Timer timeit(logger, "slow_path");
    ENFORCE(!updates.canTakeFastPath || config->disableFastPath);
    ENFORCE(updates.updatedGS.has_value());
    if (!updates.updatedGS.has_value()) {
        Exception::raise("runSlowPath called with an update that lacks an updated global state.");
    }
    logger->debug("Taking slow path");

    UnorderedSet<int> updatedFiles;
    vector<ast::ParsedFile> indexedCopies;
    vector<core::FileRef> affectedFiles;
    auto finalGS = move(updates.updatedGS.value());
    // Replace error queue with one that is owned by this thread.
    finalGS->errorQueue = make_shared<core::ErrorQueue>(finalGS->errorQueue->logger, finalGS->errorQueue->tracer);
    finalGS->errorQueue->ignoreFlushes = true;

    // Note: Commits can only be canceled if this edit is cancelable, LSP is running across multiple threads, and the
    // cancelation feature is enabled.
    const bool committed = finalGS->tryCommitEpoch(updates.versionEnd, cancelableAndPreemptible, [&]() -> void {
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
        commitFileUpdates(updates, false, cancelableAndPreemptible);

        // Copy the indexes of unchanged files.
        for (const auto &tree : indexed) {
            // Note: indexed entries for payload files don't have any contents.
            if (tree.tree && !updatedFiles.contains(tree.file.id())) {
                indexedCopies.emplace_back(ast::ParsedFile{tree.tree->deepCopy(), tree.file});
            }
        }
        if (gs->wasTypecheckingCanceled()) {
            return;
        }

        ENFORCE(gs->lspQuery.isEmpty());
        if (gs->sleepInSlowPath) {
            Timer::timedSleep(3000ms, *logger, "slow_path.resolve.sleep");
        }
        auto maybeResolved =
            pipeline::resolve(gs, move(indexedCopies), config->opts, config->workers, config->skipConfigatron);
        if (!maybeResolved.hasResult()) {
            return;
        }

        auto &resolved = maybeResolved.result();
        for (auto &tree : resolved) {
            ENFORCE(tree.file.exists());
            affectedFiles.push_back(tree.file);
        }

        // Inform the fast path that this global state is OK for typechecking as resolution has completed.
        gs->lspTypecheckCount.fetch_add(1);

        // (Tests only) Run any expected preemption functions.
        while (updates.expectedPreemptions > 0) {
            if (gs->tryRunPreemptionFunction()) {
                updates.expectedPreemptions--;
            }
        }

        pipeline::typecheck(gs, move(resolved), config->opts, config->workers, cancelableAndPreemptible);
        if (gs->sleepInSlowPath) {
            Timer::timedSleep(3000ms, *logger, "slow_path.typecheck.sleep");
        }
    });
    ENFORCE(gs);

    // Note: This is important to do even if the slow path was canceled. It clears out any typechecking errors from the
    // aborted typechecking run.
    auto out = gs->errorQueue->drainWithQueryResponses();
    gs->lspQuery = core::lsp::Query::noQuery();

    if (config->getClientConfig().enableTypecheckInfo) {
        auto sorbetTypecheckInfo = make_unique<SorbetTypecheckRunInfo>(false, vector<string>(), committed);
        config->output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::SorbetTypecheckRunInfo, move(sorbetTypecheckInfo))));
    }

    if (committed) {
        prodCategoryCounterInc("lsp.updates", "slowpath");
        // No need to keep around cancelation state!
        cancellationUndoState = nullopt;
        pushDiagnostics(updates.versionEnd, move(affectedFiles), move(out.first));
        return true;
    } else {
        prodCategoryCounterInc("lsp.updates", "slowpath_canceled");
        // Update responsible will use state in `cancellationUndoState` to restore typechecker to the point before
        // this slow path.
        ENFORCE(cancelableAndPreemptible);
        return false;
    }
}

void LSPTypechecker::pushDiagnostics(u4 version, vector<core::FileRef> filesTypechecked,
                                     vector<std::unique_ptr<core::Error>> errors) {
    vector<core::FileRef> errorFilesInNewRun;
    UnorderedMap<core::FileRef, vector<std::unique_ptr<core::Error>>> errorsAccumulated;

    for (auto &e : errors) {
        if (e->isSilenced) {
            continue;
        }
        auto file = e->loc.file();
        errorsAccumulated[file].emplace_back(std::move(e));
    }

    for (auto &accumulated : errorsAccumulated) {
        // Ignore errors from files that have been typechecked on newer versions (e.g. because they preempted the slow
        // path)
        // TODO(jvilk): Overflow could theoretically happen. I calculated that it would take an absurdly long time for
        // someone to make 4294967295 edits in one session. One way to handle that case: Have a special overflow
        // request that blocks preemption and resets all versions to 0.
        if (globalStateHashes[accumulated.first.id()].version <= version) {
            errorFilesInNewRun.push_back(accumulated.first);
        }
    }

    vector<core::FileRef> filesToUpdateErrorListFor = errorFilesInNewRun;

    UnorderedSet<core::FileRef> filesTypecheckedAsSet;
    filesTypecheckedAsSet.insert(filesTypechecked.begin(), filesTypechecked.end());

    for (auto f : this->filesThatHaveErrors) {
        // TODO(jvilk): Overflow warning applies here, too.
        if (filesTypecheckedAsSet.find(f) != filesTypecheckedAsSet.end() &&
            globalStateHashes[f.id()].version <= version) {
            // We've retypechecked this file, it hasn't been typechecked with newer edits, and it doesn't have errors.
            // thus, we will update the error list for this file on client to be the empty list.
            filesToUpdateErrorListFor.push_back(f);
        } else {
            // We either did not retypecheck this file, _or_ it has since been typechecked with newer edits.
            // We need to remember that it had error.
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
                    uri = config->fileRef2Uri(*gs, file);
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
            }

            config->output->write(make_unique<LSPMessage>(
                make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentPublishDiagnostics,
                                                 make_unique<PublishDiagnosticsParams>(uri, move(diagnostics)))));
        }
    }
}

void LSPTypechecker::commitFileUpdates(LSPFileUpdates &updates, bool tookFastPath, bool couldBeCanceled) {
    // Only take the fast path if the updates _can_ take the fast path.
    ENFORCE((tookFastPath && updates.canTakeFastPath) || !tookFastPath);

    unique_ptr<absl::MutexLock> gsLock;

    if (couldBeCanceled) {
        gsLock = make_unique<absl::MutexLock>(&gsMutex);
        cancellationUndoState = make_optional<LSPTypecheckerUndoState>(updates.versionEnd, move(gs),
                                                                       std::move(indexedFinalGS), filesThatHaveErrors);
    }

    // Clear out state associated with old finalGS.
    if (!tookFastPath) {
        indexedFinalGS.clear();
    }

    int i = -1;
    ENFORCE(updates.updatedFileIndexes.size() == updates.updatedFileHashes.size() &&
            updates.updatedFileHashes.size() == updates.updatedFiles.size());
    ENFORCE(indexed.size() == globalStateHashes.size());
    for (auto &ast : updates.updatedFileIndexes) {
        i++;
        const int id = ast.file.id();
        if (id >= indexed.size()) {
            // New file.
            indexed.resize(id + 1);
            globalStateHashes.resize(id + 1);
        }
        if (cancellationUndoState.has_value()) {
            cancellationUndoState->recordEvictedState(move(indexed[id]), move(globalStateHashes[id]));
        }
        indexed[id] = move(ast);
        globalStateHashes[id] = move(updates.updatedFileHashes[i]);
    }

    for (auto &ast : updates.updatedFinalGSFileIndexes) {
        indexedFinalGS[ast.file.id()] = move(ast);
    }

    if (updates.updatedGS.has_value()) {
        if (!gsLock) {
            gsLock = make_unique<absl::MutexLock>(&gsMutex);
        }
        gs = move(updates.updatedGS.value());
    } else {
        ENFORCE(tookFastPath);
    }
}

void LSPTypechecker::commitTypecheckRun(TypecheckRun run) {
    auto &logger = config->logger;
    if (run.canceled) {
        logger->debug("[Typechecker] Typecheck run for edits {} thru {} was canceled.", run.updates.versionStart,
                      run.updates.versionEnd);
        return;
    }

    Timer timeit(logger, "commitTypecheckRun");
    commitFileUpdates(run.updates, run.tookFastPath, false);
    if (config->getClientConfig().enableTypecheckInfo) {
        vector<string> pathsTypechecked;
        for (auto &f : run.filesTypechecked) {
            pathsTypechecked.emplace_back(f.data(*gs).path());
        }
        auto sorbetTypecheckInfo = make_unique<SorbetTypecheckRunInfo>(run.tookFastPath, move(pathsTypechecked), false);
        config->output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::SorbetTypecheckRunInfo, move(sorbetTypecheckInfo))));
    }
    pushDiagnostics(run.updates.versionEnd, move(run.filesTypechecked), move(run.errors));
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

LSPQueryResult LSPTypechecker::query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery) const {
    // We assume gs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(gs->lspTypecheckCount.load() > 0,
            "Tried to run a query with a GlobalState object that never had inferencer and resolver runs.");

    Timer timeit(config->logger, "query");
    prodCategoryCounterInc("lsp.updates", "query");
    ENFORCE(gs->errorQueue->isEmpty());
    ENFORCE(gs->lspQuery.isEmpty());
    gs->lspQuery = q;
    auto resolved = getResolved(filesForQuery);
    tryApplyDefLocSaver(*gs, resolved);
    tryApplyLocalVarSaver(*gs, resolved);
    // Disable multithreading for fast path since it can preempt a slow path that is already monopolizing all workers.
    auto workers = WorkerPool::create(0, gs->tracer());
    pipeline::typecheck(gs, move(resolved), config->opts, *workers);
    auto out = gs->errorQueue->drainWithQueryResponses();
    gs->lspTypecheckCount.fetch_add(1);
    gs->lspQuery = core::lsp::Query::noQuery();
    return LSPQueryResult{move(out.second)};
}

TypecheckRun LSPTypechecker::retypecheck(LSPFileUpdates updates) const {
    if (!updates.canTakeFastPath) {
        Exception::raise("Tried to typecheck slow path updates with retypecheck. Retypecheck can only typecheck the "
                         "previously typechecked version of a file.");
    }

    for (const auto &file : updates.updatedFiles) {
        auto path = file->path();
        auto source = file->source();
        auto fref = gs->findFileByPath(path);
        if (!fref.exists() || fref.data(*gs).source() != source) {
            Exception::raise("Retypecheck can only typecheck the previously typechecked version of a file.");
        }
    }

    return runFastPath(move(updates));
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
    return globalStateHashes;
}

const core::GlobalState &LSPTypechecker::state() const {
    return *gs;
}

void LSPTypechecker::changeThread() {
    auto newId = this_thread::get_id();
    ENFORCE(newId != typecheckerThreadId);
    typecheckerThreadId = newId;
}

TypecheckRun::TypecheckRun(vector<unique_ptr<core::Error>> errors, vector<core::FileRef> filesTypechecked,
                           LSPFileUpdates updates, bool tookFastPath)
    : errors(move(errors)), filesTypechecked(move(filesTypechecked)), updates(move(updates)),
      tookFastPath(tookFastPath) {}

TypecheckRun TypecheckRun::makeCanceled() {
    TypecheckRun run;
    run.canceled = true;
    return run;
}

bool LSPTypechecker::tryPreemptSlowPath(function<void()> &lambda) {
    absl::MutexLock gsLock(&gsMutex);
    // Note: GS may be null if initialization has not yet completed.
    if (gs) {
        return gs->tryPreempt(lambda);
    }
    return false;
}

} // namespace sorbet::realmain::lsp
