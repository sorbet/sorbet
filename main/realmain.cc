#include "main/realmain.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/substitute/substitute.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "common/ConcurrentQueue.h"
#include "common/KeyValueStore.h"
#include "common/ProgressIndicator.h"
#include "common/WorkerPool.h"
#include "core/ErrorQueue.h"
#include "core/Files.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "dsl/dsl.h"
#include "infer/infer.h"
#include "namer/configatron/configatron.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "payload/text/text.h"
#include "resolver/resolver.h"
#include "spdlog/fmt/ostr.h"
#include "version/version.h"
#include <algorithm> // find
#include <iostream>
namespace spd = spdlog;
using namespace std;

namespace ruby_typer {
namespace realmain {
std::shared_ptr<spd::logger> logger;
int returnCode;

shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> make_stderr_color_sink() {
    auto color_sink = make_shared<spd::sinks::ansicolor_stderr_sink_mt>();
    color_sink->set_color(spd::level::info, color_sink->white);
    color_sink->set_color(spd::level::debug, color_sink->magenta);
    return color_sink;
}

shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderr_color_sink = make_stderr_color_sink();

const auto PROGRESS_REFRESH_TIME_MILLIS = ProgressIndicator::REPORTING_INTERVAL();

const string GLOBAL_STATE_KEY = "GlobalState";

bool wantTypedSource(const Options &opts, core::Context ctx, core::FileRef file) {
    if (opts.print.TypedSource) {
        return true;
    }
    if (opts.typedSource.empty()) {
        return false;
    }
    return file.data(ctx).path().find(opts.typedSource) != string::npos;
}

class CFG_Collector_and_Typer {
    const Options &opts;

public:
    CFG_Collector_and_Typer(const Options &opts) : opts(opts){};

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> m) {
        if (m->loc.file.data(ctx).strict == core::StrictLevel::Stripe) {
            return m;
        }
        auto &print = opts.print;
        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);

        bool printSrc = wantTypedSource(opts, ctx, m->loc.file);

        if (print.CFGRaw || printSrc) {
            cfg = cfg::CFGBuilder::addDebugEnvironment(ctx.withOwner(m->symbol), move(cfg));
        }
        if (opts.stopAfterPhase == Phase::CFG) {
            return m;
        }
        cfg = infer::Inference::run(ctx.withOwner(m->symbol), move(cfg));
        if (print.CFG || print.CFGRaw) {
            cout << cfg->toString(ctx) << '\n' << '\n';
        }
        if (printSrc) {
            cfg->recordAnnotations(ctx);
        }
        return m;
    }
};

struct thread_result {
    unique_ptr<core::GlobalState> gs;
    core::CounterState counters;
    vector<unique_ptr<ast::Expression>> trees;
};

unique_ptr<ast::Expression> indexOne(const Options &opts, unique_ptr<core::GlobalState> &lgs, core::FileRef file,
                                     std::unique_ptr<KeyValueStore> &kvstore,
                                     const shared_ptr<core::GlobalState> &pgs) {
    auto &print = opts.print;
    std::unique_ptr<ast::Expression> dslsInlined;

    try {
        if (kvstore && file.id() < pgs->filesUsed()) {
            auto maybeCached = kvstore->read(file.data(*pgs).hashKey());
            if (maybeCached) {
                auto t = core::serialize::Serializer::loadExpression(*pgs, maybeCached);
                t->loc.file.data(*pgs).cachedParseTree = true;
                dslsInlined = move(t);
            }
        }
        if (!dslsInlined) {
            // tree isn't cached. Need to start from parser

            std::unique_ptr<parser::Node> nodes;
            {
                logger->trace("Parsing: {}", file.data(*lgs).path());
                core::ErrorRegion errs(*lgs, file);
                core::UnfreezeNameTable nameTableAccess(*lgs); // enters strings from source code as names
                nodes = parser::Parser::run(*lgs, file);
            }
            if (print.ParseTree) {
                cout << nodes->toString(*lgs, 0) << '\n';
            }
            if (print.ParseTreeJSON) {
                cout << nodes->toJSON(*lgs, 0) << '\n';
            }
            if (opts.stopAfterPhase == Phase::PARSER) {
                return make_unique<ast::EmptyTree>(core::Loc::none(file));
            }

            std::unique_ptr<ast::Expression> ast;
            core::MutableContext ctx(*lgs, core::Symbols::root());
            {
                logger->trace("Desugaring: {}", file.data(*lgs).path());
                core::ErrorRegion errs(*lgs, file);
                core::UnfreezeNameTable nameTableAccess(*lgs); // creates temporaries during desugaring
                ast = ast::desugar::node2Tree(ctx, move(nodes));
            }
            if (print.Desugared) {
                cout << ast->toString(*lgs, 0) << '\n';
            }
            if (print.DesugaredRaw) {
                cout << ast->showRaw(*lgs) << '\n';
            }
            if (opts.stopAfterPhase == Phase::DESUGARER) {
                return make_unique<ast::EmptyTree>(core::Loc::none(file));
            }

            {
                logger->trace("Inlining DSLs: {}", file.data(*lgs).path());
                core::ErrorRegion errs(*lgs, file);
                dslsInlined = dsl::DSL::run(ctx, move(ast));
            }
        }
        if (print.DSLTree) {
            cout << dslsInlined->toString(*lgs, 0) << '\n';
        }
        if (print.DSLTreeRaw) {
            cout << dslsInlined->showRaw(*lgs) << '\n';
        }
        if (opts.stopAfterPhase == Phase::DSL) {
            return make_unique<ast::EmptyTree>(core::Loc::none(file));
        }

        return dslsInlined;
    } catch (...) {
        if (auto e = lgs->beginError(ruby_typer::core::Loc::none(file), core::errors::Internal::InternalError)) {
            e.setHeader("Exception parsing file: `{}` (backtrace is above)", file.data(*lgs).path());
        }
        returnCode = 12;
        return make_unique<ast::EmptyTree>(core::Loc::none(file));
    }
}

