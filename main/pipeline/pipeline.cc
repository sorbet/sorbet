// has to go first, as it violates poisons
#include "core/proto/proto.h"

#include "ProgressIndicator.h"
#include "absl/strings/escaping.h" // BytesToHexString
#include "ast/desugar/Desugar.h"
#include "ast/substitute/substitute.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "common/Timer.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/crypto_hashing/crypto_hashing.h"
#include "core/Unfreeze.h"
#include "core/errors/parser.h"
#include "core/serialize/serialize.h"
#include "dsl/dsl.h"
#include "infer/infer.h"
#include "namer/configatron/configatron.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "pipeline.h"
#include "resolver/resolver.h"

using namespace std;

namespace sorbet::realmain::pipeline {

bool wantTypedSource(const options::Options &opts, core::Context ctx, core::FileRef file) {
    if (opts.print.TypedSource) {
        return true;
    }
    if (opts.typedSource.empty()) {
        return false;
    }
    return file.data(ctx).path().find(opts.typedSource) != string::npos;
}

class CFG_Collector_and_Typer {
    const options::Options &opts;

public:
    CFG_Collector_and_Typer(const options::Options &opts) : opts(opts){};

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> m) {
        if (m->loc.file().data(ctx).strictLevel < core::StrictLevel::Typed || m->symbol.data(ctx)->isOverloaded()) {
            return m;
        }
        auto &print = opts.print;
        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);

        bool printSrc = wantTypedSource(opts, ctx, m->loc.file());

        if (print.CFGRaw || printSrc) {
            cfg = cfg::CFGBuilder::addDebugEnvironment(ctx.withOwner(m->symbol), move(cfg));
        }
        if (opts.stopAfterPhase == options::Phase::CFG) {
            return m;
        }
        cfg = infer::Inference::run(ctx.withOwner(m->symbol), move(cfg));
        if (print.CFG || print.CFGRaw) {
            fmt::print("{}\n\n", cfg->toString(ctx));
        }
        if (printSrc) {
            cfg->recordAnnotations(ctx);
        }
        return m;
    }
};

struct thread_result {
    unique_ptr<core::GlobalState> gs;
    CounterState counters;
    vector<ast::ParsedFile> trees;
};

string fileKey(core::GlobalState &gs, core::FileRef file) {
    auto path = file.data(gs).path();
    string key(path.begin(), path.end());
    key += "//";
    auto hashBytes = sorbet::crypto_hashing::hash64(file.data(gs).source());
    key += absl::BytesToHexString(string_view{(char *)hashBytes.data(), size(hashBytes)});
    return key;
}

