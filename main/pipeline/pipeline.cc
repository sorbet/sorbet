#ifdef SORBET_REALMAIN_MIN
// minimal build to speedup compilation. Remove extra features
#else
// has to go first, as it violates poisons
#include "core/proto/proto.h"
// ^^ has to go first
#include "common/json2msgpack/json2msgpack.h"
#include "main/autogen/cache.h"
#include "main/autogen/constant_hash.h"
#include "packager/packager.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"
#include <sstream>
#endif
#include "ProgressIndicator.h"
#include "absl/strings/escaping.h" // BytesToHexString
#include "absl/strings/match.h"
#include "ast/Helpers.h"
#include "ast/desugar/Desugar.h"
#include "ast/substitute/substitute.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "class_flatten/class_flatten.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/crypto_hashing/crypto_hashing.h"
#include "common/formatting.h"
#include "common/sort.h"
#include "core/ErrorQueue.h"
#include "core/NameSubstitution.h"
#include "core/Unfreeze.h"
#include "core/errors/parser.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "core/serialize/serialize.h"
#include "definition_validator/validator.h"
#include "infer/infer.h"
#include "local_vars/local_vars.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "pipeline.h"
#include "resolver/resolver.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::realmain::pipeline {

class CFGCollectorAndTyper {
    const options::Options &opts;

public:
    CFGCollectorAndTyper(const options::Options &opts) : opts(opts){};

    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &m = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        if (ctx.file.data(ctx).strictLevel < core::StrictLevel::True || m.symbol.data(ctx)->flags.isOverloaded ||
            (m.symbol.data(ctx)->flags.isAbstract && ctx.file.data(ctx).compiledLevel != core::CompiledLevel::True)) {
            return;
        }
        auto &print = opts.print;
        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m.symbol), m);

        if (opts.stopAfterPhase != options::Phase::CFG) {
            cfg = infer::Inference::run(ctx.withOwner(cfg->symbol), move(cfg));
            if (cfg) {
                for (auto &extension : ctx.state.semanticExtensions) {
                    extension->typecheck(ctx, ctx.file, *cfg, m);
                }
            }
        }
        if (print.CFG.enabled) {
            print.CFG.fmt("{}\n\n", cfg->toString(ctx));
        }
        if (print.CFGText.enabled) {
            print.CFG.fmt("{}\n\n", cfg->toTextualString(ctx));
        }
        if (print.CFGRaw.enabled) {
            print.CFGRaw.fmt("{}\n\n", cfg->showRaw(ctx));
        }
    }
};

string fileKey(const core::File &file) {
    auto path = file.path();
    string key(path.begin(), path.end());
    key += "//";
    auto hashBytes = sorbet::crypto_hashing::hash64(file.source());
    key += absl::BytesToHexString(string_view{(char *)hashBytes.data(), size(hashBytes)});
    return key;
}

ast::ExpressionPtr fetchTreeFromCache(core::GlobalState &gs, core::FileRef fref, core::File &file,
                                      const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    if (kvstore && fref.id() < gs.filesUsed()) {
        string fileHashKey = fileKey(file);
        auto maybeCached = kvstore->read(fileHashKey);
        if (maybeCached.data != nullptr) {
            prodCounterInc("types.input.files.kvstore.hit");
            return core::serialize::Serializer::loadTree(gs, file, maybeCached.data);
        } else {
            prodCounterInc("types.input.files.kvstore.miss");
        }
    }
    return nullptr;
}

unique_ptr<parser::Node> runParser(core::GlobalState &gs, core::FileRef file, const options::Printers &print,
                                   bool traceLexer, bool traceParser) {
    Timer timeit(gs.tracer(), "runParser", {{"file", string(file.data(gs).path())}});
    unique_ptr<parser::Node> nodes;
    {
        core::UnfreezeNameTable nameTableAccess(gs); // enters strings from source code as names
        auto indentationAware = false;               // Don't start in indentation-aware error recovery mode
        auto settings = parser::Parser::Settings{traceLexer, traceParser, indentationAware};
        nodes = parser::Parser::run(gs, file, settings);
    }
    if (print.ParseTree.enabled) {
        print.ParseTree.fmt("{}\n", nodes->toStringWithTabs(gs, 0));
    }
    if (print.ParseTreeJson.enabled) {
        print.ParseTreeJson.fmt("{}\n", nodes->toJSON(gs, 0));
    }
    if (print.ParseTreeJsonWithLocs.enabled) {
        print.ParseTreeJson.fmt("{}\n", nodes->toJSONWithLocs(gs, file, 0));
    }
    if (print.ParseTreeWhitequark.enabled) {
        print.ParseTreeWhitequark.fmt("{}\n", nodes->toWhitequark(gs, 0));
    }
    return nodes;
}

ast::ExpressionPtr runDesugar(core::GlobalState &gs, core::FileRef file, unique_ptr<parser::Node> parseTree,
                              const options::Printers &print) {
    Timer timeit(gs.tracer(), "runDesugar", {{"file", string(file.data(gs).path())}});
    ast::ExpressionPtr ast;
    core::MutableContext ctx(gs, core::Symbols::root(), file);
    {
        core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
        ast = ast::desugar::node2Tree(ctx, move(parseTree));
    }
    if (print.DesugarTree.enabled) {
        print.DesugarTree.fmt("{}\n", ast.toStringWithTabs(gs, 0));
    }
    if (print.DesugarTreeRaw.enabled) {
        print.DesugarTreeRaw.fmt("{}\n", ast.showRaw(gs));
    }
    return ast;
}

ast::ExpressionPtr runRewriter(core::GlobalState &gs, core::FileRef file, ast::ExpressionPtr ast) {
    core::MutableContext ctx(gs, core::Symbols::root(), file);
    Timer timeit(gs.tracer(), "runRewriter", {{"file", string(file.data(gs).path())}});
    core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
    return rewriter::Rewriter::run(ctx, move(ast));
}

ast::ParsedFile runLocalVars(core::GlobalState &gs, ast::ParsedFile tree) {
    Timer timeit(gs.tracer(), "runLocalVars", {{"file", string(tree.file.data(gs).path())}});
    core::MutableContext ctx(gs, core::Symbols::root(), tree.file);
    return sorbet::local_vars::LocalVars::run(ctx, move(tree));
}

ast::ParsedFile emptyParsedFile(core::FileRef file) {
    return {ast::MK::EmptyTree(), file};
}