vector<unique_ptr<ast::Expression>> index(shared_ptr<core::GlobalState> &gs, std::vector<std::string> frs,
                                          std::vector<core::FileRef> mainThreadFiles, const Options &opts,
                                          WorkerPool &workers, std::unique_ptr<KeyValueStore> &kvstore) {
    vector<unique_ptr<ast::Expression>> ret;
    vector<unique_ptr<ast::Expression>> empty;

    if (opts.stopAfterPhase == Phase::INIT) {
        return empty;
    }

    shared_ptr<ConcurrentBoundedQueue<std::pair<int, std::string>>> fileq;

    shared_ptr<BlockingBoundedQueue<thread_result>> resultq;

    {
        Timer timeit(logger, "creating index queues");

        fileq = make_shared<ConcurrentBoundedQueue<std::pair<int, std::string>>>(frs.size());
        resultq = make_shared<BlockingBoundedQueue<thread_result>>(frs.size());
    }

    int i = gs->filesUsed();
    for (auto f : frs) {
        logger->trace("enqueue: {}", f);
        std::pair<int, std::string> job(i++, f);
        fileq->push(move(job), 1);
    }

    gs->sanityCheck();

    const std::shared_ptr<core::GlobalState> cgs = gs;
    gs = nullptr;
    logger->trace("Done deep copying global state");
    {
        ProgressIndicator indexingProgress(opts.showProgress, "Indexing", frs.size());

        workers.multiplexJob([cgs, &opts, fileq, resultq, &kvstore]() {
            logger->trace("worker deep copying global state");
            auto lgs = cgs->deepCopy();
            logger->trace("worker done deep copying global state");
            thread_result threadResult;
            int processedByThread = 0;
            std::pair<int, std::string> job;

            {
                for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                    if (result.gotItem()) {
                        core::ErrorRegion errs(*lgs, core::FileRef(job.first));
                        processedByThread++;
                        std::string fileName = job.second;
                        logger->trace("Reading: {}", fileName);
                        int fileId = job.first;
                        string src;
                        try {
                            src = FileOps::read(fileName.c_str());
                        } catch (FileNotFoundException e) {
                            if (auto e = lgs->beginError(ruby_typer::core::Loc::none(),
                                                         core::errors::Internal::InternalError)) {
                                e.setHeader("File Not Found: `{}`", fileName);
                            }
                            returnCode = 11;
                            // continue with an empty source, because the
                            // assertion below requires every input file to map
                            // to one output tree
                        }
                        core::prodCounterAdd("types.input.bytes", src.size());
                        core::prodCounterInc("types.input.files");

                        core::FileRef file;
                        {
                            core::UnfreezeFileTable unfreezeFiles(*lgs);
                            file = lgs->enterFileAt(fileName, src, fileId);
                        }
                        if (core::enable_counters) {
                            core::counterAdd("types.input.lines", file.data(*lgs).lineCount());
                        }

                        switch (file.data(*lgs).strict) {
                            case core::StrictLevel::Stripe:
                                core::prodCounterInc("types.input.files.sigil.none");
                                break;
                            case core::StrictLevel::Typed:
                                core::prodCounterInc("types.input.files.sigil.typed");
                                break;
                            case core::StrictLevel::Strict:
                                core::prodCounterInc("types.input.files.sigil.strict");
                                break;
                            case core::StrictLevel::Strong:
                                core::prodCounterInc("types.input.files.sigil.strong");
                                break;
                        }

                        core::StrictLevel minStrict = opts.forceMinStrict;
                        core::StrictLevel maxStrict = opts.forceMaxStrict;
                        if (!opts.typedSource.empty() &&
                            file.data(*lgs).path().find(opts.typedSource) != std::string::npos) {
                            minStrict = core::StrictLevel::Typed;
                        }
                        file.data(*lgs).strict = std::max(std::min(file.data(*lgs).sigil, maxStrict), minStrict);

                        if (!opts.storeState.empty()) {
                            file.data(*lgs).source_type = core::File::PayloadGeneration;
                        }

                        threadResult.trees.emplace_back(indexOne(opts, lgs, file, kvstore, cgs));
                    }
                }
            }

            if (processedByThread > 0) {
                threadResult.counters = core::getAndClearThreadCounters();
                threadResult.gs = move(lgs);
                resultq->push(move(threadResult), processedByThread);
            }
        });

        logger->trace("Deep copying global state");
        auto mainTheadGs = cgs->deepCopy();
        logger->trace("Done deep copying global state");

        for (auto f : mainThreadFiles) {
            ret.emplace_back(indexOne(opts, mainTheadGs, f, kvstore, cgs));
        }

        gs = move(mainTheadGs);

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
                        auto file = tree->loc.file;
                        if (!file.data(*gs).cachedParseTree) {
                            auto subst = ast::Substitute::run(ctx, substitution, move(tree));
                            if (kvstore) {
                                kvstore->write(file.data(*gs).hashKey(),
                                               core::serialize::Serializer::store(*gs, subst));
                            }
                            ret.emplace_back(move(subst));
                        } else {
                            ret.emplace_back(move(tree));
                        }
                    }
                    logger->trace("Tree substitution done");
                }
                gs->flushErrors();
                indexingProgress.reportProgress(fileq->doneEstimate());
            }
            logger->trace("Done collecting results from indexing threads");
        }
    }
    ENFORCE(mainThreadFiles.size() + frs.size() == ret.size());

    auto by_file = [](unique_ptr<ast::Expression> const &a, unique_ptr<ast::Expression> const &b) {
        return a->loc.file < b->loc.file;
    };
    std::sort(ret.begin(), ret.end(), by_file);

    return ret;
}

