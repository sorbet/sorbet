#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/substitute/substitute.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "common/ConcurrentQueue.h"
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
#include "rang.hpp"
#include "resolver/resolver.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include "version/version.h"
#include <algorithm> // find
#include <ctime>
#include <cxxopts.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace spd = spdlog;
using namespace std;

namespace ruby_typer {

struct Printers {
    bool ParseTree = false;
    bool ParseTreeJSON = false;
    bool Desugared = false;
    bool DesugaredRaw = false;
    bool DSLTree = false;
    bool DSLTreeRaw = false;
    bool NameTree = false;
    bool NameTreeRaw = false;
    bool NameTable = false;
    bool NameTableFull = false;
    bool CFG = false;
    bool CFGRaw = false;
    bool TypedSource = false;
};

enum Phase {
    INIT,
    PARSER,
    DESUGARER,
    DSL,
    NAMER,
    CFG,
    INFERENCER,
};

struct Options {
    Printers print;
    Phase stopAfterPhase = Phase::INFERENCER;
    bool noStdlib = false;
    bool forceTyped = false;
    bool forceUntyped = false;
    bool showProgress = false;
    bool storeState = false;
    int threads = 0;
    string typedSource = "";
    std::vector<string> configatronDirs;
    std::vector<string> configatronFiles;
};

shared_ptr<spdlog::sinks::ansicolor_stderr_sink_mt> make_stderr_color_sink() {
    auto color_sink = make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>();
    color_sink->set_color(spd::level::info, color_sink->white);
    color_sink->set_color(spd::level::debug, color_sink->magenta);
    return color_sink;
}

shared_ptr<spdlog::sinks::ansicolor_stderr_sink_mt> stderr_color_sink = make_stderr_color_sink();

shared_ptr<spd::logger> make_tracer() {
    auto tracer = spd::details::registry::instance().create("tracer", stderr_color_sink);
    tracer->set_pattern("[T%t][%Y-%m-%dT%T.%f] %v");
    tracer->set_level(spd::level::off);
    return tracer;
}

shared_ptr<spd::logger> console_err;
shared_ptr<spd::logger> tracer = make_tracer();
shared_ptr<spd::logger> console;

const auto PROGRESS_REFRESH_TIME_MILLIS = ProgressIndicator::REPORTING_INTERVAL();

int returnCode = 0;

struct {
    string option;
    bool Printers::*flag;
} print_options[] = {
    {"parse-tree", &Printers::ParseTree},
    {"parse-tree-json", &Printers::ParseTreeJSON},
    {"ast", &Printers::Desugared},
    {"ast-raw", &Printers::DesugaredRaw},
    {"dsl-tree", &Printers::DSLTree},
    {"dsl-tree-raw", &Printers::DSLTreeRaw},
    {"name-table", &Printers::NameTable},
    {"name-table-full", &Printers::NameTableFull},
    {"name-tree", &Printers::NameTree},
    {"name-tree-raw", &Printers::NameTreeRaw},
    {"cfg", &Printers::CFG},
    {"cfg-raw", &Printers::CFGRaw},
    {"typed-source", &Printers::TypedSource},
};

struct {
    string option;
    Phase flag;
} stop_after_options[] = {
    {"init", Phase::INIT},   {"parser", Phase::PARSER}, {"desugarer", Phase::DESUGARER},   {"dsl", Phase::DSL},
    {"namer", Phase::NAMER}, {"cfg", Phase::CFG},       {"inferencer", Phase::INFERENCER},
};

long timespec_delta(struct timespec *start, struct timespec *stop) {
    return (stop->tv_sec - start->tv_sec) * 1000000000 + stop->tv_nsec - start->tv_nsec;
}

bool wantTypedSource(const Options &opts, core::Context ctx, core::FileRef file) {
    if (opts.print.TypedSource) {
        return true;
    }
    if (opts.typedSource.empty()) {
        return false;
    }
    return file.data(ctx).path().find(opts.typedSource) != string::npos;
}

class Timer {
public:
    Timer(shared_ptr<spd::logger> log, const std::string &msg) : log(log), msg(msg) {
        clock_gettime(CLOCK_REALTIME, &begin);
    }