ast::ParsedFile indexOne(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                         ast::ExpressionPtr tree) {
    auto &print = opts.print;
    ast::ParsedFile rewriten{nullptr, file};

    Timer timeit(lgs.tracer(), "indexOne", {{"file", string(file.data(lgs).path())}});
    try {
        if (!tree) {
            // tree isn't cached. Need to start from parser
            if (file.data(lgs).strictLevel == core::StrictLevel::Ignore) {
                return emptyParsedFile(file);
            }
            auto parseTree = runParser(lgs, file, print, opts.traceLexer, opts.traceParser);
            if (opts.stopAfterPhase == options::Phase::PARSER) {
                return emptyParsedFile(file);
            }
            tree = runDesugar(lgs, file, move(parseTree), print);
            if (opts.stopAfterPhase == options::Phase::DESUGARER) {
                return emptyParsedFile(file);
            }
            if (!opts.skipRewriterPasses) {
                tree = runRewriter(lgs, file, move(tree));
            }
            if (print.RewriterTree.enabled) {
                print.RewriterTree.fmt("{}\n", tree.toStringWithTabs(lgs, 0));
            }
            if (print.RewriterTreeRaw.enabled) {
                print.RewriterTreeRaw.fmt("{}\n", tree.showRaw(lgs));
            }
            tree = runLocalVars(lgs, ast::ParsedFile{move(tree), file}).tree;
            if (opts.stopAfterPhase == options::Phase::LOCAL_VARS) {
                return emptyParsedFile(file);
            }
        }
        if (print.IndexTree.enabled) {
            print.IndexTree.fmt("{}\n", tree.toStringWithTabs(lgs, 0));
        }
        if (print.IndexTreeRaw.enabled) {
            print.IndexTreeRaw.fmt("{}\n", tree.showRaw(lgs));
        }
        if (opts.stopAfterPhase == options::Phase::REWRITER) {
            return emptyParsedFile(file);
        }

        rewriten.tree = move(tree);
        return rewriten;
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = lgs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
            e.setHeader("Exception parsing file: `{}` (backtrace is above)", file.data(lgs).path());
        }
        return emptyParsedFile(file);
    }
}

vector<ast::ParsedFile>
incrementalResolve(core::GlobalState &gs, vector<ast::ParsedFile> what,
                   optional<UnorderedMap<core::FileRef, core::FoundMethodHashes>> &&foundMethodHashesForFiles,
                   const options::Options &opts) {
    try {
#ifndef SORBET_REALMAIN_MIN
        if (opts.stripePackages) {
            Timer timeit(gs.tracer(), "incremental_packager");
            what = packager::Packager::runIncremental(gs, move(what));
        }
#endif
        {
            Timer timeit(gs.tracer(), "incremental_naming");
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);
            auto emptyWorkers = WorkerPool::create(0, gs.tracer());

            auto result = foundMethodHashesForFiles.has_value()
                              ? sorbet::namer::Namer::runIncremental(
                                    gs, move(what), std::move(foundMethodHashesForFiles.value()), *emptyWorkers)
                              : sorbet::namer::Namer::run(gs, move(what), *emptyWorkers, nullptr);

            // Cancellation cannot occur during incremental namer.
            ENFORCE(result.hasResult());
            what = move(result.result());

            // Required for autogen tests, which need to control which phase to stop after.
            if (opts.stopAfterPhase == options::Phase::NAMER) {
                return what;
            }
        }

        {
            Timer timeit(gs.tracer(), "incremental_resolve");
            gs.tracer().trace("Resolving (incremental pass)...");
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);

            auto result = sorbet::resolver::Resolver::runIncremental(gs, move(what));
            // incrementalResolve is not cancelable.
            ENFORCE(result.hasResult());
            what = move(result.result());

            // Required for autogen tests, which need to control which phase to stop after.
            if (opts.stopAfterPhase == options::Phase::RESOLVER) {
                return what;
            }
        }

#ifndef SORBET_REALMAIN_MIN
        if (opts.stripePackages) {
            auto emptyWorkers = WorkerPool::create(0, gs.tracer());
            what = packager::VisibilityChecker::runIncremental(gs, *emptyWorkers, std::move(what));
        }