unique_ptr<ast::Expression> typecheckFile(core::Context ctx, unique_ptr<ast::Expression> resolved,
                                          const Options &opts) {
    unique_ptr<ast::Expression> result;
    core::FileRef f = resolved->loc.file;
    if (opts.stopAfterPhase == Phase::NAMER) {
        return make_unique<ast::EmptyTree>(core::Loc::none(f));
    }

    try {
        if (opts.print.CFG || opts.print.CFGRaw) {
            cout << "digraph \"" << FileOps::getFileName(f.data(ctx).path()) << "\"{" << '\n';
        }
        CFG_Collector_and_Typer collector(opts);
        {
            logger->trace("CFG+Infer: {}", f.data(ctx).path());
            core::ErrorRegion errs(ctx, f);
            result = ast::TreeMap::apply(ctx, collector, move(resolved));
        }
        if (wantTypedSource(opts, ctx, f)) {
            cout << ctx.state.showAnnotatedSource(f);
        }
        if (opts.print.CFG || opts.print.CFGRaw) {
            cout << "}" << '\n' << '\n';
        }
        if (opts.suggestTyped && !f.data(ctx).hadErrors() && f.data(ctx).sigil == core::StrictLevel::Stripe) {
            core::counterInc("types.input.files.suggest_typed");
            logger->error("Suggest adding # typed: true to: {}", f.data(ctx).path());
        }

    } catch (...) {
        if (auto e = ctx.state.beginError(ruby_typer::core::Loc::none(f), core::errors::Internal::InternalError)) {
            e.setHeader("Exception in cfg+infer: {} (backtrace is above)", f.data(ctx).path());
        }
        returnCode = 15;
    }
    return result;
}

