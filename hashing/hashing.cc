#include "hashing/hashing.h"
#include "ast/substitute/substitute.h"
#include "ast/treemap/treemap.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "core/ErrorQueue.h"
#include "core/GlobalSubstitution.h"
#include "core/NameHash.h"
#include "core/NullFlusher.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "main/pipeline/pipeline.h"

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
    core::LazyGlobalSubstitution subst(originalGS, newGS);
    core::MutableContext ctx(newGS, core::Symbols::root(), newFref);
    core::UnfreezeNameTable nameTableAccess(newGS);
    rewritten.tree = ast::Substitute::run(ctx, subst, move(rewritten.tree));
    return make_pair<ast::ParsedFile, core::UsageHash>(move(rewritten), subst.getAllNames());
}

unique_ptr<core::FileHash> computeFileHashForAST(unique_ptr<core::GlobalState> &lgs, core::UsageHash usageHash,
                                                 ast::ParsedFile file) {
    vector<ast::ParsedFile> single;
    single.emplace_back(move(file));

    core::Context ctx(*lgs, core::Symbols::root(), single[0].file);
    auto workers = WorkerPool::create(0, lgs->tracer());
    realmain::pipeline::resolve(lgs, move(single), opts(), *workers);

    return make_unique<core::FileHash>(move(*lgs->hash()), move(usageHash));
}

// Note: lgs is an outparameter.
core::FileRef makeEmptyGlobalStateForFile(spdlog::logger &logger, shared_ptr<core::File> forWhat,
                                          unique_ptr<core::GlobalState> &lgs,
                                          const realmain::options::Options &hashingOpts) {
    lgs = make_unique<core::GlobalState>(
        (make_shared<core::ErrorQueue>(logger, logger, make_shared<core::NullFlusher>())));
    lgs->initEmpty();
    lgs->silenceErrors = true;
    lgs->requiresAncestorEnabled = hashingOpts.requiresAncestorEnabled;
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

    // Calculate UsageHash. We use LazyGlobalSubstitution for this purpose, but it will not do any actual substitution
    // when fromGS == toGS (hence we intentionally do not unfreeze name table).
    core::LazyGlobalSubstitution subst(*lgs, *lgs);
    core::MutableContext ctx(*lgs, core::Symbols::root(), fref);
    ast.tree = ast::Substitute::run(ctx, subst, move(ast.tree));
    return computeFileHashForAST(lgs, subst.getAllNames(), move(ast));
}

struct FileHashJob {
    bool compressed;
    size_t index;
};

void computeFileHashesFromIndexResult(realmain::pipeline::IndexResult &indexResult, spdlog::logger &logger,
                                      WorkerPool &workers, const unique_ptr<const OwnedKeyValueStore> &kvstore,
                                      const realmain::options::Options &hashingOpts) {
    // Rewrite ASTs to an empty GlobalState and use them for hashing.
    auto fileq =
        make_shared<ConcurrentBoundedQueue<FileHashJob>>(indexResult.trees.size() + indexResult.compressedTrees.size());
    for (size_t i = 0; i < indexResult.trees.size(); i++) {
        fileq->push(FileHashJob{false, i}, 1);
    }
    for (size_t i = 0; i < indexResult.compressedTrees.size(); i++) {
        fileq->push(FileHashJob{true, i}, 1);
    }

    logger.debug("Computing state hashes for {} files", indexResult.trees.size() + indexResult.compressedTrees.size());

    const core::GlobalState &sharedGs = *indexResult.gs;
    const vector<ast::ParsedFile> &trees = indexResult.trees;
    const vector<ast::CompressedParsedFile> &compressedTrees = indexResult.compressedTrees;
    auto resultq = make_shared<BlockingBoundedQueue<vector<pair<core::FileRef, unique_ptr<const core::FileHash>>>>>(
        indexResult.trees.size() + indexResult.compressedTrees.size());
    Timer timeit(logger, "computeFileHashes");
    workers.multiplexJob("lspStateHash", [fileq, resultq, &trees, &compressedTrees, &sharedGs, &logger,
                                          &hashingOpts]() {
        unique_ptr<Timer> timeit;
        vector<pair<core::FileRef, unique_ptr<const core::FileHash>>> threadResult;
        int processedByThread = 0;
        FileHashJob job;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    if (timeit == nullptr) {
                        timeit = make_unique<Timer>(logger, "computeFileHashesWorker");
                    }
                    processedByThread++;

                    ast::ParsedFile temp;
                    const ast::ParsedFile *ast;
                    if (job.compressed) {
                        const auto &compressed = compressedTrees[job.index];
                        if (!compressed.file.exists() || compressed.file.data(sharedGs).getFileHash() != nullptr) {
                            continue;
                        }

                        // Decompress and hash. This is expected to be an uncommon case, as 1) all compressedFiles
                        // are cached and 2) LSP caches files, ASTs, _and_ their hashes. However, if a user runs Sorbet
                        // on the CLI, it will cache files and ASTs but no hashes, so we need to handle this case.
                        temp = ast::ParsedFile{core::serialize::Serializer::loadAST(sharedGs, compressed.tree->data()),
                                               compressed.file};
                        ast = &temp;
                    } else {
                        ast = &trees[job.index];

                        if (!ast->file.exists() || ast->file.data(sharedGs).getFileHash() != nullptr) {
                            continue;
                        }
                    }

                    unique_ptr<core::GlobalState> lgs;
                    auto newFref =
                        makeEmptyGlobalStateForFile(logger, sharedGs.getFiles()[ast->file.id()], lgs, hashingOpts);
                    auto [rewrittenAST, usageHash] = rewriteAST(sharedGs, *lgs, newFref, *ast);

                    threadResult.emplace_back(ast->file,
                                              computeFileHashForAST(lgs, move(usageHash), move(rewrittenAST)));
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
                    a.first.data(*indexResult.gs).setFileHash(move(a.second));
                }
            }
        }
    }
}