#endif

    } catch (SorbetException &) {
        if (auto e = gs.beginError(sorbet::core::Loc::none(), sorbet::core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }

    return what;
}

// TODO: Deduplicate implementation with incrementalResolve using is_const_v-ish tricks
vector<ast::ParsedFile> incrementalResolveBestEffort(const core::GlobalState &gs, vector<ast::ParsedFile> what,
                                                     const options::Options &opts) {
    try {
#ifndef SORBET_REALMAIN_MIN
        if (opts.stripePackages) {
            Timer timeit(gs.tracer(), "incremental_packager");
            what = packager::Packager::runIncrementalBestEffort(gs, move(what));
        }
#endif
        {
            Timer timeit(gs.tracer(), "incremental_naming");
            auto emptyWorkers = WorkerPool::create(0, gs.tracer());

            auto result = sorbet::namer::Namer::symbolizeTreesBestEffort(gs, move(what), *emptyWorkers);
            // Cancellation cannot occur during incremental namer.
            ENFORCE(result.hasResult());
            what = move(result.result());

            // Required for autogen tests, which need to control which phase to stop after.
            if (opts.stopAfterPhase == options::Phase::NAMER) {
                return what;
            }
        }

        {
            Timer timeit(gs.tracer(), "incremental_resolve");
            gs.tracer().trace("Resolving (incremental pass)...");

            auto result = sorbet::resolver::Resolver::runIncrementalBestEffort(gs, move(what));
            // incrementalResolve is not cancelable.
            ENFORCE(result.hasResult());
            what = move(result.result());

            // Required for autogen tests, which need to control which phase to stop after.
            if (opts.stopAfterPhase == options::Phase::RESOLVER) {
                return what;
            }
        }

#ifndef SORBET_REALMAIN_MIN
        if (opts.stripePackages) {
            auto emptyWorkers = WorkerPool::create(0, gs.tracer());
            what = packager::VisibilityChecker::runIncremental(gs, *emptyWorkers, move(what));
        }
#endif
    } catch (SorbetException &) {
        if (auto e = gs.beginError(sorbet::core::Loc::none(), sorbet::core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }

    return what;
}

vector<core::FileRef> reserveFiles(unique_ptr<core::GlobalState> &gs, const vector<string> &files) {
    Timer timeit(gs->tracer(), "reserveFiles");
    vector<core::FileRef> ret;
    ret.reserve(files.size());
    core::UnfreezeFileTable unfreezeFiles(*gs);
    for (auto &f : files) {
        auto fileRef = gs->findFileByPath(f);
        if (!fileRef.exists()) {
            fileRef = gs->reserveFileRef(f);
        }
        ret.emplace_back(move(fileRef));
    }
    return ret;
}

core::StrictLevel decideStrictLevel(const core::GlobalState &gs, const core::FileRef file,
                                    const options::Options &opts) {
    auto &fileData = file.data(gs);

    core::StrictLevel level;
    string filePath = string(fileData.path());
    // make sure all relative file paths start with ./
    if (!absl::StartsWith(filePath, "/") && !absl::StartsWith(filePath, "./")) {
        filePath.insert(0, "./");
    }

    if (fileData.originalSigil == core::StrictLevel::None) {
        level = core::StrictLevel::False;
    } else {
        level = fileData.originalSigil;
    }

    core::StrictLevel minStrict = opts.forceMinStrict;
    core::StrictLevel maxStrict = opts.forceMaxStrict;
    if (level <= core::StrictLevel::Max && level > core::StrictLevel::Ignore) {
        level = max(min(level, maxStrict), minStrict);
    }

    auto fnd = opts.strictnessOverrides.find(filePath);
    if (fnd != opts.strictnessOverrides.end()) {
        if (fnd->second == fileData.originalSigil && fnd->second > opts.forceMinStrict &&
            fnd->second < opts.forceMaxStrict) {
            if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Parser::ParserError)) {
                e.setHeader("Useless override of strictness level");
            }
        }
        level = fnd->second;
    }

    if (gs.runningUnderAutogen) {
        // Autogen stops before infer but needs to see all definitions
        level = core::StrictLevel::False;
    }

    return level;
}

void incrementStrictLevelCounter(core::StrictLevel level) {
    switch (level) {
        case core::StrictLevel::None:
            Exception::raise("Should never attempt to increment StrictLevel::None");
            break;
        case core::StrictLevel::Ignore:
            prodCounterInc("types.input.files.sigil.ignore");
            break;
        case core::StrictLevel::Internal:
            Exception::raise("Should never attempt to increment StrictLevel::Internal");
            break;
        case core::StrictLevel::False:
            prodCounterInc("types.input.files.sigil.false");
            break;
        case core::StrictLevel::True:
            prodCounterInc("types.input.files.sigil.true");
            break;
        case core::StrictLevel::Strict:
            prodCounterInc("types.input.files.sigil.strict");
            break;
        case core::StrictLevel::Strong:
            prodCounterInc("types.input.files.sigil.strong");
            break;
        case core::StrictLevel::Max:
            Exception::raise("Should never attempt to increment StrictLevel::Max");
            break;
        case core::StrictLevel::Autogenerated:
            prodCounterInc("types.input.files.sigil.autogenerated");
            break;
        case core::StrictLevel::Stdlib:
            prodCounterInc("types.input.files.sigil.stdlib");
            break;
    }
}

// Returns a non-null ast::Expression if kvstore contains the AST.
ast::ExpressionPtr readFileWithStrictnessOverrides(core::GlobalState &gs, core::FileRef file,
                                                   const options::Options &opts,
                                                   const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    ast::ExpressionPtr ast;
    if (file.dataAllowingUnsafe(gs).sourceType != core::File::Type::NotYetRead) {
        return ast;
    }
    auto fileName = file.dataAllowingUnsafe(gs).path();
    Timer timeit(gs.tracer(), "readFileWithStrictnessOverrides", {{"file", string(fileName)}});
    string src;
    bool fileFound = true;
    try {
        src = opts.fs->readFile(fileName);
    } catch (FileNotFoundException e) {
        // continue with an empty source, because the
        // assertion below requires every input file to map
        // to one output tree
        fileFound = false;
    }
    prodCounterAdd("types.input.bytes", src.size());
    prodCounterInc("types.input.files");

    {
        core::UnfreezeFileTable unfreezeFiles(gs);
        auto fileObj =
            make_shared<core::File>(string(fileName.begin(), fileName.end()), move(src), core::File::Type::Normal);
        // Returns nullptr if tree is not in cache.
        ast = fetchTreeFromCache(gs, file, *fileObj, kvstore);

        auto entered = gs.enterNewFileAt(move(fileObj), file);
        ENFORCE(entered == file);
    }
    if (enable_counters) {
        counterAdd("types.input.lines", file.data(gs).lineCount());
    }

    auto &fileData = file.data(gs);
    if (!fileFound) {
        if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::FileNotFound)) {
            e.setHeader("File Not Found");
        }
    }

    if (!opts.storeState.empty()) {
        fileData.sourceType = core::File::Type::PayloadGeneration;
    }

    auto level = decideStrictLevel(gs, file, opts);
    fileData.strictLevel = level;
    incrementStrictLevelCounter(level);
    return ast;
}

struct IndexResult {
    unique_ptr<core::GlobalState> gs;
    vector<ast::ParsedFile> trees;
};

struct IndexThreadResultPack {
    CounterState counters;
    IndexResult res;
};

struct IndexSubstitutionJob {
    // Not necessary for substitution, but passing this through to the worker means it's freed in that thread, instead
    // of serially in the main thread.
    unique_ptr<core::GlobalState> threadGs;

    std::optional<core::NameSubstitution> subst;
    vector<ast::ParsedFile> trees;

    IndexSubstitutionJob() {}

    IndexSubstitutionJob(core::GlobalState &to, IndexResult res)
        : threadGs{std::move(res.gs)}, subst{}, trees{std::move(res.trees)} {
        to.mergeFileTable(*this->threadGs);
        if (absl::c_any_of(this->trees, [this](auto &parsed) { return !parsed.file.data(*this->threadGs).cached(); })) {
            this->subst.emplace(*this->threadGs, to);
        }
    }

    IndexSubstitutionJob(IndexSubstitutionJob &&other) = default;
    IndexSubstitutionJob &operator=(IndexSubstitutionJob &&other) = default;
};