ast::ParsedFile indexOne(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                         unique_ptr<KeyValueStore> &kvstore, shared_ptr<spdlog::logger> logger) {
    auto &print = opts.print;
    ast::ParsedFile dslsInlined{nullptr, file};

    try {
        if (kvstore && file.id() < lgs.filesUsed()) {
            string fileHashKey = fileKey(lgs, file);
            auto maybeCached = kvstore->read(fileHashKey);
            if (maybeCached) {
                logger->trace("Reading from cache: {}", file.data(lgs).path());
                auto t = core::serialize::Serializer::loadExpression(lgs, maybeCached, file.id());
                file.data(lgs).cachedParseTree = true;
                ENFORCE(t->loc.file() == file);
                dslsInlined.tree = move(t);
            }
        }
        if (!dslsInlined.tree) {
            // tree isn't cached. Need to start from parser

            if (file.data(lgs).strictLevel == core::StrictLevel::Ignore) {
                return {make_unique<ast::EmptyTree>(), file};
            }

            unique_ptr<parser::Node> nodes;
            {
                logger->trace("Parsing: {}", file.data(lgs).path());
                core::ErrorRegion errs(lgs, file);
                core::UnfreezeNameTable nameTableAccess(lgs); // enters strings from source code as names
                nodes = parser::Parser::run(lgs, file);
            }
            if (print.ParseTree) {
                fmt::print("{}\n", nodes->toStringWithTabs(lgs, 0));
            }
            if (print.ParseTreeJSON) {
                fmt::print("{}\n", nodes->toJSON(lgs, 0));
            }

            if (opts.stopAfterPhase == options::Phase::PARSER) {
                return {make_unique<ast::EmptyTree>(), file};
            }

            unique_ptr<ast::Expression> ast;
            core::MutableContext ctx(lgs, core::Symbols::root());
            {
                logger->trace("Desugaring: {}", file.data(lgs).path());
                core::ErrorRegion errs(lgs, file);
                core::UnfreezeNameTable nameTableAccess(lgs); // creates temporaries during desugaring
                ast = ast::desugar::node2Tree(ctx, move(nodes));
            }
            if (print.Desugared) {
                fmt::print("{}\n", ast->toStringWithTabs(lgs, 0));
            }
            if (print.DesugaredRaw) {
                fmt::print("{}\n", ast->showRaw(lgs));
            }
            if (opts.stopAfterPhase == options::Phase::DESUGARER) {
                return {make_unique<ast::EmptyTree>(), file};
            }

            if (!opts.skipDSLPasses) {
                logger->trace("Inlining DSLs: {}", file.data(lgs).path());
                core::UnfreezeNameTable nameTableAccess(lgs); // creates temporaries during desugaring
                core::ErrorRegion errs(lgs, file);
                dslsInlined.tree = dsl::DSL::run(ctx, move(ast));
            } else {
                dslsInlined.tree = move(ast);
            }
        }
        if (print.DSLTree) {
            fmt::print("{}\n", dslsInlined.tree->toStringWithTabs(lgs, 0));
        }
        if (print.DSLTreeRaw) {
            fmt::print("{}\n", dslsInlined.tree->showRaw(lgs));
        }
        if (opts.stopAfterPhase == options::Phase::DSL) {
            return {make_unique<ast::EmptyTree>(), file};
        }

        return dslsInlined;
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = lgs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
            e.setHeader("Exception parsing file: `{}` (backtrace is above)", file.data(lgs).path());
        }
        return {make_unique<ast::EmptyTree>(), file};
    }
}