vector<ast::CompressedParsedFile> compressTrees(spdlog::logger &logger, vector<ast::ParsedFile> trees,
                                                WorkerPool &workers) {
    Timer timeit(logger, "Hashing::compressTrees");
    // Compress files in parallel.
    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
    vector<ast::CompressedParsedFile> output;
    output.reserve(trees.size());
    for (auto &tree : trees) {
        fileq->push(move(tree), 1);
    }

    auto resultq = make_shared<BlockingBoundedQueue<vector<ast::CompressedParsedFile>>>(trees.size());
    trees.clear();
    workers.multiplexJob("compressTreesAndFiles", [fileq, resultq]() {
        vector<ast::CompressedParsedFile> threadResult;
        int processedByThread = 0;
        ast::ParsedFile job;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;

                    if (!job.file.exists()) {
                        continue;
                    }

                    threadResult.emplace_back(ast::CompressedParsedFile{
                        make_unique<vector<u1>>(core::serialize::Serializer::storeAST(job)), job.file});
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    {
        vector<ast::CompressedParsedFile> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger); !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger)) {
            if (result.gotItem()) {
                for (auto &compressed : threadResult) {
                    output.emplace_back(move(compressed));
                }
            }
        }
    }
    return output;
}

}; // namespace

void Hashing::computeFileHashes(const vector<shared_ptr<core::File>> &files, spdlog::logger &logger,
                                WorkerPool &workers, const realmain::options::Options &opts) {
    Timer timeit(logger, "computeFileHashes");
    auto fileq = make_shared<ConcurrentBoundedQueue<size_t>>(files.size());
    for (size_t i = 0; i < files.size(); i++) {
        auto copy = i;
        fileq->push(move(copy), 1);
    }

    logger.debug("Computing state hashes for {} files", files.size());

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
    realmain::pipeline::IndexResult result{nullptr, {}, {}};
    result.trees = realmain::pipeline::index(gs, files, opts, workers, kvstore);
    ENFORCE_NO_TIMER(result.trees.size() == files.size());
    result.gs = move(gs);
    computeFileHashesFromIndexResult(result, logger, workers, kvstore, opts);
    gs = move(result.gs);
    return move(result.trees);
}

vector<ast::CompressedParsedFile>
Hashing::indexCompressAndComputeFileHashes(unique_ptr<core::GlobalState> &gs, const realmain::options::Options &opts,
                                           spdlog::logger &logger, vector<core::FileRef> &files, WorkerPool &workers,
                                           const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    auto result = realmain::pipeline::indexWithNoDecompression(move(gs), files, opts, workers, kvstore);

    ENFORCE_NO_TIMER(result.compressedTrees.size() + result.trees.size() == files.size());
    computeFileHashesFromIndexResult(result, logger, workers, kvstore, opts);
    gs = move(result.gs);

    if (!result.trees.empty()) {
        auto compressed = compressTrees(logger, move(result.trees), workers);
        result.compressedTrees.reserve(result.compressedTrees.size() + compressed.size());
        result.compressedTrees.insert(result.compressedTrees.end(), make_move_iterator(compressed.begin()),
                                      make_move_iterator(compressed.end()));
    }

    return move(result.compressedTrees);
}

} // namespace sorbet::hashing