vector<ast::ParsedFile> mergeIndexResults(core::GlobalState &cgs, const options::Options &opts,
                                          shared_ptr<BlockingBoundedQueue<IndexThreadResultPack>> input,
                                          WorkerPool &workers, const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    ProgressIndicator progress(opts.showProgress, "Indexing", input->bound);
    Timer timeit(cgs.tracer(), "mergeIndexResults");

    auto batchq = make_shared<ConcurrentBoundedQueue<IndexSubstitutionJob>>(input->bound);
    vector<ast::ParsedFile> ret;
    size_t totalNumTrees = 0;

    {
        Timer timeit(cgs.tracer(), "mergeGlobalStates");
        IndexThreadResultPack threadResult;
        for (auto result = input->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), cgs.tracer());
             !result.done(); result = input->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), cgs.tracer())) {
            if (result.gotItem()) {
                counterConsume(move(threadResult.counters));
                auto numTrees = threadResult.res.trees.size();
                batchq->push(IndexSubstitutionJob{cgs, std::move(threadResult.res)}, numTrees);
                totalNumTrees += numTrees;
            }
        }
    }

    {
        Timer timeit(cgs.tracer(), "substituteTrees");
        auto resultq = make_shared<BlockingBoundedQueue<vector<ast::ParsedFile>>>(batchq->bound);

        workers.multiplexJob("substituteTrees", [&cgs, batchq, resultq]() {
            Timer timeit(cgs.tracer(), "substituteTreesWorker");
            IndexSubstitutionJob job;
            for (auto result = batchq->try_pop(job); !result.done(); result = batchq->try_pop(job)) {
                if (result.gotItem()) {
                    if (job.subst.has_value()) {
                        for (auto &tree : job.trees) {
                            auto file = tree.file;
                            if (!file.data(cgs).cached()) {
                                core::MutableContext ctx(cgs, core::Symbols::root(), file);
                                tree = ast::Substitute::run(ctx, *job.subst, move(tree));
                            }
                        }
                    }
                    auto numSubstitutedTrees = job.trees.size();
                    resultq->push(std::move(job.trees), numSubstitutedTrees);
                }
            }
        });

        ret.reserve(totalNumTrees);
        vector<ast::ParsedFile> trees;
        for (auto result = resultq->wait_pop_timed(trees, WorkerPool::BLOCK_INTERVAL(), cgs.tracer()); !result.done();
             result = resultq->wait_pop_timed(trees, WorkerPool::BLOCK_INTERVAL(), cgs.tracer())) {
            if (result.gotItem()) {
                ret.insert(ret.end(), std::make_move_iterator(trees.begin()), std::make_move_iterator(trees.end()));
                progress.reportProgress(resultq->doneEstimate());
            }
        }
    }

    return ret;
}

vector<ast::ParsedFile> indexSuppliedFiles(core::GlobalState &baseGs, vector<core::FileRef> &files,
                                           const options::Options &opts, WorkerPool &workers,
                                           const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    Timer timeit(baseGs.tracer(), "indexSuppliedFiles");
    auto resultq = make_shared<BlockingBoundedQueue<IndexThreadResultPack>>(files.size());
    auto fileq = make_shared<ConcurrentBoundedQueue<core::FileRef>>(files.size());
    for (auto &file : files) {
        fileq->push(move(file), 1);
    }

    std::shared_ptr<const core::GlobalState> emptyGs = baseGs.copyForIndex();

    workers.multiplexJob("indexSuppliedFiles", [emptyGs, &opts, fileq, resultq, &kvstore]() {
        Timer timeit(emptyGs->tracer(), "indexSuppliedFilesWorker");

        // clone the empty global state to avoid manually re-entering everything, and copy the base filetable so that
        // file sources are available.
        unique_ptr<core::GlobalState> localGs = emptyGs->deepCopy();

        IndexThreadResultPack threadResult;

        {
            core::FileRef job;
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    core::FileRef file = job;
                    auto cachedTree = readFileWithStrictnessOverrides(*localGs, file, opts, kvstore);
                    auto parsedFile = indexOne(opts, *localGs, file, move(cachedTree));
                    threadResult.res.trees.emplace_back(move(parsedFile));
                }
            }
        }

        if (!threadResult.res.trees.empty()) {
            threadResult.counters = getAndClearThreadCounters();
            threadResult.res.gs = move(localGs);
            auto computedTreesCount = threadResult.res.trees.size();
            resultq->push(move(threadResult), computedTreesCount);
        }
    });

    return mergeIndexResults(baseGs, opts, resultq, workers, kvstore);
}

vector<ast::ParsedFile> index(core::GlobalState &gs, vector<core::FileRef> files, const options::Options &opts,
                              WorkerPool &workers, const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    Timer timeit(gs.tracer(), "index");
    vector<ast::ParsedFile> ret;
    vector<ast::ParsedFile> empty;

    if (opts.stopAfterPhase == options::Phase::INIT) {
        return empty;
    }

    gs.sanityCheck();

    if (files.size() < 3) {
        // Run singlethreaded if only using 2 files
        for (auto file : files) {
            auto tree = readFileWithStrictnessOverrides(gs, file, opts, kvstore);
            auto parsedFile = indexOne(opts, gs, file, move(tree));
            ret.emplace_back(move(parsedFile));
        }
        ENFORCE(files.size() == ret.size());
    } else {
        ret = indexSuppliedFiles(gs, files, opts, workers, kvstore);
    }

    fast_sort(ret, [](ast::ParsedFile const &a, ast::ParsedFile const &b) { return a.file < b.file; });
    return ret;
}