vector<ast::ParsedFile> incrementalResolve(core::GlobalState &gs, vector<ast::ParsedFile> what,
                                           const options::Options &opts, shared_ptr<spdlog::logger> logger) {
    try {
        int i = 0;
        for (auto &tree : what) {
            auto file = tree.file;
            try {
                unique_ptr<ast::Expression> ast;
                core::MutableContext ctx(gs, core::Symbols::root());
                logger->trace("Naming: {}", file.data(gs).path());
                core::ErrorRegion errs(gs, file);
                core::UnfreezeSymbolTable symbolTable(gs);
                core::UnfreezeNameTable nameTable(gs);
                tree = sorbet::namer::Namer::run(ctx, move(tree));
                i++;
            } catch (SorbetException &) {
                if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
                    e.setHeader("Exception naming file: `{}` (backtrace is above)", file.data(gs).path());
                }
            }
        }

        core::MutableContext ctx(gs, core::Symbols::root());
        {
            Timer timeit(logger, "incremental_resolve");
            logger->trace("Resolving (incremental pass)...");
            core::ErrorRegion errs(gs, sorbet::core::FileRef());
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);

            what = sorbet::resolver::Resolver::runTreePasses(ctx, move(what));
        }
    } catch (SorbetException &) {
        if (auto e = gs.beginError(sorbet::core::Loc::none(), sorbet::core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }

    return what;
}

vector<core::FileRef> reserveFiles(unique_ptr<core::GlobalState> &gs, const vector<string> &files,
                                   shared_ptr<spdlog::logger> logger) {
    vector<core::FileRef> ret;
    core::UnfreezeFileTable unfreezeFiles(*gs);
    for (auto f : files) {
        logger->trace("enqueue: {}", f);
        auto fileRef = gs->findFileByPath(f);
        if (!fileRef.exists()) {
            fileRef = gs->reserveFileRef(f);
        }
        ret.emplace_back(move(fileRef));
    }
    return ret;
}

void readFile(unique_ptr<core::GlobalState> &gs, core::FileRef file, const options::Options &opts,
              shared_ptr<spdlog::logger> logger) {
    if (file.dataAllowingUnsafe(*gs).sourceType != core::File::NotYetRead) {
        return;
    }
    auto fileName = file.dataAllowingUnsafe(*gs).path();
    logger->trace("Reading: {}", fileName);
    string src;
    bool fileFound = true;
    try {
        src = FileOps::read(fileName);
    } catch (FileNotFoundException e) {
        // continue with an empty source, because the
        // assertion below requires every input file to map
        // to one output tree
        fileFound = false;
    }
    prodCounterAdd("types.input.bytes", src.size());
    prodCounterInc("types.input.files");

    {
        core::UnfreezeFileTable unfreezeFiles(*gs);
        auto entered = gs->enterNewFileAt(
            make_shared<core::File>(string(fileName.begin(), fileName.end()), move(src), core::File::Normal), file);
        ENFORCE(entered == file);
    }
    if (enable_counters) {
        counterAdd("types.input.lines", file.data(*gs).lineCount());
    }

    auto &fileData = file.data(*gs);
    if (!fileFound) {
        if (auto e = gs->beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
            e.setHeader("File Not Found");
        }
    }

    core::StrictLevel minStrict = opts.forceMinStrict;
    core::StrictLevel maxStrict = opts.forceMaxStrict;
    if (!opts.typedSource.empty() && fileData.path().find(opts.typedSource) != string::npos) {
        minStrict = core::StrictLevel::Typed;
    }
    auto fnd = opts.strictnessOverrides.find(string(fileData.path()));
    if (fnd != opts.strictnessOverrides.end()) {
        if (fnd->second == fileData.originalSigil) {
            core::ErrorRegion errs(*gs, file);
            if (auto e = gs->beginError(sorbet::core::Loc::none(file), core::errors::Parser::ParserError)) {
                e.setHeader("Useless override of strictness level");
            }
        }
        fileData.strictLevel = fnd->second;
    } else {
        if (fileData.originalSigil == core::StrictLevel::None) {
            fileData.strictLevel = core::StrictLevel::Stripe;
        } else {
            fileData.strictLevel = fileData.originalSigil;
        }
    }

    fileData.strictLevel = max(min(fileData.strictLevel, maxStrict), minStrict);

    if (opts.print.Autogen || opts.print.AutogenMsgPack) {
        // Autogen stops before infer but needs to see
        // all definitions
        fileData.strictLevel = core::StrictLevel::Stripe;
    }

    switch (fileData.strictLevel) {
        case core::StrictLevel::None:
            Exception::raise("Should never happen");
            break;
        case core::StrictLevel::Ignore:
            prodCounterInc("types.input.files.sigil.ignore");
            break;
        case core::StrictLevel::Internal:
            Exception::raise("Should never happen");
            break;
        case core::StrictLevel::Stripe:
            prodCounterInc("types.input.files.sigil.none");
            break;
        case core::StrictLevel::Typed:
            prodCounterInc("types.input.files.sigil.typed");
            break;
        case core::StrictLevel::Strict:
            prodCounterInc("types.input.files.sigil.strictLevel");
            break;
        case core::StrictLevel::Strong:
            prodCounterInc("types.input.files.sigil.strong");
            break;
        case core::StrictLevel::Max:
            Exception::raise("Should never happen");
            break;
        case core::StrictLevel::Autogenerated:
            prodCounterInc("types.input.files.sigil.autogenerated");
            break;
    }

    if (!opts.storeState.empty()) {
        fileData.sourceType = core::File::PayloadGeneration;
    }
}

vector<ast::ParsedFile> index(unique_ptr<core::GlobalState> &gs, vector<core::FileRef> files,
                              const options::Options &opts, WorkerPool &workers, unique_ptr<KeyValueStore> &kvstore,
                              shared_ptr<spdlog::logger> logger) {
    vector<ast::ParsedFile> ret;
    vector<ast::ParsedFile> empty;

    if (opts.stopAfterPhase == options::Phase::INIT) {
        return empty;
    }

    shared_ptr<ConcurrentBoundedQueue<core::FileRef>> fileq;

    shared_ptr<BlockingBoundedQueue<thread_result>> resultq;

    {
        fileq = make_shared<ConcurrentBoundedQueue<core::FileRef>>(files.size());
        resultq = make_shared<BlockingBoundedQueue<thread_result>>(files.size());
    }

    gs->sanityCheck();

    if (files.size() < 3) {
        // Run singlethreaded if only using 2 files
        for (auto file : files) {
            readFile(gs, file, opts, logger);
            ret.emplace_back(indexOne(opts, *gs, file, kvstore, logger));
        }
    } else {
        ProgressIndicator indexingProgress(opts.showProgress, "Indexing", files.size());
        const shared_ptr<core::GlobalState> cgs = move(gs);
        for (auto &file : files) {
            fileq->push(move(file), 1);
        }
        {
            workers.multiplexJob("index", [cgs, &opts, fileq, resultq, &kvstore, logger]() {
                logger->trace("worker deep copying global state");
                auto lgs = cgs->deepCopy();
                logger->trace("worker done deep copying global state");
                thread_result threadResult;
                int processedByThread = 0;
                core::FileRef job;

                {
                    for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                        if (result.gotItem()) {
                            core::FileRef file = job;
                            processedByThread++;
                            readFile(lgs, file, opts, logger);
                            threadResult.trees.emplace_back(indexOne(opts, *lgs, file, kvstore, logger));
                        }
                    }
                }

                if (processedByThread > 0) {
                    threadResult.counters = getAndClearThreadCounters();
                    threadResult.gs = move(lgs);
                    resultq->push(move(threadResult), processedByThread);
                }
            });
            logger->trace("Deep copying global state");
            unique_ptr<core::GlobalState> mainTheadGs = cgs->deepCopy();
            logger->trace("Done deep copying global state");
            gs = move(mainTheadGs);
        }

        thread_result threadResult;
        {
            logger->trace("Collecting results from indexing threads");
            for (auto result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS); !result.done();
                 result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS)) {
                if (result.gotItem()) {
                    logger->trace("Building global substitution");
                    core::GlobalSubstitution substitution(*threadResult.gs, *gs, cgs.get());
                    logger->trace("Consuming counters");
                    counterConsume(move(threadResult.counters));
                    core::MutableContext ctx(*gs, core::Symbols::root());
                    logger->trace("Running tree substitution");
                    for (auto &tree : threadResult.trees) {
                        auto file = tree.file;
                        core::ErrorRegion errs(*gs, file);
                        if (!file.data(*gs).cachedParseTree) {
                            tree.tree = ast::Substitute::run(ctx, substitution, move(tree.tree));
                            if (kvstore) {
                                string fileHashKey = fileKey(*gs, file);
                                kvstore->write(fileHashKey,
                                               core::serialize::Serializer::storeExpression(*gs, tree.tree));
                            }
                        }
                        ret.emplace_back(move(tree));
                    }
                    logger->trace("Tree substitution done");
                }
                gs->errorQueue->flushErrors();
                indexingProgress.reportProgress(fileq->doneEstimate());
            }
            logger->trace("Done collecting results from indexing threads");
        }
    }
    ENFORCE(files.size() == ret.size());

    auto by_file = [](ast::ParsedFile const &a, ast::ParsedFile const &b) { return a.file < b.file; };
    fast_sort(ret, by_file);

    return ret;
}

