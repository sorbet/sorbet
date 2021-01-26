#include "hashing/hashing.h"
#include "ast/treemap/treemap.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "core/ErrorQueue.h"
#include "core/NameHash.h"
#include "core/NullFlusher.h"
#include "core/Unfreeze.h"
#include "main/pipeline/pipeline.h"

using namespace std;
namespace sorbet::hashing {
namespace {
class AllNamesCollector {
public:
    core::UsageHash acc;
    ast::ExpressionPtr preTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        acc.sends.emplace_back(ctx, ast::cast_tree_nonnull<ast::Send>(tree).fun);
        return tree;
    }

    ast::ExpressionPtr postTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        acc.constants.emplace_back(ctx, original.name);
        return tree;
    }

    void handleUnresolvedConstantLit(core::Context ctx, ast::UnresolvedConstantLit *expr) {
        while (expr) {
            acc.constants.emplace_back(ctx, expr->cnst);
            // Handle references to 'Foo' in 'Foo::Bar'.
            expr = ast::cast_tree<ast::UnresolvedConstantLit>(expr->scope);
        }
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        acc.constants.emplace_back(ctx, original.symbol.data(ctx)->name);

        handleUnresolvedConstantLit(ctx, ast::cast_tree<ast::UnresolvedConstantLit>(original.name));

        // Grab names of superclasses. (N.B. `include` and `extend` are captured as ConstantLits.)
        for (auto &ancst : original.ancestors) {
            handleUnresolvedConstantLit(ctx, ast::cast_tree<ast::UnresolvedConstantLit>(ancst));
        }

        return tree;
    }

    ast::ExpressionPtr postTransformUnresolvedConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(tree);
        handleUnresolvedConstantLit(ctx, &original);
        return tree;
    }

    ast::ExpressionPtr postTransformUnresolvedIdent(core::Context ctx, ast::ExpressionPtr tree) {
        auto &id = ast::cast_tree_nonnull<ast::UnresolvedIdent>(tree);
        if (id.kind != ast::UnresolvedIdent::Kind::Local) {
            acc.constants.emplace_back(ctx, id.name);
        }
        return tree;
    }
};

core::UsageHash getAllNames(core::Context ctx, ast::ExpressionPtr &tree) {
    AllNamesCollector collector;
    tree = ast::TreeMap::apply(ctx, collector, move(tree));
    core::NameHash::sortAndDedupe(collector.acc.sends);
    core::NameHash::sortAndDedupe(collector.acc.constants);
    return move(collector.acc);
};

core::FileHash computeFileHash(shared_ptr<core::File> forWhat, spdlog::logger &logger) {
    Timer timeit(logger, "computeFileHash");
    const static realmain::options::Options emptyOpts{};
    unique_ptr<core::GlobalState> lgs = make_unique<core::GlobalState>(
        (make_shared<core::ErrorQueue>(logger, logger, make_shared<core::NullFlusher>())));
    lgs->initEmpty();
    lgs->silenceErrors = true;
    core::FileRef fref;
    {
        core::UnfreezeFileTable fileTableAccess(*lgs);
        fref = lgs->enterFile(forWhat);
        fref.data(*lgs).strictLevel = realmain::pipeline::decideStrictLevel(*lgs, fref, emptyOpts);
    }
    vector<ast::ParsedFile> single;

    single.emplace_back(realmain::pipeline::indexOne(emptyOpts, *lgs, fref));
    core::Context ctx(*lgs, core::Symbols::root(), single[0].file);
    auto allNames = getAllNames(ctx, single[0].tree);
    auto workers = WorkerPool::create(0, lgs->tracer());
    realmain::pipeline::resolve(lgs, move(single), emptyOpts, *workers);

    return {move(*lgs->hash()), move(allNames)};
}
}; // namespace

void Hashing::computeFileHashes(const vector<shared_ptr<core::File>> &files, spdlog::logger &logger,
                                WorkerPool &workers) {
    Timer timeit(logger, "computeFileHashes");
    shared_ptr<ConcurrentBoundedQueue<int>> fileq = make_shared<ConcurrentBoundedQueue<int>>(files.size());
    for (int i = 0; i < files.size(); i++) {
        auto copy = i;
        fileq->push(move(copy), 1);
    }

    logger.debug("Computing state hashes for {} files", files.size());

    shared_ptr<BlockingBoundedQueue<vector<pair<int, unique_ptr<const core::FileHash>>>>> resultq =
        make_shared<BlockingBoundedQueue<vector<pair<int, unique_ptr<const core::FileHash>>>>>(files.size());
    workers.multiplexJob("lspStateHash", [fileq, resultq, &files, &logger]() {
        vector<pair<int, unique_ptr<const core::FileHash>>> threadResult;
        int processedByThread = 0;
        int job;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;

                    if (!files[job] || files[job]->getFileHash() != nullptr) {
                        continue;
                    }

                    threadResult.emplace_back(job, make_unique<core::FileHash>(computeFileHash(files[job], logger)));
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    {
        vector<pair<int, unique_ptr<const core::FileHash>>> threadResult;
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

} // namespace sorbet::hashing