namespace {
void typecheckOne(core::Context ctx, ast::ParsedFile resolved, const options::Options &opts,
                  bool intentionallyLeakASTs) {
    core::FileRef f = resolved.file;

    if (opts.stopAfterPhase == options::Phase::NAMER) {
        if (intentionallyLeakASTs) {
            intentionallyLeakMemory(resolved.tree.release());
        }
        return;
    }

    resolved = class_flatten::runOne(ctx, move(resolved));

    resolved = definition_validator::runOne(ctx, std::move(resolved));

    if (opts.print.FlattenTree.enabled || opts.print.AST.enabled) {
        opts.print.FlattenTree.fmt("{}\n", resolved.tree.toString(ctx));
    }
    if (opts.print.FlattenTreeRaw.enabled || opts.print.ASTRaw.enabled) {
        opts.print.FlattenTreeRaw.fmt("{}\n", resolved.tree.showRaw(ctx));
    }

    if (opts.stopAfterPhase == options::Phase::RESOLVER) {
        if (intentionallyLeakASTs) {
            intentionallyLeakMemory(resolved.tree.release());
        }
        return;
    }
    if (f.data(ctx).isRBI()) {
        if (intentionallyLeakASTs) {
            intentionallyLeakMemory(resolved.tree.release());
        }
        return;
    }

    Timer timeit(ctx.state.tracer(), "typecheckOne", {{"file", string(f.data(ctx).path())}});
    try {
        if (opts.print.CFG.enabled) {
            opts.print.CFG.fmt("digraph \"{}\" {{\n", FileOps::getFileName(f.data(ctx).path()));
        }
        if (opts.print.CFGRaw.enabled) {
            opts.print.CFGRaw.fmt("digraph \"{}\" {{\n", FileOps::getFileName(f.data(ctx).path()));
            opts.print.CFGRaw.fmt("  graph [fontname = \"Courier\"];\n");
            opts.print.CFGRaw.fmt("  node [fontname = \"Courier\"];\n");
            opts.print.CFGRaw.fmt("  edge [fontname = \"Courier\"];\n");
        }
        CFGCollectorAndTyper collector(opts);
        {
            ast::ShallowWalk::apply(ctx, collector, resolved.tree);
            for (auto &extension : ctx.state.semanticExtensions) {
                extension->finishTypecheckFile(ctx, f);
            }
        }
        if (opts.print.CFG.enabled) {
            opts.print.CFG.fmt("}}\n\n");
        }
        if (opts.print.CFGRaw.enabled) {
            opts.print.CFGRaw.fmt("}}\n\n");
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = ctx.state.beginError(sorbet::core::Loc::none(f), core::errors::Internal::InternalError)) {
            e.setHeader("Exception in cfg+infer: {} (backtrace is above)", f.data(ctx).path());
        }
    }

    if (intentionallyLeakASTs) {
        intentionallyLeakMemory(resolved.tree.release());
    }
    return;
}
} // namespace

vector<ast::ParsedFile> package(core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
                                WorkerPool &workers) {
#ifndef SORBET_REALMAIN_MIN
    if (opts.stripePackages) {
        Timer timeit(gs.tracer(), "package");
        {
            core::UnfreezeNameTable unfreezeToEnterPackagerOptionsGS(gs);
            core::packages::UnfreezePackages unfreezeToEnterPackagerOptionsPackageDB = gs.unfreezePackages();
            gs.setPackagerOptions(opts.secondaryTestPackageNamespaces,
                                  opts.extraPackageFilesDirectoryUnderscorePrefixes,
                                  opts.extraPackageFilesDirectorySlashPrefixes,
                                  opts.packageSkipRBIExportEnforcementDirs, opts.stripePackagesHint);
        }
        what = packager::Packager::run(gs, workers, move(what));
        if (opts.print.Packager.enabled) {
            for (auto &f : what) {
                opts.print.Packager.fmt("# -- {} --\n", f.file.data(gs).path());
                opts.print.Packager.fmt("{}\n", f.tree.toStringWithTabs(gs, 0));
            }
        }
    }
#endif
    return what;
}

ast::ParsedFilesOrCancelled nameBestEffortConst(const core::GlobalState &gs, vector<ast::ParsedFile> what,
                                                WorkerPool &workers) {
    Timer timeit(gs.tracer(), "nameBestEffortConst");
    auto result = namer::Namer::symbolizeTreesBestEffort(gs, move(what), workers);

    return result;
}

ast::ParsedFilesOrCancelled name(core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
                                 WorkerPool &workers, core::FoundMethodHashes *foundMethodHashes) {
    Timer timeit(gs.tracer(), "name");
    core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
    core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
    auto result = namer::Namer::run(gs, move(what), workers, foundMethodHashes);

    return result;
}
class GatherUnresolvedConstantsWalk {
public:
    vector<string> unresolvedConstants;
    void postTransformConstantLit(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto unresolvedPath = ast::cast_tree_nonnull<ast::ConstantLit>(tree).fullUnresolvedPath(ctx);
        if (unresolvedPath.has_value()) {
            unresolvedConstants.emplace_back(fmt::format(
                "{}::{}", unresolvedPath->first != core::Symbols::root() ? unresolvedPath->first.show(ctx) : "",
                fmt::map_join(unresolvedPath->second, "::", [&](const auto &el) -> string { return el.show(ctx); })));
        }
    }
};

vector<ast::ParsedFile> printMissingConstants(core::GlobalState &gs, const options::Options &opts,
                                              vector<ast::ParsedFile> what) {
    Timer timeit(gs.tracer(), "printMissingConstants");
    GatherUnresolvedConstantsWalk walk;
    for (auto &resolved : what) {
        core::MutableContext ctx(gs, core::Symbols::root(), resolved.file);
        ast::TreeWalk::apply(ctx, walk, resolved.tree);
    }
    auto &missing = walk.unresolvedConstants;
    fast_sort(missing);
    missing.erase(unique(missing.begin(), missing.end()), missing.end());

    opts.print.MissingConstants.fmt("{}\n", fmt::join(missing, "\n"));
    return what;
}

class DefinitionLinesDenylistEnforcer {
private:
    const core::FileRef file;
    const int prohibitedLinesStart;
    const int prohibitedLinesEnd;

    bool isAllowListed(core::Context ctx, core::SymbolRef sym) {
        return sym.name(ctx) == core::Names::staticInit() || sym.name(ctx) == core::Names::Constants::Root() ||
               sym.name(ctx) == core::Names::unresolvedAncestors();
    }

    void checkLoc(core::Context ctx, core::Loc loc) {
        auto detailStart = core::Loc::offset2Pos(file.data(ctx), loc.beginPos());
        auto detailEnd = core::Loc::offset2Pos(file.data(ctx), loc.endPos());
        ENFORCE(!(detailStart.line >= prohibitedLinesStart && detailEnd.line <= prohibitedLinesEnd));
    }

    void checkSym(core::Context ctx, core::SymbolRef sym) {
        if (isAllowListed(ctx, sym)) {
            return;
        }
        checkLoc(ctx, sym.loc(ctx));
    }

public:
    DefinitionLinesDenylistEnforcer(core::FileRef file, int prohibitedLinesStart, int prohibitedLinesEnd)
        : file(file), prohibitedLinesStart(prohibitedLinesStart), prohibitedLinesEnd(prohibitedLinesEnd) {
        // Can be equal if file was empty.
        ENFORCE(prohibitedLinesStart <= prohibitedLinesEnd);
        ENFORCE(file.exists());
    };

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        checkSym(ctx, ast::cast_tree_nonnull<ast::ClassDef>(tree).symbol);
    }
    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        checkSym(ctx, ast::cast_tree_nonnull<ast::MethodDef>(tree).symbol);
    }
};