ast::ParsedFile typecheckOne(core::Context ctx, ast::ParsedFile resolved, const options::Options &opts,
                             shared_ptr<spdlog::logger> logger) {
    ast::ParsedFile result{make_unique<ast::EmptyTree>(), resolved.file};
    core::FileRef f = resolved.file;
    if (opts.stopAfterPhase == options::Phase::NAMER || opts.stopAfterPhase == options::Phase::RESOLVER) {
        return result;
    }
    if (f.data(ctx).isRBI()) {
        return result;
    }

    try {
        if (opts.print.CFG || opts.print.CFGRaw) {
            fmt::print("digraph \"{}\" {{\n", FileOps::getFileName(f.data(ctx).path()));
        }
        CFG_Collector_and_Typer collector(opts);
        {
            logger->trace("CFG+Infer: {}", f.data(ctx).path());
            core::ErrorRegion errs(ctx, f);
            result.tree = ast::TreeMap::apply(ctx, collector, move(resolved.tree));
        }
        if (wantTypedSource(opts, ctx, f)) {
            fmt::print("{}", ctx.state.showAnnotatedSource(f));
        }
        if (opts.print.CFG || opts.print.CFGRaw) {
            fmt::print("}}\n\n");
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = ctx.state.beginError(sorbet::core::Loc::none(f), core::errors::Internal::InternalError)) {
            e.setHeader("Exception in cfg+infer: {} (backtrace is above)", f.data(ctx).path());
        }
    }
    return result;
}

