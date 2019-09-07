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

unique_ptr<LSPMessage> makeShowOperation(std::string_view operationName, std::string_view description,
                                         SorbetOperationStatus status) {
    return make_unique<LSPMessage>(make_unique<NotificationMessage>(
        "2.0", LSPMethod::SorbetShowOperation,
        make_unique<SorbetShowOperationParams>(string(operationName), string(description),
                                               SorbetOperationStatus::Start)));
}

LSPLoop::ShowOperation::ShowOperation(const LSPLoop &loop, string_view operationName, string_view description)
    : loop(loop), operationName(string(operationName)), description(string(description)) {
    if (loop.config.enableOperationNotifications) {
        loop.sendMessage(*makeShowOperation(this->operationName, this->description, SorbetOperationStatus::Start));
    }
}

LSPLoop::ShowOperation::~ShowOperation() {
    if (loop.config.enableOperationNotifications) {
        loop.sendMessage(*makeShowOperation(this->operationName, this->description, SorbetOperationStatus::End));
    }
}

ShowOperationPreprocessorThread::ShowOperationPreprocessorThread(const LSPConfiguration &config, absl::Mutex &mtx,
                                                                 std::deque<std::unique_ptr<LSPMessage>> &queue,
                                                                 std::string_view operationName,
                                                                 std::string_view description)
    : config(config), mtx(mtx), queue(queue), operationName(string(operationName)), description(string(description)) {
    if (config.enableOperationNotifications) {
        absl::MutexLock lck(&this->mtx);
        this->queue.push_back(makeShowOperation(this->operationName, this->description, SorbetOperationStatus::Start));
    }
}

ShowOperationPreprocessorThread::~ShowOperationPreprocessorThread() {
    if (config.enableOperationNotifications) {
        absl::MutexLock lck(&this->mtx);
        this->queue.push_back(makeShowOperation(this->operationName, this->description, SorbetOperationStatus::End));
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

    return pushDiagnostics(move(run));
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

LSPLoop::TypecheckRun LSPLoop::runSlowPath(LSPFileUpdates updates) const {
    ShowOperation slowPathOp(*this, "SlowPath", "Typechecking...");
    Timer timeit(logger, "slow_path");
    ENFORCE(!updates.canTakeFastPath || config.disableFastPath);
    ENFORCE(updates.updatedGS.has_value());
    if (!updates.updatedGS.has_value()) {
        Exception::raise("runSlowPath called with an update that lacks an updated global state.");
    }
    prodCategoryCounterInc("lsp.updates", "slowpath");
    logger->debug("Taking slow path");

    UnorderedSet<int> updatedFiles;
    vector<ast::ParsedFile> indexedCopies;
    auto finalGS = move(updates.updatedGS.value());
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

    ENFORCE(finalGS->lspQuery.isEmpty());
    auto resolved = pipeline::resolve(finalGS, move(indexedCopies), config.opts, workers, config.skipConfigatron);
    vector<core::FileRef> affectedFiles;
    for (auto &tree : resolved) {
        ENFORCE(tree.file.exists());
        affectedFiles.push_back(tree.file);
    }
    pipeline::typecheck(finalGS, move(resolved), config.opts, workers);
    auto out = finalGS->errorQueue->drainWithQueryResponses();
    finalGS->lspTypecheckCount++;
    finalGS->lspQuery = core::lsp::Query::noQuery();
    return TypecheckRun{move(out.first), move(affectedFiles), move(finalGS), move(updates), false};
}

LSPLoop::TypecheckRun LSPLoop::runTypechecking(unique_ptr<core::GlobalState> gs, LSPFileUpdates updates) const {
    // We assume gs is a copy of initialGS, which has had the inferencer & resolver run.
    ENFORCE(gs->lspTypecheckCount > 0,
            "Tried to run fast path with a GlobalState object that never had inferencer and resolver runs.");

    if (!updates.canTakeFastPath) {
        return runSlowPath(move(updates));
    }

    Timer timeit(logger, "fast_path");
    vector<core::FileRef> subset;
    vector<core::NameHash> changedHashes;
    {
        const auto &hashes = updates.updatedFileHashes;
        logger->debug("Trying to see if fast path is available after {} file changes", updates.updatedFiles.size());
        ENFORCE(updates.updatedFiles.size() == hashes.size());

        int i = -1;
        for (auto &f : updates.updatedFiles) {
            ++i;
            auto fref = gs->findFileByPath(f->path());
            // We don't support new files on the fast path. This enforce failing indicates a bug in our fast/slow path
            // logic in LSPPreprocessor.
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
    return TypecheckRun{move(out.first), move(subset), move(gs), move(updates), true};
}

LSPLoop::QueryRun LSPLoop::runQuery(unique_ptr<core::GlobalState> gs, const core::lsp::Query &q,
                                    const vector<core::FileRef> &filesForQuery) const {
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
    return QueryRun{move(gs), move(out.second)};
}
} // namespace sorbet::realmain::lsp
