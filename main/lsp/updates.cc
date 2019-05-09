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

LSPLoop::ShowOperation::ShowOperation(LSPLoop &loop, string_view operationName, string_view description)
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

core::FileRef LSPLoop::updateFile(const shared_ptr<core::File> &file) {
    Timer timeit(logger, "updateFile");
    core::FileRef fref;
    if (!file)
        return fref;
    fref = initialGS->findFileByPath(file->path());
    if (fref.exists()) {
        initialGS = core::GlobalState::replaceFile(move(initialGS), fref, move(file));
    } else {
        fref = initialGS->enterFile(move(file));
    }

    vector<string> emptyInputNames;
    auto t = pipeline::indexOne(opts, *initialGS, fref, kvstore);
    int id = t.file.id();
    if (id >= indexed.size()) {
        indexed.resize(id + 1);
    }
    indexed[id] = move(t);
    return fref;
}

vector<core::FileHash> LSPLoop::computeStateHashes(const vector<shared_ptr<core::File>> &files) {
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
        for (auto result = resultq->wait_pop_timed(threadResult, pipeline::PROGRESS_REFRESH_TIME_MILLIS, *logger);
             !result.done();
             result = resultq->wait_pop_timed(threadResult, pipeline::PROGRESS_REFRESH_TIME_MILLIS, *logger)) {
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
    ShowOperation op(*this, "Indexing", "Sorbet: Indexing files...");
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
    if (gs.lspQuery.kind != core::lsp::Query::Kind::LOC) {
        return;
    }
    for (auto &t : indexedCopies) {
        DefLocSaver defLocSaver;
        core::Context ctx(gs, core::Symbols::root());
        t.tree = ast::TreeMap::apply(ctx, defLocSaver, move(t.tree));
    }
}

LSPLoop::TypecheckRun LSPLoop::runSlowPath(const vector<shared_ptr<core::File>> &changedFiles) {
    ShowOperation slowPathOp(*this, "SlowPath", "Sorbet: Typechecking...");
    Timer timeit(logger, "slow_path");
    ENFORCE(initialGS->errorQueue->isEmpty());
    prodCategoryCounterInc("lsp.updates", "slowpath");
    logger->debug("Taking slow path");

    core::UnfreezeFileTable fileTableAccess(*initialGS);
    indexed.reserve(indexed.size() + changedFiles.size());
    for (auto &t : changedFiles) {
        updateFile(t);
    }

    vector<ast::ParsedFile> indexedCopies;
    for (const auto &tree : indexed) {
        if (tree.tree) {
            indexedCopies.emplace_back(ast::ParsedFile{tree.tree->deepCopy(), tree.file});
        }
    }

    auto finalGs = initialGS->deepCopy(true);
    auto resolved = pipeline::resolve(*finalGs, move(indexedCopies), opts, skipConfigatron);
    tryApplyDefLocSaver(*finalGs, resolved);
    tryApplyLocalVarSaver(*finalGs, resolved);
    vector<core::FileRef> affectedFiles;
    for (auto &tree : resolved) {
        ENFORCE(tree.file.exists());
        affectedFiles.push_back(tree.file);
    }
    pipeline::typecheck(finalGs, move(resolved), opts, workers);
    auto out = initialGS->errorQueue->drainWithQueryResponses();
    finalGs->lspTypecheckCount++;
    return TypecheckRun{move(out.first), move(affectedFiles), move(out.second), move(finalGs)};
}

LSPLoop::TypecheckRun LSPLoop::tryFastPath(unique_ptr<core::GlobalState> gs,
                                           vector<shared_ptr<core::File>> &changedFiles, bool allFiles) {
    if (disableFastPath) {
        logger->debug("Taking sad path because happy path is disabled.");
        return runSlowPath(changedFiles);
    }

    auto finalGs = move(gs);
    // We assume finalGs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(finalGs->lspTypecheckCount > 0,
            "Tried to run fast path with a GlobalState object that never had inferencer and resolver runs.");
    logger->debug("Trying to see if happy path is available after {} file changes", changedFiles.size());
    bool good = true;
    auto hashes = computeStateHashes(changedFiles);
    ENFORCE(changedFiles.size() == hashes.size());
    vector<core::FileRef> subset;
    vector<core::NameHash> changedHashes;
    int i = -1;
    {
        core::UnfreezeFileTable fileTableAccess(*initialGS);
        for (auto &f : changedFiles) {
            ++i;
            if (!f) {
                continue;
            }
            auto wasFiles = initialGS->filesUsed();
            auto fref = updateFile(f);
            if (wasFiles != initialGS->filesUsed()) {
                logger->debug("Taking sad path because {} is a new file", changedFiles[i]->path());
                good = false;
                if (globalStateHashes.size() <= fref.id()) {
                    globalStateHashes.resize(fref.id() + 1);
                    globalStateHashes[fref.id()] = hashes[i];
                }
            } else {
                auto &oldHash = globalStateHashes[fref.id()];
                ENFORCE(oldHash.definitions.hierarchyHash != core::GlobalStateHash::HASH_STATE_NOT_COMPUTED);
                if (hashes[i].definitions.hierarchyHash != core::GlobalStateHash::HASH_STATE_INVALID &&
                    hashes[i].definitions.hierarchyHash != oldHash.definitions.hierarchyHash) {
                    logger->debug("Taking sad path because {} has changed definitions", changedFiles[i]->path());
                    good = false;
                    oldHash = hashes[i];
                }
                if (good) {
                    // TODO: update list of sends in this file, mark all(other) files that had same namerefs that this
                    // defines for retypechecking
                    for (auto &p : hashes[i].definitions.methodHashes) {
                        auto fnd = oldHash.definitions.methodHashes.find(p.first);
                        ENFORCE(fnd != oldHash.definitions.methodHashes.end(), "definitionHash should have failed");
                        if (fnd->second != p.second) {
                            changedHashes.emplace_back(p.first);
                        }
                    }
                    finalGs = core::GlobalState::replaceFile(move(finalGs), fref, changedFiles[i]);
                }

                subset.emplace_back(fref);
            }
        }
        fast_sort(changedHashes);
        changedHashes.resize(
            std::distance(changedHashes.begin(), std::unique(changedHashes.begin(), changedHashes.end())));
    }

    if (good) {
        Timer timeit(logger, "fast_path");
        if (allFiles) {
            subset.clear();
            for (int i = 1; i < finalGs->filesUsed(); i++) {
                core::FileRef fref(i);
                if (fref.data(*finalGs).sourceType == core::File::Type::Normal) {
                    subset.emplace_back(core::FileRef(i));
                }
            }
        } else {
            int i = -1;
            for (auto &oldHash : globalStateHashes) {
                i++;
                vector<core::NameHash> intersection;
                std::set_intersection(changedHashes.begin(), changedHashes.end(), oldHash.usages.usages.begin(),
                                      oldHash.usages.usages.end(), intersection.begin());
                if (!intersection.empty()) {
                    logger->debug("Added {} to update set as used a changed method");
                    subset.emplace_back(core::FileRef(i));
                }
            }
        }
        prodCategoryCounterInc("lsp.updates", "fastpath");
        ENFORCE(initialGS->errorQueue->isEmpty());
        vector<ast::ParsedFile> updatedIndexed;
        for (auto &f : subset) {
            auto t = pipeline::indexOne(opts, *finalGs, f, kvstore);
            int id = t.file.id();
            indexed[id] = move(t);
            updatedIndexed.emplace_back(ast::ParsedFile{indexed[id].tree->deepCopy(), indexed[id].file});
        }

        auto resolved = pipeline::incrementalResolve(*finalGs, move(updatedIndexed), opts);
        tryApplyDefLocSaver(*finalGs, resolved);
        tryApplyLocalVarSaver(*finalGs, resolved);
        pipeline::typecheck(finalGs, move(resolved), opts, workers);
        auto out = initialGS->errorQueue->drainWithQueryResponses();
        finalGs->lspTypecheckCount++;
        return TypecheckRun{move(out.first), move(subset), move(out.second), move(finalGs)};
    } else {
        return runSlowPath(changedFiles);
    }
}
} // namespace sorbet::realmain::lsp
