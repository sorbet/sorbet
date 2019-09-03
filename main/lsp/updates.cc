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
    if (loop.config.enableOperationNotifications) {
        auto params = make_unique<SorbetShowOperationParams>(this->operationName, this->description,
                                                             SorbetOperationStatus::Start);
        loop.sendMessage(
            LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetShowOperation, move(params))));
    }
}

LSPLoop::ShowOperation::~ShowOperation() {
    if (loop.config.enableOperationNotifications) {
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
    auto &updates = run.updates;
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
        int i = -1;
        for (auto &file : updates.updatedFiles) {
            i++;
            // Update initialGS and index.
            auto rv = updateFile(move(initialGS), file, config.opts);
            initialGS = move(rv.first);
            const auto id = rv.second.file.id();
            if (id >= indexed.size()) {
                indexed.resize(id + 1);
            }
            indexed[id] = move(rv.second);
            if (id >= globalStateHashes.size()) {
                globalStateHashes.resize(id + 1);
            }
            globalStateHashes[id] = move(updates.updatedFileHashes[i]);
        }
        // Drop any indexing errors produced during `updateFile`.
        // (Note: Flushing is disabled in LSP mode, so we have to drain.)
        errorQueue->drainWithQueryResponses();
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
    vector<core::FileRef> inputFiles = pipeline::reserveFiles(initialGS, config.opts.inputFileNames);
    for (auto &t : pipeline::index(initialGS, inputFiles, config.opts, workers, kvstore)) {
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

LSPLoop::TypecheckRun LSPLoop::runSlowPath(FileUpdates updates) const {
    ShowOperation slowPathOp(*this, "SlowPath", "Typechecking...");
    Timer timeit(logger, "slow_path");
    ENFORCE(initialGS->errorQueue->isEmpty());
    prodCategoryCounterInc("lsp.updates", "slowpath");
    logger->debug("Taking slow path");

    UnorderedSet<int> updatedFiles;
    vector<ast::ParsedFile> indexedCopies;
    auto finalGS = initialGS->deepCopy(true);
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
                updates.updatedFileIndexes.push_back(move(ast));
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

    ENFORCE(finalGS->lspQuery.isEmpty());
    auto resolved = pipeline::resolve(finalGS, move(indexedCopies), config.opts, workers, config.skipConfigatron);
    vector<core::FileRef> affectedFiles;
    for (auto &tree : resolved) {
        ENFORCE(tree.file.exists());
        affectedFiles.push_back(tree.file);
    }
    pipeline::typecheck(finalGS, move(resolved), config.opts, workers);
    auto out = initialGS->errorQueue->drainWithQueryResponses();
    finalGS->lspTypecheckCount++;
    finalGS->lspQuery = core::lsp::Query::noQuery();
    return TypecheckRun{move(out.first), move(affectedFiles), move(finalGS), move(updates), false};
}

bool LSPLoop::canTakeFastPath(const FileUpdates &updates, const vector<core::FileHash> &hashes) const {
    if (config.disableFastPath) {
        logger->debug("Taking slow path because fast path is disabled.");
        return false;
    }
    auto &changedFiles = updates.updatedFiles;
    logger->debug("Trying to see if fast path is available after {} file changes", changedFiles.size());

    ENFORCE(changedFiles.size() == hashes.size());
    int i = -1;
    {
        for (auto &f : changedFiles) {
            ++i;
            auto fref = initialGS->findFileByPath(f->path());
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

LSPLoop::TypecheckRun LSPLoop::runTypechecking(unique_ptr<core::GlobalState> gs, FileUpdates updates) const {
    // We assume gs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(gs->lspTypecheckCount > 0,
            "Tried to run fast path with a GlobalState object that never had inferencer and resolver runs.");

    bool takeFastPath = false;
    vector<core::FileRef> subset;
    vector<core::NameHash> changedHashes;
    {
        Timer timeit(logger, "fast_path_decision");
        updates.updatedFileHashes = computeStateHashes(updates.updatedFiles);
        const auto &hashes = updates.updatedFileHashes;
        logger->debug("Trying to see if fast path is available after {} file changes", updates.updatedFiles.size());
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
                gs = core::GlobalState::replaceFile(move(gs), fref, f);
                subset.emplace_back(fref);
            }
        }
        core::NameHash::sortAndDedupe(changedHashes);
    }

    if (!takeFastPath) {
        return runSlowPath(move(updates));
    }

    Timer timeit(logger, "fast_path");
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
    ENFORCE(initialGS->errorQueue->isEmpty());
    vector<ast::ParsedFile> updatedIndexed;
    for (auto &f : subset) {
        unique_ptr<KeyValueStore> kvstore; // nullptr
        // TODO: Thread through kvstore.
        ENFORCE(this->kvstore == nullptr);
        auto t = pipeline::indexOne(config.opts, *gs, f, kvstore);
        updatedIndexed.emplace_back(ast::ParsedFile{t.tree->deepCopy(), t.file});
        updates.updatedFileIndexes.push_back(move(t));
    }

    ENFORCE(gs->lspQuery.isEmpty());
    auto resolved = pipeline::incrementalResolve(*gs, move(updatedIndexed), config.opts);
    pipeline::typecheck(gs, move(resolved), config.opts, workers);
    auto out = initialGS->errorQueue->drainWithQueryResponses();
    gs->lspTypecheckCount++;
    return TypecheckRun{move(out.first), move(subset), move(gs), move(updates), true};
}

LSPLoop::QueryRun LSPLoop::runQuery(unique_ptr<core::GlobalState> gs, const core::lsp::Query &q,
                                    const vector<core::FileRef> &filesForQuery) const {
    // We assume gs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(gs->lspTypecheckCount > 0,
            "Tried to run a query with a GlobalState object that never had inferencer and resolver runs.");

    Timer timeit(logger, "query");
    prodCategoryCounterInc("lsp.updates", "query");
    ENFORCE(initialGS->errorQueue->isEmpty());
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
    auto out = initialGS->errorQueue->drainWithQueryResponses();
    gs->lspTypecheckCount++;
    gs->lspQuery = core::lsp::Query::noQuery();
    return QueryRun{move(gs), move(out.second)};
}
} // namespace sorbet::realmain::lsp