struct typecheck_thread_result {
    vector<unique_ptr<ast::Expression>> trees;
    core::CounterState counters;
};

// If ever given a result type, it should be something along the lines of
// vector<pair<vector<unique_ptr<ast::Expression>>, unique_ptr<core::GlobalState>>>
void typecheck(shared_ptr<core::GlobalState> &gs, vector<unique_ptr<ast::Expression>> what, const Options &opts,
               WorkerPool &workers) {
    vector<vector<unique_ptr<ast::Expression>>> typecheck_result;

    try {
        {
            core::UnfreezeNameTable nameTableAccess(*gs);     // creates names from config
            core::UnfreezeSymbolTable symbolTableAccess(*gs); // creates methods for them
            namer::configatron::fillInFromFileSystem(*gs, opts.configatronDirs, opts.configatronFiles);
        }

        {
            ProgressIndicator namingProgress(opts.showProgress, "Naming", what.size());

            Timer timeit(logger, "naming");
            int i = 0;
            for (auto &tree : what) {
                auto file = tree->loc.file;
                try {
                    unique_ptr<ast::Expression> ast;
                    {
                        core::MutableContext ctx(*gs, core::Symbols::root());
                        logger->trace("Naming: {}", file.data(*gs).path());
                        core::ErrorRegion errs(*gs, file);
                        core::UnfreezeNameTable nameTableAccess(*gs);     // creates singletons and class names
                        core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters symbols
                        tree = namer::Namer::run(ctx, move(tree));
                    }
                    gs->flushErrors();
                    namingProgress.reportProgress(i);
                    i++;
                } catch (...) {
                    returnCode = 13;
                    if (auto e =
                            gs->beginError(ruby_typer::core::Loc::none(file), core::errors::Internal::InternalError)) {
                        e.setHeader("Exception naming file: `{}` (backtrace is above)", file.data(*gs).path());
                    }
                }
            }
        }

        core::MutableContext ctx(*gs, core::Symbols::root());
        ProgressIndicator namingProgress(opts.showProgress, "Resolving", 1);
        {
            Timer timeit(logger, "Resolving");
            logger->trace("Resolving (global pass)...");
            core::ErrorRegion errs(*gs, ruby_typer::core::FileRef());
            core::UnfreezeNameTable nameTableAccess(*gs);     // Resolver::defineAttr
            core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters stubs
            what = resolver::Resolver::run(ctx, move(what));
        }
    } catch (...) {
        if (auto e = gs->beginError(ruby_typer::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
        returnCode = 14;
    }
    gs->flushErrors();

    for (auto &resolved : what) {
        if (opts.print.NameTree) {
            cout << resolved->toString(*gs, 0) << '\n';
        }
        if (opts.print.NameTreeRaw) {
            cout << resolved->showRaw(*gs) << '\n';
        }
    }
    {
        Timer timeit(logger, "Infer+CFG");

        shared_ptr<ConcurrentBoundedQueue<unique_ptr<ast::Expression>>> fileq;
        shared_ptr<BlockingBoundedQueue<typecheck_thread_result>> resultq;

        {
            Timer timeit(logger, "creating typecheck queues");
            fileq = make_shared<ConcurrentBoundedQueue<unique_ptr<ast::Expression>>>(what.size());
            resultq = make_shared<BlockingBoundedQueue<typecheck_thread_result>>(what.size());
        }

        core::Context ctx(*gs, core::Symbols::root());
        for (auto &resolved : what) {
            logger->trace("enqueue-typer {}", resolved->loc.file.data(*gs).path());
            fileq->push(move(resolved), 1);
        }

        {
            ProgressIndicator cfgInferProgress(opts.showProgress, "CFG+Inference", what.size());
            workers.multiplexJob([ctx, &opts, fileq, resultq]() {
                typecheck_thread_result threadResult;
                unique_ptr<ast::Expression> job;
                int processedByThread = 0;

                {
                    for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                        if (result.gotItem()) {
                            processedByThread++;
                            core::FileRef file = job->loc.file;
                            core::ErrorRegion errs(ctx, file);
                            try {
                                threadResult.trees.emplace_back(typecheckFile(ctx, move(job), opts));
                            } catch (...) {
                                logger->error("Exception typing file: {} (backtrace is above)", file.data(ctx).path());
                            }
                        }
                    }
                }
                if (processedByThread > 0) {
                    threadResult.counters = core::getAndClearThreadCounters();
                    resultq->push(move(threadResult), processedByThread);
                }
            });

            typecheck_thread_result threadResult;
            {
                for (auto result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS); !result.done();
                     result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS)) {
                    if (result.gotItem()) {
                        counterConsume(move(threadResult.counters));
                        typecheck_result.emplace_back(move(threadResult.trees));
                    }
                    cfgInferProgress.reportProgress(fileq->doneEstimate());
                    gs->flushErrors();
                }
            }
        }

        if (opts.print.NameTable) {
            cout << gs->toString() << '\n';
        }
        if (opts.print.NameTableJson) {
            cout << gs->toJSON() << '\n';
        }
        if (opts.print.NameTableFull) {
            cout << gs->toString(true) << '\n';
        }
        return;
    }
}