    ~Timer() {
        struct timespec end;
        clock_gettime(CLOCK_REALTIME, &end);
        log->debug("{}: {}ms", this->msg, timespec_delta(&begin, &end) / 1000000);
    }

private:
    shared_ptr<spd::logger> log;
    const std::string msg;
    struct timespec begin;
};

class CFG_Collector_and_Typer {
    const Options &opts;

public:
    CFG_Collector_and_Typer(const Options &opts) : opts(opts){};

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> m) {
        if (m->loc.file.data(ctx).source_type != core::File::Typed) {
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
    CounterState counters;
    vector<unique_ptr<ast::Expression>> trees;
};

unique_ptr<ast::Expression> indexOne(const Options &opts, unique_ptr<core::GlobalState> &lgs, core::FileRef file) {
    auto &print = opts.print;
    try {
        std::unique_ptr<parser::Node> nodes;
        {
            tracer->trace("Parsing: {}", file.data(*lgs).path());
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

        core::MutableContext ctx(*lgs, core::Symbols::root());
        std::unique_ptr<ast::Expression> ast;
        {
            tracer->trace("Desugaring: {}", file.data(*lgs).path());
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
            tracer->trace("Inlining DSLs: {}", file.data(*lgs).path());
            core::ErrorRegion errs(*lgs, file);
            ast = dsl::DSL::run(ctx, move(ast));
        }
        if (print.DSLTree) {
            cout << ast->toString(*lgs, 0) << '\n';
        }
        if (print.DSLTreeRaw) {
            cout << ast->showRaw(*lgs) << '\n';
        }
        if (opts.stopAfterPhase == Phase::DSL) {
            return make_unique<ast::EmptyTree>(core::Loc::none(file));
        }

        return ast;
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
                                          WorkerPool &workers) {
    vector<unique_ptr<ast::Expression>> result;
    vector<unique_ptr<ast::Expression>> empty;
    vector<unique_ptr<ast::Expression>> trees;

    if (opts.stopAfterPhase == Phase::INIT) {
        return empty;
    }

    shared_ptr<ConcurrentBoundedQueue<std::pair<int, std::string>>> fileq;

    shared_ptr<BlockingBoundedQueue<thread_result>> resultq;

    {
        Timer timeit(console_err, "creating index queues");

        fileq = make_shared<ConcurrentBoundedQueue<std::pair<int, std::string>>>(frs.size());
        resultq = make_shared<BlockingBoundedQueue<thread_result>>(frs.size());
    }

    int i = gs->filesUsed();
    for (auto f : frs) {
        tracer->trace("enqueue: {}", f);
        std::pair<int, std::string> job(i++, f);
        fileq->push(move(job), 1);
    }

    gs->sanityCheck();

    const std::shared_ptr<core::GlobalState> cgs = gs;
    gs = nullptr;
    tracer->trace("Done deep copying global state");
    {
        ProgressIndicator indexingProgress(opts.showProgress, "Indexing", frs.size());

        workers.multiplexJob([cgs, opts, fileq, resultq]() {
            tracer->trace("worker deep copying global state");
            auto lgs = cgs->deepCopy();
            tracer->trace("worker done deep copying global state");
            thread_result threadResult;
            int processedByThread = 0;
            std::pair<int, std::string> job;

            {
                for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                    if (result.gotItem()) {
                        core::ErrorRegion errs(*lgs, core::FileRef(job.first));
                        processedByThread++;
                        std::string fileName = job.second;
                        tracer->trace("Reading: {}", fileName);
                        int fileId = job.first;
                        string src;
                        try {
                            src = File::read(fileName.c_str());
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
                        counterAdd("types.input.bytes", src.size());
                        counterAdd("types.input.lines", count(src.begin(), src.end(), '\n'));
                        counterInc("types.input.files");

                        core::FileRef file;
                        {
                            core::UnfreezeFileTable unfreezeFiles(*lgs);
                            file = lgs->enterFileAt(fileName, src, fileId);
                        }

                        bool forceTypedSource = !opts.typedSource.empty() &&
                                                file.data(*lgs).path().find(opts.typedSource) != std::string::npos;
                        if (forceTypedSource || opts.forceTyped) {
                            file.data(*lgs).source_type = core::File::Typed;
                        } else if (opts.forceUntyped) {
                            file.data(*lgs).source_type = core::File::Untyped;
                        } else if (opts.storeState) {
                            file.data(*lgs).source_type = core::File::PayloadGeneration;
                        }

                        threadResult.trees.emplace_back(indexOne(opts, lgs, file));
                    }
                }
            }

            if (processedByThread > 0) {
                threadResult.counters = getAndClearThreadCounters();
                threadResult.gs = move(lgs);
                resultq->push(move(threadResult), processedByThread);
            }
        });

        tracer->trace("Deep copying global state");
        auto mainTheadGs = cgs->deepCopy();
        tracer->trace("Done deep copying global state");

        for (auto f : mainThreadFiles) {
            trees.emplace_back(indexOne(opts, mainTheadGs, f));
        }

        gs = move(mainTheadGs);

        thread_result threadResult;
        {
            tracer->trace("Collecting results from indexing threads");
            for (auto result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS); !result.done();
                 result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS)) {
                if (result.gotItem()) {
                    tracer->trace("Building global substitution");
                    core::GlobalSubstitution substitution(*threadResult.gs, *gs, cgs.get());
                    tracer->trace("Consuming counters");
                    counterConsume(move(threadResult.counters));
                    core::MutableContext ctx(*gs, core::Symbols::root());
                    tracer->trace("Running tree substitution");
                    for (auto &tree : threadResult.trees) {
                        trees.emplace_back(ast::Substitute::run(ctx, substitution, move(tree)));
                    }
                    tracer->trace("Tree substitution done");
                }
                gs->flushErrors();
                indexingProgress.reportProgress(fileq->doneEstimate());
            }
            tracer->trace("Done collecting results from indexing threads");
        }
    }

    ENFORCE(mainThreadFiles.size() + frs.size() == trees.size());
    {
        core::UnfreezeNameTable nameTableAccess(*gs);     // creates names from config
        core::UnfreezeSymbolTable symbolTableAccess(*gs); // creates methods for them
        ProgressIndicator namingProgress(opts.showProgress, "Configatron", 1);
        namer::configatron::fillInFromFileSystem(*gs, opts.configatronDirs, opts.configatronFiles);
    }

    {
        ProgressIndicator namingProgress(opts.showProgress, "Naming", frs.size());

        Timer timeit(console_err, "naming");
        for (auto &tree : trees) {
            auto file = tree->loc.file;
            try {
                unique_ptr<ast::Expression> ast;
                {
                    core::MutableContext ctx(*gs, core::Symbols::root());
                    tracer->trace("Naming: {}", file.data(*gs).path());
                    core::ErrorRegion errs(*gs, file);
                    core::UnfreezeNameTable nameTableAccess(*gs);     // creates singletons and class names
                    core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters symbols
                    ast = namer::Namer::run(ctx, move(tree));
                }
                result.emplace_back(move(ast));
                gs->flushErrors();
                namingProgress.reportProgress(result.size());
            } catch (...) {
                returnCode = 13;
                if (auto e = gs->beginError(ruby_typer::core::Loc::none(file), core::errors::Internal::InternalError)) {
                    e.setHeader("Exception naming file: `{}` (backtrace is above)", file.data(*gs).path());
                }
            }
        }
    }
    return result;
}

unique_ptr<ast::Expression> typecheckFile(core::Context ctx, unique_ptr<ast::Expression> resolved, Options opts) {
    unique_ptr<ast::Expression> result;
    core::FileRef f = resolved->loc.file;
    if (opts.stopAfterPhase == Phase::NAMER) {
        return make_unique<ast::EmptyTree>(core::Loc::none(f));
    }

    try {
        if (opts.print.CFG || opts.print.CFGRaw) {
            cout << "digraph \"" << File::getFileName(f.data(ctx).path()) << "\"{" << '\n';
        }
        CFG_Collector_and_Typer collector(opts);
        {
            tracer->trace("CFG+Infer: {}", f.data(ctx).path());
            core::ErrorRegion errs(ctx, f);
            result = ast::TreeMap::apply(ctx, collector, move(resolved));
        }
        if (wantTypedSource(opts, ctx, f)) {
            cout << ctx.state.showAnnotatedSource(f);
        }
        if (opts.print.CFG || opts.print.CFGRaw) {
            cout << "}" << '\n' << '\n';
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
    CounterState counters;
};

// If ever given a result type, it should be something along the lines of
// vector<pair<vector<unique_ptr<ast::Expression>>, unique_ptr<core::GlobalState>>>
void typecheck(shared_ptr<core::GlobalState> &gs, vector<unique_ptr<ast::Expression>> what, const Options &opts,
               WorkerPool &workers) {
    vector<vector<unique_ptr<ast::Expression>>> typecheck_result;

    try {
        core::MutableContext ctx(*gs, core::Symbols::root());
        ProgressIndicator namingProgress(opts.showProgress, "Resolving", 1);
        {
            Timer timeit(console_err, "Resolving");
            tracer->trace("Resolving (global pass)...");
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
    shared_ptr<ConcurrentBoundedQueue<unique_ptr<ast::Expression>>> fileq;
    shared_ptr<BlockingBoundedQueue<typecheck_thread_result>> resultq;

    {
        Timer timeit(console_err, "creating typecheck queues");
        fileq = make_shared<ConcurrentBoundedQueue<unique_ptr<ast::Expression>>>(what.size());
        resultq = make_shared<BlockingBoundedQueue<typecheck_thread_result>>(what.size());
    }

    core::Context ctx(*gs, core::Symbols::root());
    for (auto &resolved : what) {
        tracer->trace("enqueue-typer {}", resolved->loc.file.data(*gs).path());
        fileq->push(move(resolved), 1);
    }

    {
        ProgressIndicator cfgInferProgress(opts.showProgress, "CFG+Inference", what.size());
        workers.multiplexJob([ctx, opts, fileq, resultq]() {
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
                            console_err->error("Exception typing file: {} (backtrace is above)", file.data(ctx).path());
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
    if (opts.print.NameTableFull) {
        cout << gs->toString(true) << '\n';
    }
    return;
}

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

/** read @file arguments and put them explicitly
 *  Steals the original arguments and will put them back on destruction.
 * */
class FileFlatMapper {
    int origArgc;
    char **origArgv;
    int &argc;
    char **&argv;
    std::vector<char *> args;

public:
    FileFlatMapper(int &argc, char **&argv) : origArgc(argc), origArgv(argv), argc(argc), argv(argv) {
        for (int i = 0; i < argc; i++) {
            if (argv[i][0] == '@') {
                try {
                    string argsP = File::read(argv[i] + 1);
                    for (string arg : split(argsP, '\n')) {
                        char *c_arg = (char *)malloc(arg.size() + 1);
                        memcpy(c_arg, arg.c_str(), arg.size() + 1);
                        args.push_back(c_arg);
                    }
                } catch (FileNotFoundException e) {
                    console_err->error("File Not Found: {}", argv[i]);
                    returnCode = 11;
                    continue;
                }
            } else {
                int length = strlen(argv[i]);
                char *c_arg = (char *)malloc(length + 1);
                memcpy(c_arg, argv[i], length);
                c_arg[length] = '\0';
                args.push_back(c_arg);
            }
        }
        argc = args.size();
        argv = args.data();
    }

    ~FileFlatMapper() {
        argc = origArgc;
        argv = origArgv;
        for (char *c : args) {
            free(c);
        }
    }
};

cxxopts::Options buildOptions() {
    cxxopts::Options options("ruby_typer", "Typechecker for Ruby");

    // Common user options in order of use
    options.add_options()("e", "Parse an inline ruby string", cxxopts::value<string>(), "string");
    options.add_options()("files", "Input files", cxxopts::value<vector<string>>());
    options.add_options()("q,quiet", "Silence all non-critical errors");
    options.add_options()("P,progress", "Draw progressbar");
    options.add_options()("v,verbose", "Verbosity level [0-3]");
    options.add_options()("h,help", "Show long help");
    options.add_options()("version", "Show version");
    options.add_options()("color", "Use color output", cxxopts::value<string>()->default_value("auto"),
                          "{always,never,[auto]}");

    stringstream all_prints;
    all_prints << "Print: [";
    for (auto &pr : print_options) {
        if (&pr != &print_options[0]) {
            all_prints << ", ";
        }
        all_prints << pr.option;
    }
    all_prints << "]";

    stringstream all_stop_after;
    all_stop_after << "Stop After: [";
    for (auto &pr : stop_after_options) {
        if (&pr != &stop_after_options[0]) {
            all_stop_after << ", ";
        }
        all_stop_after << pr.option;
    }
    all_stop_after << "]";

    // Advanced options
    options.add_options("advanced")("configatron-dir", "Path to configatron yaml folders",
                                    cxxopts::value<vector<string>>(), "path");
    options.add_options("advanced")("configatron-file", "Path to configatron yaml files",
                                    cxxopts::value<vector<string>>(), "path");

    // Developer options
    options.add_options("dev")("p,print", all_prints.str(), cxxopts::value<vector<string>>(), "type");
    options.add_options("dev")("stop-after", all_stop_after.str(),
                               cxxopts::value<string>()->default_value("inferencer"), "phase");
    options.add_options("dev")("no-stdlib", "Do not load included rbi files for stdlib");
    options.add_options("dev")("typed", "Run full checks and report errors on all/no/only @typed code",
                               cxxopts::value<string>()->default_value("auto"), "{always,never,[auto]}");
    options.add_options("dev")("store-state", "Store state into file", cxxopts::value<string>(), "file");
    options.add_options("dev")("typed-source", "Print the specified file with type annotations",
                               cxxopts::value<string>(), "file");
    options.add_options("dev")("suppress-non-critical", "Exit 0 unless there was a critical error");
    options.add_options("dev")("trace", "Trace phases");

    int defaultThreads = std::thread::hardware_concurrency();
    if (defaultThreads == 0) {
        defaultThreads = 2;
    }

    options.add_options("dev")("max-threads", "Set number of threads",
                               cxxopts::value<int>()->default_value(to_string(defaultThreads)), "int");
    options.add_options("dev")("counters", "Print internal counters");
    options.add_options("dev")("statsd-host", "StatsD sever hostname", cxxopts::value<string>(), "host");
    options.add_options("dev")("statsd-prefix", "StatsD prefix",
                               cxxopts::value<string>()->default_value("ruby_typer.unknown"), "prefix");
    options.add_options("dev")("statsd-port", "StatsD sever port", cxxopts::value<int>()->default_value("8200"),
                               "port");
    options.add_options("dev")("metrics-file", "File to export metrics to", cxxopts::value<string>(), "file");
    options.add_options("dev")("metrics-prefix", "Prefix to use in metrics",
                               cxxopts::value<string>()->default_value("ruby_typer.unknown."), "file");
    options.add_options("dev")("metrics-branch", "Branch to report in metrics export",
                               cxxopts::value<string>()->default_value("none"), "branch");
    options.add_options("dev")("metrics-sha", "Sha1 to report in metrics export",
                               cxxopts::value<string>()->default_value("none"), "sha1");
    options.add_options("dev")("metrics-repo", "Repo to report in metrics export",
                               cxxopts::value<string>()->default_value("none"), "repo");

    // Positional params
    options.parse_positional("files");
    options.positional_help("<file1.rb> <file2.rb> ...");
    return options;
}

void createInitialGlobalState(std::shared_ptr<core::GlobalState> &gs, const Options &options) {
    if (options.noStdlib) {
        gs->initEmpty();
        return;
    }

    const u1 *const nameTablePayload = getNameTablePayload;
    if (nameTablePayload == nullptr) {
        gs->initEmpty();
        Timer timeit(console_err, "Indexed payload");

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
        WorkerPool workers(emptyOpts.threads, tracer);
        vector<std::string> empty;
        auto oldSilence = gs->silenceErrors;
        gs->silenceErrors = true;

        typecheck(gs, index(gs, empty, payloadFiles, emptyOpts, workers), emptyOpts, workers); // result is thrown away
        gs->silenceErrors = oldSilence;
    } else {
        Timer timeit(console_err, "Read serialized payload");
        core::serialize::GlobalStateSerializer::load(*gs, nameTablePayload);
    }
}

bool extractPrinters(cxxopts::Options &opts, Printers &print) {
    vector<string> printOpts = opts["print"].as<vector<string>>();
    for (auto opt : printOpts) {
        bool found = false;
        for (auto &known : print_options) {
            if (known.option == opt) {
                print.*(known.flag) = true;
                found = true;
                break;
            }
        }
        if (!found) {
            stringstream all;
            for (auto &known : print_options) {
                if (&known != &print_options[0]) {
                    all << ", ";
                }
                all << known.option;
            }
            console_err->error("Unknown --print option: {}\nValid values: {}", opt, all.str());
            return false;
        }
    }
    return true;
}

Phase extractStopAfter(cxxopts::Options &opts) {
    string opt = opts["stop-after"].as<string>();
    for (auto &known : stop_after_options) {
        if (known.option == opt) {
            return known.flag;
        }
    }
    stringstream all;
    for (auto &known : stop_after_options) {
        if (&known != &stop_after_options[0]) {
            all << ", ";
        }
        all << known.option;
    }
    console_err->error("Unknown --stop-after option: {}\nValid values: {}", opt, all.str());
    return Phase::INIT;
}

int realmain(int argc, char **argv) {
    console = spd::details::registry::instance().create("console", stderr_color_sink);
    console->set_pattern("%v");
    console_err = spd::stderr_color_mt("");
    console_err->set_pattern("%v");

    FileFlatMapper flatMapper(argc, argv);
    if (returnCode != 0) {
        return returnCode;
    }

    cxxopts::Options options = buildOptions();

    try {
        options.parse(argc, argv);
    } catch (cxxopts::OptionParseException &e) {
        console->info("{}\n\n{}", e.what(), options.help({"", "advanced", "dev"}));
        return 1;
    }

    vector<string> files = options["files"].as<vector<string>>();

    Options opts;
    if (!extractPrinters(options, opts.print)) {
        return 1;
    }
    opts.stopAfterPhase = extractStopAfter(options);
    opts.noStdlib = options["no-stdlib"].as<bool>();

    opts.threads = min(options["max-threads"].as<int>(), int(files.size() / 2));
    if (opts.threads == 0) {
        opts.threads = 1;
    }

    if (files.size() > 10) {
        //        spd::set_async_mode(1024); // makes logging asyncronous but adds 60ms to startup time.
    }

    if (options["h"].as<bool>()) {
        console->info("{}", options.help({"", "advanced", "dev"}));
        return 0;
    }
    if (options["version"].as<bool>()) {
        console->info("Ruby Typer{}{} git {}{} built on {}", Version::version, Version::codename,
                      Version::build_scm_revision, Version::build_scm_status, Version::build_timestamp_string);
        return 0;
    }
    if (options.count("e") == 0 && files.empty()) {
        console->info("You must pass either `-e` or at least one ruby file.\n\n{}", options.help());
        return 1;
    }

    switch (options.count("v")) {
        case 0:
            break;
        case 1:
            spd::set_level(spd::level::debug);
            console->debug("Debug logging enabled");
            break;
        default:
            spd::set_level(spd::level::trace);
            console->trace("Trace logging enabled");
            break;
    }

    if (options.count("q") != 0) {
        console->set_level(spd::level::critical);
    }

    if (options.count("trace") != 0) {
        tracer->set_level(spd::level::trace);
    }

    string typed = options["typed"].as<string>();
    opts.forceTyped = typed == "always";
    opts.forceUntyped = typed == "never";
    opts.showProgress = options.count("P") != 0;
    opts.configatronDirs = options["configatron-dir"].as<vector<string>>();
    opts.configatronFiles = options["configatron-file"].as<vector<string>>();
    opts.storeState = options.count("store-state") != 0;
    if (typed != "always" && typed != "never" && typed != "auto") {
        console->error("Invalid value for `--typed`: {}", typed);
    }
    opts.typedSource = options["typed-source"].as<string>();
    if (!opts.typedSource.empty()) {
        if (opts.print.TypedSource) {
            console_err->error("`--typed-source " + opts.typedSource +
                               "` and `-p typed-source` are incompatible. Either print out one file or all files.");
            return 1;
        }
        auto found = any_of(files.begin(), files.end(),
                            [&](string &path) { return path.find(opts.typedSource) != string::npos; });
        if (!found) {
            console_err->error("`--typed-source " + opts.typedSource + "`: No matching files found.");
            return 1;
        }
    }

    if (options["color"].as<string>() == "auto") {
        if (rang::rang_implementation::isTerminal(std::cerr.rdbuf())) {
            core::ErrorColors::enableColors();
        }
    } else if (options["color"].as<string>() == "always") {
        core::ErrorColors::enableColors();
    } else if (options["color"].as<string>() == "never") {
        core::ErrorColors::disableColors();
    }

    WorkerPool workers(opts.threads, tracer);
    shared_ptr<core::GlobalState> gs = make_shared<core::GlobalState>((std::make_shared<core::ErrorQueue>(*console)));

    tracer->trace("building initial global state");
    createInitialGlobalState(gs, opts);
    if (options.count("q") != 0) {
        gs->silenceErrors = true;
    }
    tracer->trace("done building initial global state");

    Timer timeall(console_err, "Done in");
    vector<core::FileRef> inputFiles;
    tracer->trace("Files: ");
    {
        Timer timeit(console_err, "reading files");
        core::UnfreezeFileTable fileTableAccess(*gs);
        if (options.count("e") != 0) {
            string src = options["e"].as<string>();
            counterAdd("types.input.bytes", src.size());
            counterInc("types.input.lines");
            counterInc("types.input.files");
            auto file = gs->enterFile(string("-e"), src + "\n");
            inputFiles.push_back(file);
            if (opts.forceUntyped) {
                console->error("`-e` implies `--typed always` and you passed `--typed never`");
                return 1;
            }
            file.data(*gs).source_type = core::File::Typed;
        }
    }

    vector<unique_ptr<ast::Expression>> indexed;
    {
        Timer timeit(console_err, "index");
        indexed = index(gs, files, inputFiles, opts, workers);
    }

    {
        Timer timeit(console_err, "typecheck");
        typecheck(gs, move(indexed), opts, workers);
    }
    tracer->trace("ruby-typer done");

    if (options.count("store-state") != 0) {
        string outfile = options["store-state"].as<string>();
        File::write(outfile.c_str(), core::serialize::GlobalStateSerializer::store(*gs));
    }

    if (options.count("counters") != 0) {
        console_err->warn("" + getCounterStatistics());
    }

    if (options.count("statsd-host") != 0) {
        submitCountersToStatsd(options["statsd-host"].as<string>(), options["statsd-port"].as<int>(),
                               options["statsd-prefix"].as<string>() + ".counters");
    }

    if (options.count("metrics-file") != 0) {
        string status;
        if (gs->hadCriticalError()) {
            status = "Error";
        } else if (returnCode != 0) {
            status = "Failure";
        } else {
            status = "Success";
        }

        storeCountersToProtoFile(options["metrics-file"].as<string>(), options["metrics-prefix"].as<string>(),
                                 options["metrics-repo"].as<string>(), options["metrics-branch"].as<string>(),
                                 options["metrics-sha"].as<string>(), status);
    }

    // je_malloc_stats_print(nullptr, nullptr, nullptr); // uncomment this to print jemalloc statistics
    if (gs->hadCriticalError()) {
        returnCode = 10;
    } else if (returnCode == 0 && gs->totalErrors() > 0 && !options.count("suppress-non-critical")) {
        returnCode = 1;
    }

    return returnCode;
}

} // namespace ruby_typer