struct typecheck_thread_result {
    vector<ast::ParsedFile> trees;
    CounterState counters;
};

vector<ast::ParsedFile> name(core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
                             shared_ptr<spdlog::logger> logger, bool skipConfigatron) {
    if (!skipConfigatron) {
        core::UnfreezeNameTable nameTableAccess(gs);     // creates names from config
        core::UnfreezeSymbolTable symbolTableAccess(gs); // creates methods for them
        namer::configatron::fillInFromFileSystem(gs, opts.configatronDirs, opts.configatronFiles);
    }

    {
        ProgressIndicator namingProgress(opts.showProgress, "Naming", what.size());

        Timer timeit(logger, "naming");
        int i = 0;
        for (auto &tree : what) {
            auto file = tree.file;
            try {
                ast::ParsedFile ast;
                {
                    core::MutableContext ctx(gs, core::Symbols::root());
                    logger->trace("Naming: {}", file.data(gs).path());
                    core::ErrorRegion errs(gs, file);
                    core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
                    core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
                    tree = namer::Namer::run(ctx, move(tree));
                }
                gs.errorQueue->flushErrors();
                namingProgress.reportProgress(i);
                i++;
            } catch (SorbetException &) {
                Exception::failInFuzzer();
                if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
                    e.setHeader("Exception naming file: `{}` (backtrace is above)", file.data(gs).path());
                }
            }
        }
    }

    return what;
}