ast::ParsedFile checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, ast::ParsedFile what,
                                                        int prohibitedLinesStart, int prohibitedLinesEnd) {
    DefinitionLinesDenylistEnforcer enforcer(what.file, prohibitedLinesStart, prohibitedLinesEnd);
    ast::TreeWalk::apply(core::Context(gs, core::Symbols::root(), what.file), enforcer, what.tree);
    return what;
}

ast::ParsedFilesOrCancelled resolve(unique_ptr<core::GlobalState> &gs, vector<ast::ParsedFile> what,
                                    const options::Options &opts, WorkerPool &workers,
                                    core::FoundMethodHashes *foundMethodHashes) {
    try {
        // packager intentionally runs outside of rewriter so that its output does not get cached.
        what = package(*gs, move(what), opts, workers);

        auto result = name(*gs, move(what), opts, workers, foundMethodHashes);
        if (!result.hasResult()) {
            return result;
        }
        what = move(result.result());

        for (auto &named : what) {
            if (opts.print.NameTree.enabled) {
                opts.print.NameTree.fmt("{}\n", named.tree.toStringWithTabs(*gs, 0));
            }
            if (opts.print.NameTreeRaw.enabled) {
                opts.print.NameTreeRaw.fmt("{}\n", named.tree.showRaw(*gs));
            }
        }

        if (opts.stopAfterPhase != options::Phase::NAMER) {
            ProgressIndicator namingProgress(opts.showProgress, "Resolving", 1);
            {
                Timer timeit(gs->tracer(), "resolving");
                core::UnfreezeNameTable nameTableAccess(*gs);     // Resolver::defineAttr
                core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters stubs
                auto maybeResult = resolver::Resolver::run(*gs, move(what), workers);
                if (!maybeResult.hasResult()) {
                    return maybeResult;
                }
                what = move(maybeResult.result());
            }

#ifndef SORBET_REALMAIN_MIN
            if (opts.stripePackages) {
                Timer timeit(gs->tracer(), "visibility_checker");
                what = packager::VisibilityChecker::run(*gs, workers, std::move(what));
            }
#endif

            if (opts.stressIncrementalResolver) {
                auto symbolsBefore = gs->symbolsUsedTotal();
                for (auto &f : what) {
                    // Shift contents of file past current file's EOF, re-run incrementalResolve, assert that no
                    // locations appear before file's old EOF.
                    const int prohibitedLines = f.file.data(*gs).source().size();
                    auto newSource = fmt::format("{}\n{}", string(prohibitedLines, '\n'), f.file.data(*gs).source());
                    auto newFile = make_shared<core::File>(string(f.file.data(*gs).path()), move(newSource),
                                                           f.file.data(*gs).sourceType);
                    gs->replaceFile(f.file, move(newFile));
                    f.file.data(*gs).strictLevel = decideStrictLevel(*gs, f.file, opts);
                    auto reIndexed = indexOne(opts, *gs, f.file);
                    vector<ast::ParsedFile> toBeReResolved;
                    toBeReResolved.emplace_back(move(reIndexed));
                    // We don't compute file hashes when running for incrementalResolve.
                    auto foundMethodHashesForFiles = nullopt;
                    auto reresolved =
                        pipeline::incrementalResolve(*gs, move(toBeReResolved), foundMethodHashesForFiles, opts);
                    ENFORCE(reresolved.size() == 1);
                    f = checkNoDefinitionsInsideProhibitedLines(*gs, move(reresolved[0]), 0, prohibitedLines);
                }
                ENFORCE(symbolsBefore == gs->symbolsUsedTotal(),
                        "Stressing the incremental resolver should not add any new symbols");
            }
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs->beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }

    if (opts.print.ResolveTree.enabled || opts.print.ResolveTreeRaw.enabled) {
        for (auto &resolved : what) {
            if (opts.print.ResolveTree.enabled) {
                opts.print.ResolveTree.fmt("{}\n", resolved.tree.toString(*gs));
            }
            if (opts.print.ResolveTreeRaw.enabled) {
                opts.print.ResolveTreeRaw.fmt("{}\n", resolved.tree.showRaw(*gs));
            }
        }
    }

    if (opts.print.SymbolTable.enabled) {
        opts.print.SymbolTable.fmt("{}\n", gs->toString());
    }
    if (opts.print.SymbolTableRaw.enabled) {
        opts.print.SymbolTableRaw.fmt("{}\n", gs->showRaw());
    }

#ifndef SORBET_REALMAIN_MIN
    if (opts.print.SymbolTableJson.enabled) {
        auto root = core::Proto::toProto(*gs, core::Symbols::root(), false);
        if (opts.print.SymbolTableJson.outputPath.empty()) {
            core::Proto::toJSON(root, cout);
        } else {
            stringstream buf;
            core::Proto::toJSON(root, buf);
            opts.print.SymbolTableJson.print(buf.str());
        }
    }
    if (opts.print.SymbolTableProto.enabled) {
        auto root = core::Proto::toProto(*gs, core::Symbols::root(), false);
        if (opts.print.SymbolTableProto.outputPath.empty()) {
            root.SerializeToOstream(&cout);
        } else {
            string buf;
            root.SerializeToString(&buf);
            opts.print.SymbolTableProto.print(buf);
        }
    }
    if (opts.print.SymbolTableMessagePack.enabled) {
        auto root = core::Proto::toProto(*gs, core::Symbols::root(), false);
        stringstream buf;
        core::Proto::toJSON(root, buf);
        auto str = buf.str();
        rapidjson::Document document;
        document.Parse(str);
        mpack_writer_t writer;
        if (opts.print.SymbolTableMessagePack.outputPath.empty()) {
            mpack_writer_init_stdfile(&writer, stdout, /* close when done */ false);
        } else {
            mpack_writer_init_filename(&writer, opts.print.SymbolTableMessagePack.outputPath.c_str());
        }
        json2msgpack::json2msgpack(document, &writer);
        if (mpack_writer_destroy(&writer)) {
            Exception::raise("failed to write msgpack");
        }
    }
    if (opts.print.SymbolTableFullJson.enabled) {
        auto root = core::Proto::toProto(*gs, core::Symbols::root(), true);
        if (opts.print.SymbolTableJson.outputPath.empty()) {
            core::Proto::toJSON(root, cout);
        } else {
            stringstream buf;
            core::Proto::toJSON(root, buf);
            opts.print.SymbolTableJson.print(buf.str());
        }
    }
    if (opts.print.SymbolTableFullProto.enabled) {
        auto root = core::Proto::toProto(*gs, core::Symbols::root(), true);
        if (opts.print.SymbolTableFullProto.outputPath.empty()) {
            root.SerializeToOstream(&cout);
        } else {
            string buf;
            root.SerializeToString(&buf);
            opts.print.SymbolTableFullProto.print(buf);
        }
    }
    if (opts.print.SymbolTableFullMessagePack.enabled) {
        auto root = core::Proto::toProto(*gs, core::Symbols::root(), true);
        stringstream buf;
        core::Proto::toJSON(root, buf);
        auto str = buf.str();
        rapidjson::Document document;
        document.Parse(str);
        mpack_writer_t writer;
        if (opts.print.SymbolTableFullMessagePack.outputPath.empty()) {
            mpack_writer_init_stdfile(&writer, stdout, /* close when done */ false);
        } else {
            mpack_writer_init_filename(&writer, opts.print.SymbolTableFullMessagePack.outputPath.c_str());
        }
        json2msgpack::json2msgpack(document, &writer);
        if (mpack_writer_destroy(&writer)) {
            Exception::raise("failed to write msgpack");
        }
    }
#endif
    if (opts.print.SymbolTableFull.enabled) {
        opts.print.SymbolTableFull.fmt("{}\n", gs->toStringFull());
    }
    if (opts.print.SymbolTableFullRaw.enabled) {
        opts.print.SymbolTableFullRaw.fmt("{}\n", gs->showRawFull());
    }

#ifndef SORBET_REALMAIN_MIN
    if (opts.print.FileTableProto.enabled || opts.print.FileTableFullProto.enabled) {
        if (opts.print.FileTableProto.enabled && opts.print.FileTableFullProto.enabled) {
            Exception::raise("file-table-proto and file-table-full-proto are mutually exclusive print options");
        }
        auto files = core::Proto::filesToProto(*gs, opts.print.FileTableFullProto.enabled);
        if (opts.print.FileTableProto.outputPath.empty()) {
            files.SerializeToOstream(&cout);
        } else {
            string buf;
            files.SerializeToString(&buf);
            opts.print.FileTableProto.print(buf);
        }
    }
    if (opts.print.FileTableJson.enabled || opts.print.FileTableFullJson.enabled) {
        if (opts.print.FileTableJson.enabled && opts.print.FileTableFullJson.enabled) {
            Exception::raise("file-table-json and file-table-full-json are mutually exclusive print options");
        }
        auto files = core::Proto::filesToProto(*gs, opts.print.FileTableFullJson.enabled);
        if (opts.print.FileTableJson.outputPath.empty()) {
            core::Proto::toJSON(files, cout);
        } else {
            stringstream buf;
            core::Proto::toJSON(files, buf);
            opts.print.FileTableJson.print(buf.str());
        }
    }
    if (opts.print.FileTableMessagePack.enabled || opts.print.FileTableFullMessagePack.enabled) {
        if (opts.print.FileTableMessagePack.enabled && opts.print.FileTableFullMessagePack.enabled) {
            Exception::raise("file-table-msgpack and file-table-full-msgpack are mutually exclusive print options");
        }
        auto files = core::Proto::filesToProto(*gs, opts.print.FileTableFullMessagePack.enabled);
        stringstream buf;
        core::Proto::toJSON(files, buf);
        auto str = buf.str();
        rapidjson::Document document;
        document.Parse(str);
        mpack_writer_t writer;
        if (opts.print.FileTableMessagePack.outputPath.empty()) {
            mpack_writer_init_stdfile(&writer, stdout, /* close when done */ false);
        } else {
            mpack_writer_init_filename(&writer, opts.print.FileTableMessagePack.outputPath.c_str());
        }
        json2msgpack::json2msgpack(document, &writer);
        if (mpack_writer_destroy(&writer)) {
            Exception::raise("failed to write msgpack");
        }
    }
#endif

    if (opts.print.MissingConstants.enabled) {
        what = printMissingConstants(*gs, opts, move(what));
    }

    return ast::ParsedFilesOrCancelled(move(what));
}

void typecheck(const core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
               WorkerPool &workers, bool cancelable,
               optional<shared_ptr<core::lsp::PreemptionTaskManager>> preemptionManager, bool presorted,
               bool intentionallyLeakASTs) {
    // Unless the error queue had a critical error, only typecheck should flush errors to the client, otherwise we will
    // drop errors in LSP mode.
    ENFORCE(gs.hadCriticalError() || gs.errorQueue->filesFlushedCount == 0);

    const auto &epochManager = *gs.epochManager;
    // Record epoch at start of typechecking before any preemption occurs.
    const uint32_t epoch = epochManager.getStatus().epoch;

    {
        Timer timeit(gs.tracer(), "typecheck");
        if (preemptionManager) {
            // Before kicking off typechecking, check if we need to preempt.
            (*preemptionManager)->tryRunScheduledPreemptionTask(gs);
        }

        shared_ptr<ConcurrentBoundedQueue<ast::ParsedFile>> fileq;
        shared_ptr<BlockingBoundedQueue<vector<core::FileRef>>> outputq;

        {
            fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(what.size());
            outputq = make_shared<BlockingBoundedQueue<vector<core::FileRef>>>(what.size());
        }

        if (!presorted) {
            // If files are not already sorted, we want to start typeckecking big files first because it helps with
            // better work distribution
            fast_sort(what, [&](const auto &lhs, const auto &rhs) -> bool {
                return lhs.file.data(gs).source().size() > rhs.file.data(gs).source().size();
            });
        }

        for (auto &resolved : what) {
            fileq->push(move(resolved), 1);
        }

        {
            ProgressIndicator cfgInferProgress(opts.showProgress, "CFG+Inference", what.size());
            workers.multiplexJob("typecheck", [&gs, &opts, epoch, &epochManager, &preemptionManager, fileq, outputq,
                                               cancelable, intentionallyLeakASTs]() {
                vector<core::FileRef> processedFiles;
                ast::ParsedFile job;
                int processedByThread = 0;

                {
                    for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                        if (result.gotItem()) {
                            unique_ptr<absl::ReaderMutexLock> lock;
                            if (preemptionManager) {
                                // [IDE] While held, no preemption tasks can run. Auto-released after each turn of
                                // the loop. Does not starve writers (tryRunScheduledPreemptionTask)
                                // because this call can block once tryRunScheduledPreemptionTask tries to acquire
                                // a (writer) lock.
                                lock = (*preemptionManager)->lockPreemption();
                            }
                            processedByThread++;
                            // [IDE] Only do the work if typechecking hasn't been canceled.
                            const bool isCanceled = cancelable && epochManager.wasTypecheckingCanceled();
                            // [IDE] Also, don't do work if the file has changed under us since we began
                            // typechecking!
                            // TODO(jvilk): epoch is unlikely to overflow, but it is theoretically possible.
                            const bool fileWasChanged = preemptionManager && job.file.data(gs).epoch > epoch;
                            if (!isCanceled && !fileWasChanged) {
                                core::FileRef file = job.file;
                                try {
                                    core::Context ctx(gs, core::Symbols::root(), file);
                                    auto file = job.file;
                                    typecheckOne(ctx, move(job), opts, intentionallyLeakASTs);
                                } catch (SorbetException &) {
                                    Exception::failInFuzzer();
                                    gs.tracer().error("Exception typing file: {} (backtrace is above)",
                                                      file.data(gs).path());
                                }
                                // Stream out errors
                                processedFiles.emplace_back(file);
                                outputq->push(move(processedFiles), processedByThread);
                                processedByThread = 0;
                            }
                        }
                    }
                }
                if (processedByThread > 0) {
                    outputq->push(move(processedFiles), processedByThread);
                }
            });

            vector<core::FileRef> files;
            {
                for (auto result = outputq->wait_pop_timed(files, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
                     !result.done();
                     result = outputq->wait_pop_timed(files, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
                    if (result.gotItem()) {
                        for (auto &file : files) {
                            gs.errorQueue->flushErrorsForFile(gs, file);
                        }
                    }
                    cfgInferProgress.reportProgress(fileq->doneEstimate());

                    if (preemptionManager) {
                        (*preemptionManager)->tryRunScheduledPreemptionTask(gs);
                    }
                }
                if (cancelable && epochManager.wasTypecheckingCanceled()) {
                    return;
                }
            }

            if (workers.size() > 0) {
                auto counterq = make_shared<BlockingBoundedQueue<CounterState>>(workers.size());
                workers.multiplexJob("collectCounters",
                                     [counterq]() { counterq->push(getAndClearThreadCounters(), 1); });
                {
                    sorbet::CounterState counters;
                    for (auto result = counterq->try_pop(counters); !result.done();
                         result = counterq->try_pop(counters)) {
                        if (result.gotItem()) {
                            counterConsume(move(counters));
                        }
                    }
                }
            }
        }
        for (auto &extension : gs.semanticExtensions) {
            extension->finishTypecheck(gs);
        }

        // Error queue is re-used across runs, so reset the flush count to ignore files flushed during typecheck.
        gs.errorQueue->filesFlushedCount = 0;

        return;
    }
}

bool cacheTreesAndFiles(const core::GlobalState &gs, WorkerPool &workers, vector<ast::ParsedFile> &parsedFiles,
                        const unique_ptr<OwnedKeyValueStore> &kvstore) {
    if (kvstore == nullptr) {
        return false;
    }

    Timer timeit(gs.tracer(), "pipeline::cacheTreesAndFiles");

    // Compress files in parallel.
    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile *>>(parsedFiles.size());
    for (auto &parsedFile : parsedFiles) {
        fileq->push(&parsedFile, 1);
    }

    auto resultq = make_shared<BlockingBoundedQueue<vector<pair<string, vector<uint8_t>>>>>(parsedFiles.size());
    workers.multiplexJob("compressTreesAndFiles", [fileq, resultq, &gs]() {
        vector<pair<string, vector<uint8_t>>> threadResult;
        int processedByThread = 0;
        ast::ParsedFile *job = nullptr;
        unique_ptr<Timer> timeit;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;
                    if (timeit == nullptr) {
                        timeit = make_unique<Timer>(gs.tracer(), "cacheTreesAndFilesWorker");
                    }

                    if (!job->file.exists()) {
                        continue;
                    }

                    auto &file = job->file.data(gs);
                    if (!file.cached() && !file.hasParseErrors()) {
                        threadResult.emplace_back(fileKey(file), core::serialize::Serializer::storeTree(file, *job));
                        // Stream out compressed files so that writes happen in parallel with processing.
                        if (processedByThread > 100) {
                            resultq->push(move(threadResult), processedByThread);
                            processedByThread = 0;
                        }
                    }
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    bool written = false;
    {
        vector<pair<string, vector<uint8_t>>> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                for (auto &a : threadResult) {
                    kvstore->write(move(a.first), move(a.second));
                    written = true;
                }
            }
        }
    }
    return written;
}

vector<ast::ParsedFile> autogenWriteCacheFile(const core::GlobalState &gs, string_view cachePath,
                                              vector<ast::ParsedFile> what, WorkerPool &workers) {
#ifndef SORBET_REALMAIN_MIN
    Timer timeit(gs.tracer(), "autogenWriteCacheFile");

    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(what.size());
    auto resultq = make_shared<BlockingBoundedQueue<autogen::HashedParsedFile>>(what.size());
    for (auto &pf : what) {
        fileq->push(move(pf), 1);
    }

    workers.multiplexJob("computeConstantCache", [&]() {
        ast::ParsedFile job;
        for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
            resultq->push(autogen::constantHashTree(gs, move(job)), 1);
        }
    });

    autogen::AutogenCache cache;
    vector<ast::ParsedFile> results;

    {
        autogen::HashedParsedFile output;
        for (auto result = resultq->wait_pop_timed(output, WorkerPool::BLOCK_INTERVAL(), gs.tracer()); !result.done();
             result = resultq->wait_pop_timed(output, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (!result.gotItem()) {
                continue;
            }
            cache.add(string(output.pf.file.data(gs).path()), output.constantHash);
            results.emplace_back(move(output.pf));
        }
    }

    FileOps::write(cachePath, cache.pack());

    return results;
#else
    return what;
#endif
}

} // namespace sorbet::realmain::pipeline
