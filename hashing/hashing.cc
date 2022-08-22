#include "hashing/hashing.h"
#include "ast/substitute/substitute.h"
#include "ast/treemap/treemap.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "core/ErrorQueue.h"
#include "core/FileHash.h"
#include "core/NameSubstitution.h"
#include "core/Unfreeze.h"
#include "main/pipeline/pipeline.h"
#include "rapidjson/writer.h"

using namespace std;
namespace sorbet::hashing {
namespace {
const realmain::options::Options &opts() {
    const static realmain::options::Options emptyOpts{};
    return emptyOpts;
};

pair<ast::ParsedFile, core::UsageHash> rewriteAST(const core::GlobalState &originalGS, core::GlobalState &newGS,
                                                  core::FileRef newFref, const ast::ParsedFile &ast) {
    // TODO(jvilk): Switch to passing around compressed ASTs which are cheaper to copy + inflate.
    ast::ParsedFile rewritten{ast.tree.deepCopy(), newFref};
    core::LazyNameSubstitution subst(originalGS, newGS);
    core::MutableContext ctx(newGS, core::Symbols::root(), newFref);
    core::UnfreezeNameTable nameTableAccess(newGS);
    rewritten = ast::Substitute::run(ctx, subst, move(rewritten));
    return make_pair<ast::ParsedFile, core::UsageHash>(move(rewritten), subst.getAllNames());
}

bool isEmptyParseResult(const core::GlobalState &gs, const ast::ExpressionPtr &tree) {
    if (ast::isa_tree<ast::EmptyTree>(tree)) {
        return true;
    }

    auto *classDef = ast::cast_tree<ast::ClassDef>(tree);
    if (classDef == nullptr || classDef->symbol != core::Symbols::root()) {
        return false;
    }

    return classDef->rhs.empty();
}

unique_ptr<core::FileHash> computeFileHashForAST(spdlog::logger &logger, unique_ptr<core::GlobalState> &lgs,
                                                 core::UsageHash usageHash, ast::ParsedFile file) {
    if (file.file.data(*lgs).hasParseErrors()) {
        if (isEmptyParseResult(*lgs, file.tree)) {
            rapidjson::StringBuffer result;
            rapidjson::Writer<rapidjson::StringBuffer> writer(result);

            {
                writer.StartObject();

                writer.String("compute_file_hash_for_ast_empty");
                writer.Bool(true);

                writer.String("file_path");
                auto path = file.file.data(*lgs).path();
                writer.String(path.data(), path.size());

                writer.String("contents");
                auto source = file.file.data(*lgs).source();
                writer.String(source.data(), source.size());

                writer.EndObject();
            }

            auto view = string_view{result.GetString(), result.GetLength()};
            logger.debug(view);

            return make_unique<core::FileHash>(core::LocalSymbolTableHashes::invalidParse(), move(usageHash),
                                               core::FoundDefHashes{});
        }
    }

    vector<ast::ParsedFile> single;
    single.emplace_back(move(file));

    auto workers = WorkerPool::create(0, lgs->tracer());
    core::FoundDefHashes foundHashes; // out parameter
    realmain::pipeline::resolve(lgs, move(single), opts(), *workers, &foundHashes);

    return make_unique<core::FileHash>(move(*lgs->hash()), move(usageHash), move(foundHashes));
}

// Note: lgs is an outparameter.
core::FileRef makeEmptyGlobalStateForFile(spdlog::logger &logger, shared_ptr<core::File> forWhat,
                                          unique_ptr<core::GlobalState> &lgs,
                                          const realmain::options::Options &hashingOpts) {
    lgs = core::GlobalState::makeEmptyGlobalStateForHashing(logger);
    lgs->requiresAncestorEnabled = hashingOpts.requiresAncestorEnabled;
    lgs->lspExperimentalFastPathEnabled = hashingOpts.lspExperimentalFastPathEnabled;
    {
        core::UnfreezeFileTable fileTableAccess(*lgs);
        auto fref = lgs->enterFile(forWhat);
        fref.data(*lgs).strictLevel = realmain::pipeline::decideStrictLevel(*lgs, fref, opts());
        return fref;
    }
}

unique_ptr<core::FileHash> computeFileHashForFile(shared_ptr<core::File> forWhat, spdlog::logger &logger,
                                                  const realmain::options::Options &hashingOpts) {
    Timer timeit(logger, "computeFileHash");
    unique_ptr<core::GlobalState> lgs;
    core::FileRef fref = makeEmptyGlobalStateForFile(logger, move(forWhat), /* out param */ lgs, hashingOpts);
    auto ast = realmain::pipeline::indexOne(opts(), *lgs, fref);

    // Calculate UsageHash. We use LazyNameSubstitution for this purpose, but it will not do any actual substitution
    // when fromGS == toGS (hence we intentionally do not unfreeze name table).
    core::LazyNameSubstitution subst(*lgs, *lgs);
    core::MutableContext ctx(*lgs, core::Symbols::root(), fref);
    ast = ast::Substitute::run(ctx, subst, move(ast));
    return computeFileHashForAST(logger, lgs, subst.getAllNames(), move(ast));
}
}; // namespace

void Hashing::computeFileHashes(const vector<shared_ptr<core::File>> &files, spdlog::logger &logger,
                                WorkerPool &workers, const realmain::options::Options &opts) {
    Timer timeit(logger, "computeFileHashes");
    auto fileq = make_shared<ConcurrentBoundedQueue<size_t>>(files.size());
    for (size_t i = 0; i < files.size(); i++) {
        fileq->push(i, 1);
    }

    logger.trace("Computing state hashes for {} files", files.size());

    auto resultq =
        make_shared<BlockingBoundedQueue<vector<pair<size_t, unique_ptr<const core::FileHash>>>>>(files.size());
    workers.multiplexJob("lspStateHash", [fileq, resultq, &files, &logger, &opts]() {
        vector<pair<size_t, unique_ptr<const core::FileHash>>> threadResult;
        int processedByThread = 0;
        size_t job;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;

                    if (!files[job] || files[job]->getFileHash() != nullptr) {
                        continue;
                    }

                    threadResult.emplace_back(job, computeFileHashForFile(files[job], logger, opts));
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    {
        vector<pair<size_t, unique_ptr<const core::FileHash>>> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger); !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger)) {
            if (result.gotItem()) {
                for (auto &a : threadResult) {
                    files[a.first]->setFileHash(move(a.second));
                }
            }
        }
    }
}