void createInitialGlobalState(std::shared_ptr<core::GlobalState> &gs, const Options &options,
                              std::unique_ptr<KeyValueStore> &kvstore) {
    if (kvstore) {
        auto maybeGsBytes = kvstore->read(GLOBAL_STATE_KEY);
        if (maybeGsBytes) {
            Timer timeit(logger, "Read cached global state");
            core::serialize::Serializer::loadGlobalState(*gs, maybeGsBytes);
            return;
        }
    }
    if (options.noStdlib) {
        gs->initEmpty();
        return;
    }

    const u1 *const nameTablePayload = getNameTablePayload;
    if (nameTablePayload == nullptr) {
        gs->initEmpty();
        Timer timeit(logger, "Indexed payload");

        vector<core::FileRef> payloadFiles;
        {
            core::UnfreezeFileTable fileTableAccess(*gs);
            for (auto &p : rbi::all()) {
                auto file = gs->enterFile(p.first, p.second);
                file.data(*gs).source_type = core::File::PayloadGeneration;
                payloadFiles.push_back(move(file));
            }
        }
        Options emptyOpts;
        emptyOpts.threads = 1;
        WorkerPool workers(emptyOpts.threads, logger);
        vector<std::string> empty;
        typecheck(gs, index(gs, empty, payloadFiles, emptyOpts, workers, kvstore), emptyOpts,
                  workers); // result is thrown away
    } else {
        Timer timeit(logger, "Read serialized payload");
        core::serialize::Serializer::loadGlobalState(*gs, nameTablePayload);
    }
}

