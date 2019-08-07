#include "ast/treemap/treemap.h"
#include "common/Timer.h"
#include "core/Error.h"
#include "core/Files.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Unfreeze.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/parser.h"
#include "core/errors/resolver.h"
#include "core/lsp/QueryResponse.h"
#include "lsp.h"
#include "main/lsp/DefLocSaver.h"
#include "main/lsp/LocalVarSaver.h"
#include "main/pipeline/pipeline.h"
#include "namer/namer.h"
#include "resolver/resolver.h"
#include <algorithm> // std::unique, std::distance

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::ShowOperation::ShowOperation(const LSPLoop &loop, string_view operationName, string_view description)
    : loop(loop), operationName(string(operationName)), description(string(description)) {
    if (loop.enableOperationNotifications) {
        auto params = make_unique<SorbetShowOperationParams>(this->operationName, this->description,
                                                             SorbetOperationStatus::Start);
        loop.sendMessage(
            LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetShowOperation, move(params))));
    }
}

LSPLoop::ShowOperation::~ShowOperation() {
    if (loop.enableOperationNotifications) {
        auto params = make_unique<SorbetShowOperationParams>(operationName, description, SorbetOperationStatus::End);
        loop.sendMessage(
            LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetShowOperation, move(params))));
    }
}

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

LSPResult LSPLoop::commitTypecheckRun(TypecheckRun run) {
    Timer timeit(logger, "commitTypecheckRun");
    if (run.updates) {
        auto &updates = run.updates.value();
        // Update editor state.
        for (auto closedFile : updates.closedFiles) {
            auto it = openFiles.find(closedFile);
            if (it != openFiles.end()) {
                openFiles.erase(it);
            }
        }
        for (auto openedFile : updates.openedFiles) {
            openFiles.insert(string(openedFile));
        }

        // Clear out state associated with old finalGS.
        if (!run.tookFastPath) {
            indexedFinalGS.clear();
        }

        for (auto &ast : updates.updatedFileIndexes) {
            indexedFinalGS[ast.file.id()] = move(ast);
        }

        {
            core::UnfreezeFileTable fileTableAccess(*initialGS);
            for (auto &file : updates.updatedFiles) {
                // Update initialGS and index.
                auto rv = updateFile(move(initialGS), file, opts);
                initialGS = move(rv.first);
                const auto id = rv.second.file.id();
                if (id >= indexed.size()) {
                    indexed.resize(id + 1);
                }
                indexed[id] = move(rv.second);
            }
            // Drop any indexing errors produced during `updateFile`.
            // (Note: Flushing is disabled in LSP mode, so we have to drain.)
            errorQueue->drainWithQueryResponses();
        }

        for (auto &entry : updates.updatedFileHashes) {
            auto fref = initialGS->findFileByPath(entry.first);
            ENFORCE(fref.exists());
            const auto id = fref.id();
            if (id >= globalStateHashes.size()) {
                globalStateHashes.resize(id + 1);
            }
            globalStateHashes[fref.id()] = move(entry.second);
        }
    }

    return pushDiagnostics(move(run));
}