vector<ast::ParsedFile> Hashing::indexAndComputeFileHashes(unique_ptr<core::GlobalState> &gs,
                                                           const realmain::options::Options &opts,
                                                           spdlog::logger &logger, vector<core::FileRef> &files,
                                                           WorkerPool &workers,
                                                           const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    auto asts = realmain::pipeline::index(*gs, files, opts, workers, kvstore);
    ENFORCE_NO_TIMER(asts.size() == files.size());

    // Below, we rewrite ASTs to an empty GlobalState and use them for hashing.
    auto fileq = make_shared<ConcurrentBoundedQueue<size_t>>(asts.size());
    for (size_t i = 0; i < asts.size(); i++) {
        fileq->push(i, 1);
    }

    logger.trace("Computing state hashes for {} files", asts.size());

    const core::GlobalState &sharedGs = *gs;
    auto resultq =
        make_shared<BlockingBoundedQueue<vector<pair<core::FileRef, unique_ptr<const core::FileHash>>>>>(asts.size());
    Timer timeit(logger, "computeFileHashes");
    workers.multiplexJob("lspStateHash", [fileq, resultq, &asts, &sharedGs, &logger, &opts]() {
        unique_ptr<Timer> timeit;
        vector<pair<core::FileRef, unique_ptr<const core::FileHash>>> threadResult;
        int processedByThread = 0;
        size_t job;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    if (timeit == nullptr) {
                        timeit = make_unique<Timer>(logger, "computeFileHashesWorker");
                    }
                    processedByThread++;

                    const auto &ast = asts[job];

                    if (!ast.file.exists() || ast.file.data(sharedGs).getFileHash() != nullptr) {
                        continue;
                    }

                    unique_ptr<core::GlobalState> lgs;
                    auto newFref = makeEmptyGlobalStateForFile(logger, sharedGs.getFiles()[ast.file.id()], lgs, opts);
                    auto [rewrittenAST, usageHash] = rewriteAST(sharedGs, *lgs, newFref, ast);

                    threadResult.emplace_back(ast.file,
                                              computeFileHashForAST(logger, lgs, move(usageHash), move(rewrittenAST)));
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    {
        vector<pair<core::FileRef, unique_ptr<const core::FileHash>>> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger); !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger)) {
            if (result.gotItem()) {
                for (auto &a : threadResult) {
                    a.first.data(*gs).setFileHash(move(a.second));
                }
            }
        }
    }

    return asts;
}

} // namespace sorbet::hashing