int realmain(int argc, const char *argv[]) {
    returnCode = 0;
    logger = spd::details::registry::instance().create("console", stderr_color_sink);
    logger->set_level(spd::level::trace); // pass through everything, let the sinks decide
    logger->set_pattern("%v");
    fatalLogger = logger;

    auto typeErrorsConsole = spd::details::registry::instance().create("typeErrors", stderr_color_sink);

    Options opts;
    readOptions(opts, argc, argv);
    if (!opts.debugLogFile.empty()) {
        auto fileSink = std::make_shared<spd::sinks::simple_file_sink_mt>(opts.debugLogFile);
        fileSink->set_level(spd::level::debug);
        { // replace console & fatal loggers
            std::vector<spd::sink_ptr> sinks{stderr_color_sink, fileSink};
            auto combinedLogger = std::make_shared<spd::logger>("consoleAndFile", begin(sinks), end(sinks));
            combinedLogger->flush_on(spdlog::level::err);
            combinedLogger->set_level(spd::level::trace); // pass through everything, let the sinks decide

            spd::register_logger(combinedLogger);
            fatalLogger = combinedLogger;
            logger = combinedLogger;
        }
        { // replace type error logger
            std::vector<spd::sink_ptr> sinks{stderr_color_sink, fileSink};
            auto combinedLogger = std::make_shared<spd::logger>("typeErrorsAndFile", begin(sinks), end(sinks));
            spd::register_logger(combinedLogger);
            combinedLogger->set_level(spd::level::trace); // pass through everything, let the sinks decide
            typeErrorsConsole = combinedLogger;
        }
    }
    logger->set_pattern("%v");
    // Use a custom formatter so we don't get a default newline
    auto formatter = make_shared<spd::pattern_formatter>("%v", spd::pattern_time_type::local, "");
    typeErrorsConsole->set_formatter(formatter);

    switch (opts.logLevel) {
        case 0:
            stderr_color_sink->set_level(spd::level::info);
            break;
        case 1:
            stderr_color_sink->set_level(spd::level::debug);
            logger->set_pattern("[T%t][%Y-%m-%dT%T.%f] %v");
            logger->debug("Debug logging enabled");
            break;
        default:
            stderr_color_sink->set_level(spd::level::trace);
            logger->set_pattern("[T%t][%Y-%m-%dT%T.%f] %v");
            logger->trace("Trace logging enabled");
            break;
    }

    {
        std::string argsConcat(argv[0]);
        for (int i = 1; i < argc; i++) {
            std::string argString(argv[i]);
            argsConcat = argsConcat + " " + argString;
        }
        logger->debug("Running ruby-typer version {} with arguments: {}", Version::build_scm_revision, argsConcat);
    }
    WorkerPool workers(opts.threads, logger);

    shared_ptr<core::GlobalState> gs =
        make_shared<core::GlobalState>((std::make_shared<core::ErrorQueue>(*typeErrorsConsole, *logger)));

    logger->trace("building initial global state");
    unique_ptr<KeyValueStore> kvstore;
    if (!opts.cacheDir.empty()) {
        kvstore = std::make_unique<KeyValueStore>(Version::build_scm_revision, opts.cacheDir);
    }
    createInitialGlobalState(gs, opts, kvstore);
    if (opts.silenceErrors) {
        gs->silenceErrors = true;
    }
    logger->trace("done building initial global state");

    Timer timeall(logger, "Done in");
    vector<core::FileRef> inputFiles;
    logger->trace("Files: ");
    {
        Timer timeit(logger, "reading files");
        core::UnfreezeFileTable fileTableAccess(*gs);
        if (!opts.inlineInput.empty()) {
            core::prodCounterAdd("types.input.bytes", opts.inlineInput.size());
            core::prodCounterInc("types.input.lines");
            core::prodCounterInc("types.input.files");
            auto file = gs->enterFile(string("-e"), opts.inlineInput + "\n");
            inputFiles.push_back(file);
            if (opts.forceMaxStrict < core::StrictLevel::Typed) {
                logger->error("`-e` is incompatible with `--typed=ruby`");
                return 1;
            }
            file.data(*gs).strict = core::StrictLevel::Strict;
        }
    }

    vector<unique_ptr<ast::Expression>> indexed;
    {
        Timer timeit(logger, "index");
        indexed = index(gs, opts.inputFileNames, inputFiles, opts, workers, kvstore);
    }

    if (kvstore && gs->wasModified()) {
        Timer timeit(logger, "caching global state");
        kvstore->write(GLOBAL_STATE_KEY, core::serialize::Serializer::store(*gs));
    }

    { typecheck(gs, move(indexed), opts, workers); }
    logger->trace("ruby-typer done");

    if (!opts.storeState.empty()) {
        gs->markAsPayload();
        FileOps::write(opts.storeState.c_str(), core::serialize::Serializer::store(*gs));
    }

    if (opts.someCounters.size() != 0) {
        if (opts.enableCounters) {
            logger->error("Don't pass both --counters and --counter");
            return 1;
        }
        logger->warn("" + core::getCounterStatistics(opts.someCounters));
    }

    if (opts.enableCounters) {
        logger->warn("" + core::getCounterStatistics(core::Counters::ALL_COUNTERS));
    } else {
        logger->debug("" + core::getCounterStatistics(core::Counters::ALL_COUNTERS));
    }

    if (!opts.statsdHost.empty()) {
        core::submitCountersToStatsd(opts.statsdHost, opts.statsdPort, opts.statsdPrefix + ".counters");
    }

    if (!opts.metricsFile.empty()) {
        string status;
        if (gs->hadCriticalError()) {
            status = "Error";
        } else if (returnCode != 0) {
            status = "Failure";
        } else {
            status = "Success";
        }

        core::storeCountersToProtoFile(opts.metricsFile, opts.metricsPrefix, opts.metricsRepo, opts.metricsBranch,
                                       opts.metricsSha, status);
    }
    if (gs->hadCriticalError()) {
        returnCode = 10;
    } else if (returnCode == 0 && gs->totalErrors() > 0 && !opts.supressNonCriticalErrors) {
        returnCode = 1;
    }

    // je_malloc_stats_print(nullptr, nullptr, nullptr); // uncomment this to print jemalloc statistics

    return returnCode;
}

EarlyReturnWithCode::EarlyReturnWithCode(int returnCode)
    : SRubyException("early return with code " + to_string(returnCode)), returnCode(returnCode){};
} // namespace realmain
} // namespace ruby_typer