vector<core::FileHash> LSPLoop::computeStateHashes(const vector<shared_ptr<core::File>> &files) const {
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

void LSPLoop::reIndexFromFileSystem() {
    ShowOperation op(*this, "Indexing", "Indexing files...");
    Timer timeit(logger, "reIndexFromFileSystem");
    indexed.clear();
    vector<core::FileRef> inputFiles = pipeline::reserveFiles(initialGS, opts.inputFileNames);
    for (auto &t : pipeline::index(initialGS, inputFiles, opts, workers, kvstore)) {
        int id = t.file.id();
        if (id >= indexed.size()) {
            indexed.resize(id + 1);
        }
        indexed[id] = move(t);
    }
}

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

LSPLoop::TypecheckRun LSPLoop::runSlowPath(std::optional<FileUpdates> updates) const {
    ShowOperation slowPathOp(*this, "SlowPath", "Typechecking...");
    Timer timeit(logger, "slow_path");
    ENFORCE(initialGS->errorQueue->isEmpty());
    prodCategoryCounterInc("lsp.updates", "slowpath");
    logger->debug("Taking slow path");

    UnorderedSet<int> updatedFiles;
    vector<ast::ParsedFile> indexedCopies;
    auto finalGS = initialGS->deepCopy(true);
    // Index the updated files using finalGS.
    if (updates) {
        auto &fileUpdates = updates.value();
        core::UnfreezeFileTable fileTableAccess(*finalGS);
        for (auto &file : fileUpdates.updatedFiles) {
            auto pair = updateFile(move(finalGS), file, opts);
            finalGS = move(pair.first);
            auto &ast = pair.second;
            if (ast.tree) {
                indexedCopies.emplace_back(ast::ParsedFile{ast.tree->deepCopy(), ast.file});
                updatedFiles.insert(ast.file.id());
                fileUpdates.updatedFileIndexes.push_back(move(ast));
            }
        }
    }

    // Copy the indexes of unchanged files.
    for (const auto &tree : indexed) {
        // Note: indexed entries for payload files don't have any contents.
        if (tree.tree && !updatedFiles.contains(tree.file.id())) {
            indexedCopies.emplace_back(ast::ParsedFile{tree.tree->deepCopy(), tree.file});
        }
    }

    auto resolved = pipeline::resolve(finalGS, move(indexedCopies), opts, workers, skipConfigatron);
    tryApplyDefLocSaver(*finalGS, resolved);
    tryApplyLocalVarSaver(*finalGS, resolved);
    vector<core::FileRef> affectedFiles;
    for (auto &tree : resolved) {
        ENFORCE(tree.file.exists());
        affectedFiles.push_back(tree.file);
    }
    pipeline::typecheck(finalGS, move(resolved), opts, workers);
    auto out = initialGS->errorQueue->drainWithQueryResponses();
    finalGS->lspTypecheckCount++;
    return TypecheckRun{move(out.first), move(affectedFiles), move(out.second), move(finalGS), move(updates), false};
}

bool LSPLoop::canTakeFastPath(const FileUpdates &updates, const vector<core::FileHash> &hashes) const {
    if (disableFastPath) {
        logger->debug("Taking sad path because happy path is disabled.");
        return false;
    }
    auto &changedFiles = updates.updatedFiles;
    logger->debug("Trying to see if happy path is available after {} file changes", changedFiles.size());

    ENFORCE(changedFiles.size() == hashes.size());
    int i = -1;
    {
        for (auto &f : changedFiles) {
            ++i;
            auto fref = initialGS->findFileByPath(f->path());
            if (!fref.exists()) {
                logger->debug("Taking sad path because {} is a new file", f->path());
                return false;
            } else {
                auto &oldHash = globalStateHashes[fref.id()];
                ENFORCE(oldHash.definitions.hierarchyHash != core::GlobalStateHash::HASH_STATE_NOT_COMPUTED);
                if (hashes[i].definitions.hierarchyHash != core::GlobalStateHash::HASH_STATE_INVALID &&
                    hashes[i].definitions.hierarchyHash != oldHash.definitions.hierarchyHash) {
                    logger->debug("Taking sad path because {} has changed definitions", f->path());
                    return false;
                }
            }
        }
    }
    return true;
}

LSPLoop::TypecheckRun LSPLoop::tryFastPath(unique_ptr<core::GlobalState> gs, std::optional<FileUpdates> maybeUpdates,
                                           const vector<core::FileRef> &filesForQuery) const {
    auto finalGs = move(gs);
    // We assume finalGs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(finalGs->lspTypecheckCount > 0,
            "Tried to run fast path with a GlobalState object that never had inferencer and resolver runs.");

    bool takeFastPath = true;
    vector<core::FileRef> subset;
    vector<core::NameHash> changedHashes;
    if (maybeUpdates.has_value()) {
        Timer timeit(logger, "fast_path_decision");

        auto &updates = maybeUpdates.value();
        auto hashes = computeStateHashes(updates.updatedFiles);
        logger->debug("Trying to see if happy path is available after {} file changes", updates.updatedFiles.size());
        ENFORCE(updates.updatedFiles.size() == hashes.size());
        takeFastPath = canTakeFastPath(updates, hashes);

        int i = -1;
        for (auto &f : updates.updatedFiles) {
            ++i;
            auto fref = initialGS->findFileByPath(f->path());
            if (fref.exists() && takeFastPath) {
                // Update to existing file on fast path
                auto &oldHash = globalStateHashes[fref.id()];
                for (auto &p : hashes[i].definitions.methodHashes) {
                    auto fnd = oldHash.definitions.methodHashes.find(p.first);
                    ENFORCE(fnd != oldHash.definitions.methodHashes.end(), "definitionHash should have failed");
                    if (fnd->second != p.second) {
                        changedHashes.emplace_back(p.first);
                    }
                }
                finalGs = core::GlobalState::replaceFile(move(finalGs), fref, f);
                subset.emplace_back(fref);
            }
            // Note: We may not have an id yet for this file if it is brand new, so we store hashes with their paths.
            updates.updatedFileHashes.push_back(make_pair(f->path(), hashes[i]));
        }
        core::NameHash::sortAndDedupe(changedHashes);
    }

    if (takeFastPath) {
        Timer timeit(logger, "fast_path");
        int i = -1;
        for (auto &oldHash : globalStateHashes) {
            i++;
            vector<core::NameHash> intersection;
            std::set_intersection(changedHashes.begin(), changedHashes.end(), oldHash.usages.sends.begin(),
                                  oldHash.usages.sends.end(), std::back_inserter(intersection));
            if (!intersection.empty()) {
                auto ref = core::FileRef(i);
                logger->debug("Added {} to update set as used a changed method",
                              !ref.exists() ? "" : ref.data(*finalGs).path());
                subset.emplace_back(ref);
            }
        }
        // Remove any duplicate files.
        fast_sort(subset);
        subset.resize(std::distance(subset.begin(), std::unique(subset.begin(), subset.end())));

        prodCategoryCounterInc("lsp.updates", "fastpath");
        logger->debug("Taking fast path");
        ENFORCE(initialGS->errorQueue->isEmpty());
        vector<ast::ParsedFile> updatedIndexed;
        for (auto &f : subset) {
            unique_ptr<KeyValueStore> kvstore; // nullptr
            auto t = pipeline::indexOne(opts, *finalGs, f, kvstore);
            updatedIndexed.emplace_back(ast::ParsedFile{t.tree->deepCopy(), t.file});
            // We can't have changed files without file updates.
            ENFORCE(maybeUpdates.has_value());
            (*maybeUpdates).updatedFileIndexes.push_back(move(t));
        }

        for (auto &f : filesForQuery) {
            const int id = f.id();
            const auto it = indexedFinalGS.find(id);
            const auto &parsedFile = it == indexedFinalGS.end() ? indexed[id] : it->second;
            if (parsedFile.tree) {
                updatedIndexed.emplace_back(ast::ParsedFile{parsedFile.tree->deepCopy(), parsedFile.file});
            }
        }
        subset.insert(subset.end(), filesForQuery.begin(), filesForQuery.end());

        auto resolved = pipeline::incrementalResolve(*finalGs, move(updatedIndexed), opts);
        tryApplyDefLocSaver(*finalGs, resolved);
        tryApplyLocalVarSaver(*finalGs, resolved);
        pipeline::typecheck(finalGs, move(resolved), opts, workers);
        auto out = initialGS->errorQueue->drainWithQueryResponses();
        finalGs->lspTypecheckCount++;
        return TypecheckRun{move(out.first), move(subset), move(out.second), move(finalGs), move(maybeUpdates), true};
    } else {
        return runSlowPath(move(maybeUpdates));
    }
}
} // namespace sorbet::realmain::lsp