vector<ast::ParsedFile> resolve(core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
                                shared_ptr<spdlog::logger> logger, bool skipConfigatron) {
    try {
        what = name(gs, move(what), opts, logger, skipConfigatron);

        for (auto &named : what) {
            if (opts.print.NameTree) {
                fmt::print("{}\n", named.tree->toStringWithTabs(gs, 0));
            }
            if (opts.print.NameTreeRaw) {
                fmt::print("{}\n", named.tree->showRaw(gs));
            }
        }

        if (opts.stopAfterPhase == options::Phase::NAMER) {
            return what;
        }

        core::MutableContext ctx(gs, core::Symbols::root());
        ProgressIndicator namingProgress(opts.showProgress, "Resolving", 1);
        {
            Timer timeit(logger, "resolving");
            logger->trace("Resolving (global pass)...");
            vector<core::ErrorRegion> errs;
            for (auto &tree : what) {
                auto file = tree.file;
                errs.emplace_back(gs, file);
            }
            core::UnfreezeNameTable nameTableAccess(gs);     // Resolver::defineAttr
            core::UnfreezeSymbolTable symbolTableAccess(gs); // enters stubs
            what = resolver::Resolver::run(ctx, move(what));
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs.beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }
    gs.errorQueue->flushErrors();

    for (auto &resolved : what) {
        if (opts.print.ResolveTree) {
            fmt::print("{}\n", resolved.tree->toString(gs));
        }
        if (opts.print.ResolveTreeRaw) {
            fmt::print("{}\n", resolved.tree->showRaw(gs));
        }
    }
    return what;
}

vector<ast::ParsedFile> typecheck(unique_ptr<core::GlobalState> &gs, vector<ast::ParsedFile> what,
                                  const options::Options &opts, WorkerPool &workers,
                                  shared_ptr<spdlog::logger> logger) {
    vector<ast::ParsedFile> typecheck_result;

    {
        Timer timeit(logger, "infer_and_cfg");

        shared_ptr<ConcurrentBoundedQueue<ast::ParsedFile>> fileq;
        shared_ptr<BlockingBoundedQueue<typecheck_thread_result>> resultq;

        {
            fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(what.size());
            resultq = make_shared<BlockingBoundedQueue<typecheck_thread_result>>(what.size());
        }

        core::Context ctx(*gs, core::Symbols::root());
        for (auto &resolved : what) {
            logger->trace("enqueue-typer {}", resolved.file.data(*gs).path());
            fileq->push(move(resolved), 1);
        }

        {
            ProgressIndicator cfgInferProgress(opts.showProgress, "CFG+Inference", what.size());
            workers.multiplexJob("typecheck", [ctx, &opts, fileq, resultq, logger]() {
                typecheck_thread_result threadResult;
                ast::ParsedFile job;
                int processedByThread = 0;

                {
                    for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                        if (result.gotItem()) {
                            processedByThread++;
                            core::FileRef file = job.file;
                            try {
                                threadResult.trees.emplace_back(typecheckOne(ctx, move(job), opts, logger));
                            } catch (SorbetException &) {
                                Exception::failInFuzzer();
                                logger->error("Exception typing file: {} (backtrace is above)", file.data(ctx).path());
                            }
                        }
                    }
                }
                if (processedByThread > 0) {
                    threadResult.counters = getAndClearThreadCounters();
                    resultq->push(move(threadResult), processedByThread);
                }
            });

            typecheck_thread_result threadResult;
            {
                for (auto result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS); !result.done();
                     result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS)) {
                    if (result.gotItem()) {
                        counterConsume(move(threadResult.counters));
                        typecheck_result.insert(typecheck_result.end(), make_move_iterator(threadResult.trees.begin()),
                                                make_move_iterator(threadResult.trees.end()));
                    }
                    cfgInferProgress.reportProgress(fileq->doneEstimate());
                    gs->errorQueue->flushErrors();
                }
            }
        }

        if (opts.print.SymbolTable) {
            fmt::print("{}\n", gs->toString());
        }
        if (opts.print.SymbolTableRaw) {
            fmt::print("{}\n", gs->toStringWithOptions(false, true));
        }
        if (opts.print.SymbolTableJson) {
            auto root = core::Proto::toProto(*gs, core::Symbols::root());
            core::Proto::toJSON(root, cout);
        }
        if (opts.print.SymbolTableFull) {
            fmt::print("{}\n", gs->toStringWithOptions(true, false));
        }
        if (opts.print.SymbolTableFullRaw) {
            fmt::print("{}\n", gs->toStringWithOptions(true, true));
        }
        if (opts.print.FileTableJson) {
            auto files = core::Proto::filesToProto(*gs);
            core::Proto::toJSON(files, cout);
        }

        return typecheck_result;
    }
}

unsigned int computeFileHash(shared_ptr<core::File> forWhat, shared_ptr<spdlog::logger> logger) {
    const static options::Options emptyOpts{};
    shared_ptr<core::GlobalState> lgs =
        make_shared<core::GlobalState>((make_shared<core::ErrorQueue>(*logger, *logger)));
    lgs->initEmpty();
    lgs->errorQueue->ignoreFlushes = true;
    lgs->silenceErrors = true;
    core::FileRef fref;
    {
        core::UnfreezeFileTable fileTableAccess(*lgs);
        fref = lgs->enterFile(forWhat);
    }
    vector<ast::ParsedFile> single;
    unique_ptr<KeyValueStore> kvstore;

    single.emplace_back(pipeline::indexOne(emptyOpts, *lgs, fref, kvstore, logger));
    auto errs = lgs->errorQueue->drainAllErrors();
    for (auto &e : errs) {
        if (e->what == core::errors::Parser::ParserError) {
            return core::GlobalState::HASH_STATE_INVALID;
        }
    }
    pipeline::resolve(*lgs, move(single), emptyOpts, logger, true);

    return lgs->hash();
}

} // namespace sorbet::realmain::pipeline
