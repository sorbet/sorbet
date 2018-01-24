#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/substitute/substitute.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "core/Files.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "infer/infer.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "payload/text/text.h"
#include "progressbar/progressbar.h"
#include "progressbar/statusbar.h"
#include "resolver/resolver.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
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
    bool Desugared = false;
    bool DesugaredRaw = false;
    bool NameTree = false;
    bool NameTreeRaw = false;
    bool NameTable = false;
    bool NameTableFull = false;
    bool CFG = false;
    bool CFGRaw = false;
    bool TypedSource = false;
};

struct Options {
    Printers print;
    bool showProgress = false;
    bool noStdlib = false;
    int threads = 0;
    string typedSource = "";
};

shared_ptr<spd::logger> console_err;
shared_ptr<spd::logger> tracer;
shared_ptr<spd::logger> console;

int returnCode = 0;

struct {
    string option;
    bool Printers::*flag;
} print_options[] = {
    {"parse-tree", &Printers::ParseTree},
    {"ast", &Printers::Desugared},
    {"ast-raw", &Printers::DesugaredRaw},
    {"name-table", &Printers::NameTable},
    {"name-table-full", &Printers::NameTableFull},
    {"name-tree", &Printers::NameTree},
    {"name-tree-raw", &Printers::NameTreeRaw},
    {"cfg", &Printers::CFG},
    {"cfg-raw", &Printers::CFGRaw},
    {"typed-source", &Printers::TypedSource},
};

long timespec_delta(struct timespec *start, struct timespec *stop) {
    return (stop->tv_sec - start->tv_sec) * 1000000000 + stop->tv_nsec - start->tv_nsec;
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
    const Printers &print;

public:
    CFG_Collector_and_Typer(const Printers &print) : print(print){};

    ast::MethodDef *preTransformMethodDef(core::Context ctx, ast::MethodDef *m) {
        if (m->loc.file.file(ctx).source_type == core::File::Untyped) {
            return m;
        }

        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);
        if (print.CFGRaw || print.TypedSource) {
            cfg::CFGBuilder::addDebugEnvironment(ctx.withOwner(m->symbol), cfg);
        }
        infer::Inference::run(ctx.withOwner(m->symbol), cfg);
        if (print.CFG || print.CFGRaw) {
            cout << cfg->toString(ctx) << endl << endl;
        }
        if (print.TypedSource) {
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

unique_ptr<ast::Expression> indexOne(const Printers &print, core::GlobalState &lgs, core::FileRef file,
                                     bool silenceErrors = false) {
    std::unique_ptr<parser::Node> nodes;
    {
        tracer->trace("Parsing: {}", file.file(lgs).path());
        core::ErrorRegion errs(lgs, silenceErrors);
        core::UnfreezeNameTable nameTableAccess(lgs); // enters strings from source code as names
        nodes = parser::Parser::run(lgs, file);
    }
    if (print.ParseTree) {
        cout << nodes->toString(lgs, 0) << endl;
    }

    core::Context context(lgs, lgs.defn_root());
    std::unique_ptr<ast::Expression> ast;
    {
        tracer->trace("Desugaring: {}", file.file(lgs).path());
        core::ErrorRegion errs(lgs, silenceErrors);
        core::UnfreezeNameTable nameTableAccess(lgs); // creates temporaries during desugaring
        ast = ast::desugar::node2Tree(context, move(nodes));
    }
    if (print.Desugared) {
        cout << ast->toString(lgs, 0) << endl;
    }

    if (print.DesugaredRaw) {
        cout << ast->showRaw(lgs) << endl;
    }
    return ast;
}

vector<unique_ptr<ast::Expression>> index(core::GlobalState &gs, std::vector<core::FileRef> frs, const Options &opts,
                                          bool silenceErrors = false) {
    vector<unique_ptr<ast::Expression>> result;
    vector<unique_ptr<ast::Expression>> empty;

    unique_ptr<progressbar> progress;

    if (opts.showProgress) {
        progress.reset(progressbar_new("Indexing", frs.size()));
    }

    vector<unique_ptr<ast::Expression>> trees;

    ThreadQueue<core::FileRef> fileq;
    ThreadQueue<thread_result> resultq;

    gs.sanityCheck();
    const auto cgs = gs.deepCopy();
    {
        ENFORCE(opts.threads > 0);
        vector<unique_ptr<Joinable>> threads;
        for (int i = 0; i < opts.threads; ++i) {
            threads.emplace_back(runInAThread([&cgs, &opts, &fileq, &resultq, silenceErrors]() {
                auto lgs = cgs->deepCopy();
                thread_result result;
                core::FileRef file;

                {
                    core::ErrorRegion errs(*lgs, silenceErrors);

                    while (fileq.pop(&file)) {
                        file = core::FileRef(*lgs, file.id());
                        try {
                            result.trees.emplace_back(indexOne(opts.print, *lgs, file, silenceErrors));
                        } catch (...) {
                            console_err->error("Exception parsing file: {} (backtrace is above)",
                                               file.file(*lgs).path());
                        }
                    }
                }

                result.counters = getAndClearThreadCounters();
                result.gs = move(lgs);
                resultq.push(move(result));
            }));
        }

        for (auto f : frs) {
            tracer->trace("enqueue: {}", f.file(gs).path());
            fileq.push(f);
        }
        fileq.close();

        thread_result result;
        for (int i = 0; i < opts.threads; ++i) {
            resultq.pop(&result);
            core::GlobalSubstitution substitution(*result.gs, gs);
            counterConsume(move(result.counters));
            core::Context context(gs, gs.defn_root());
            for (auto &tree : result.trees) {
                trees.emplace_back(ast::Substitute::run(context, substitution, move(tree)));
            }
        }
    }

    ENFORCE(frs.size() == trees.size());

    {
        Timer timeit(console_err, "naming");

        for (auto &tree : trees) {
            auto file = tree->loc.file;
            try {
                unique_ptr<ast::Expression> ast;
                {
                    core::Context context(gs, gs.defn_root());
                    tracer->trace("Naming: {}", file.file(gs).path());
                    core::ErrorRegion errs(gs, silenceErrors);
                    core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
                    core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
                    ast = namer::Namer::run(context, move(tree));
                }
                result.emplace_back(move(ast));
                if (opts.showProgress) {
                    progressbar_inc(progress.get());
                }
            } catch (...) {
                console_err->error("Exception on file: {} (backtrace is above)", file.file(gs).path());
            }
        }
        if (opts.showProgress) {
            progressbar_finish(progress.get());
        }
    }
    return result;
}

unique_ptr<ast::Expression> typecheckFile(core::GlobalState &gs, unique_ptr<ast::Expression> resolved, Options opts,
                                          progressbar *progress, bool silenceErrors = false) {
    unique_ptr<ast::Expression> result;
    core::FileRef f = resolved->loc.file;
    bool forceTypedSource = !opts.typedSource.empty() && f.file(gs).path().find(opts.typedSource) != std::string::npos;
    if (forceTypedSource) {
        ENFORCE(!opts.print.TypedSource);
        opts.print.TypedSource = true;
        f.file(gs).source_type = ruby_typer::core::File::Typed;
    }
    try {
        core::Context context(gs, gs.defn_root());
        if (opts.print.CFG || opts.print.CFGRaw) {
            cout << "digraph \"" << File::getFileName(f.file(gs).path()) << "\"{" << endl;
        }
        CFG_Collector_and_Typer collector(opts.print);
        {
            tracer->trace("CFG+Infer: {}", f.file(gs).path());
            core::ErrorRegion errs(gs, silenceErrors);
            core::UnfreezeNameTable nameTableAccess(gs); // creates names for temporaries in CFG
            result = ast::TreeMap<CFG_Collector_and_Typer>::apply(context, collector, move(resolved));
        }
        if (opts.print.TypedSource) {
            cout << gs.showAnnotatedSource(f);
        }
        if (opts.print.CFG || opts.print.CFGRaw) {
            cout << "}" << endl << endl;
        }
        if (opts.showProgress) {
            progressbar_inc(progress);
        }
    } catch (...) {
        console_err->error("Exception resolving: {} (backtrace is above)", f.file(gs).path());
    }
    if (forceTypedSource) {
        opts.print.TypedSource = false;
    }
    return result;
}

vector<unique_ptr<ast::Expression>> typecheck(core::GlobalState &gs, vector<unique_ptr<ast::Expression>> what,
                                              const Options &opts, bool silenceErrors = false) {
    vector<unique_ptr<ast::Expression>> result;
    unique_ptr<progressbar> progress;
    unique_ptr<statusbar> status;

    if (opts.showProgress) {
        status.reset(statusbar_new("Resolving"));
    }
    try {
        core::Context context(gs, gs.defn_root());
        {
            Timer timeit(console_err, "Resolving");
            tracer->trace("Resolving (global pass)...");
            core::ErrorRegion errs(gs, silenceErrors);
            core::UnfreezeNameTable nameTableAccess(gs);     // Resolver::defineAttr
            core::UnfreezeSymbolTable symbolTableAccess(gs); // enters stubs
            what = resolver::Resolver::run(context, move(what));
        }
    } catch (...) {
        console_err->error("Exception resolving (backtrace is above)");
    }
    if (opts.showProgress) {
        statusbar_finish(status.get());
    }

    for (auto &resolved : what) {
        if (opts.print.NameTree) {
            cout << resolved->toString(gs, 0) << endl;
        }
        if (opts.print.NameTreeRaw) {
            cout << resolved->showRaw(gs) << endl;
        }
    }

    if (opts.showProgress) {
        progress.reset(progressbar_new("CFG+Typechecking", what.size()));
    }

    for (auto &resolved : what) {
        result.emplace_back(typecheckFile(gs, move(resolved), opts, progress.get(), silenceErrors));
    }
    if (opts.showProgress) {
        progressbar_finish(progress.get());
    }
    if (!opts.typedSource.empty()) {
        stringstream files;
        for (auto &cfg : result) {
            files << "  " << cfg->loc.file.file(gs).path() << endl;
        }
        console_err->error("`--typed-source " + opts.typedSource + "` not found in input list of:\n" + files.str());
    }

    if (opts.print.NameTable) {
        cout << gs.toString() << endl;
    }
    if (opts.print.NameTableFull) {
        cout << gs.toString(true) << endl;
    }
    return result;
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

    stringstream all_prints;
    all_prints << "Print: [";
    for (auto &pr : print_options) {
        if (&pr != &print_options[0]) {
            all_prints << ", ";
        }
        all_prints << pr.option;
    }
    all_prints << "]";

    // Developer options
    options.add_options("dev")("p,print", all_prints.str(), cxxopts::value<vector<string>>(), "type");
    options.add_options("dev")("no-stdlib", "Do not load included rbi files for stdlib");
    options.add_options("dev")("typed", "Run full checks and report errors on all/no/only @typed code",
                               cxxopts::value<string>()->default_value("auto"), "{always,never,[auto]}");
    options.add_options("dev")("store-state", "Store state into file", cxxopts::value<string>(), "file");
    options.add_options("dev")("typed-source", "Print the specified file with type annotations",
                               cxxopts::value<string>(), "file");
    options.add_options("dev")("trace", "Trace phases");
    options.add_options("dev")("threads", "Set number of threads", cxxopts::value<int>(), "int");
    options.add_options("dev")("counters", "Print internal counters");
    options.add_options("dev")("statsd-host", "StatsD sever hostname", cxxopts::value<string>(), "host");
    options.add_options("dev")("statsd-prefix", "StatsD prefix", cxxopts::value<string>(), "prefix");
    options.add_options("dev")("statsd-port", "StatsD sever port", cxxopts::value<int>()->default_value("8200"),
                               "port");

    // Positional params
    options.parse_positional("files");
    options.positional_help("<file1.rb> <file2.rb> ...");
    return options;
}

void createInitialGlobalState(core::GlobalState &gs, const Options &options) {
    if (options.noStdlib) {
        gs.initEmpty();
        return;
    }

    const u4 *const nameTablePayload = getNameTablePayload;
    if (nameTablePayload == nullptr) {
        gs.initEmpty();
        Timer timeit(console_err, "Indexed payload");

        vector<core::FileRef> payloadFiles;
        {
            core::UnfreezeFileTable fileTableAccess(gs);
            for (auto &p : rbi::all()) {
                payloadFiles.push_back(gs.enterFile(p.first, p.second));
            }
        }
        Options emptyOpts;
        emptyOpts.threads = 1;

        typecheck(gs, index(gs, payloadFiles, emptyOpts, true), emptyOpts, true); // result is thrown away
    } else {
        Timer timeit(console_err, "Read serialized payload");
        core::serialize::GlobalStateSerializer::load(gs, nameTablePayload);
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

int realmain(int argc, char **argv) {
    FileFlatMapper flatMapper(argc, argv);

    auto color_sink = make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>();
    color_sink->set_color(spd::level::info, color_sink->white);
    color_sink->set_color(spd::level::debug, color_sink->magenta);
    tracer = spd::details::registry::instance().create("tracer", color_sink);
    tracer->set_pattern("[T%t][%Y-%m-%dT%T.%f] %v");

    console = spd::details::registry::instance().create("console", color_sink);
    console->set_pattern("%v");
    console_err = spd::stderr_color_mt("");
    console_err->set_pattern("%v");

    cxxopts::Options options = buildOptions();

    try {
        options.parse(argc, argv);
    } catch (cxxopts::OptionParseException &e) {
        console->info("{}\n\n{}", e.what(), options.help({"", "dev"}));
        return 1;
    }

    vector<string> files = options["files"].as<vector<string>>();

    Options opts;
    if (!extractPrinters(options, opts.print)) {
        return 1;
    }
    opts.noStdlib = options["no-stdlib"].as<bool>();
    opts.threads = options["threads"].as<int>();
    if (opts.threads <= 0) {
        int ncpu = std::thread::hardware_concurrency();
        if (ncpu == 0) {
            ncpu = 4;
        }
        opts.threads = min(ncpu, int(files.size() / 2));
        if (opts.threads == 0) {
            opts.threads = 1;
        }
    }

    if (files.size() > 10) {
        //        spd::set_async_mode(1024); // makes logging asyncronous but adds 60ms to startup time.
    }

    if (options["h"].as<bool>()) {
        console->info("{}", options.help({"", "dev"}));
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

    console_err->debug("Using {} threads", opts.threads);

    if (options.count("q") != 0) {
        console->set_level(spd::level::critical);
    }

    if (options.count("P") != 0) {
        opts.showProgress = true;
    }

    if (options.count("trace") != 0) {
        tracer->set_level(spd::level::trace);
    } else {
        tracer->set_level(spd::level::off);
    }

    string typed = options["typed"].as<string>();
    bool forceTyped = typed == "always";
    opts.typedSource = options["typed-source"].as<string>();
    if (opts.typedSource != "" && opts.print.TypedSource) {
        console_err->error("`--typed-source " + opts.typedSource +
                           "` and `-p typed-source` are incompatible. Either print out one file or all files.");
        return 1;
    }

    core::GlobalState gs(*console);
    createInitialGlobalState(gs, opts);

    Timer timeall(console_err, "Done in");
    vector<core::FileRef> inputFiles;
    tracer->trace("Files: ");
    {
        Timer timeit(console_err, "reading files");
        core::UnfreezeFileTable fileTableAccess(gs);
        for (auto &fileName : files) {
            string src;
            try {
                src = File::read(fileName.c_str());
            } catch (FileNotFoundException e) {
                console->error("File Not Found: {}", fileName);
                returnCode = 11;
                continue;
            }
            counterAdd("types.input.bytes", src.size());
            counterAdd("types.input.lines", count(src.begin(), src.end(), '\n'));
            counterInc("types.input.files");
            inputFiles.push_back(gs.enterFile(fileName, src));
            tracer->trace("{}", fileName);
        }
        if (options.count("e") != 0) {
            string src = options["e"].as<string>();
            counterAdd("types.input.bytes", src.size());
            counterInc("types.input.lines");
            counterInc("types.input.files");
            inputFiles.push_back(gs.enterFile(string("-e"), src));
            if (typed == "never") {
                console->error("`-e` implies `--typed always` and you passed `--typed never`");
                return 1;
            }
            forceTyped = true;
        }
        if (forceTyped) {
            for (auto &f : inputFiles) {
                f.file(gs).source_type = core::File::Typed;
            }
        } else if (typed == "never") {
            for (auto &f : inputFiles) {
                f.file(gs).source_type = core::File::Untyped;
            }
        } else if (typed == "auto") {
            // Use the annotation in the file
        } else {
            console->error("Invalid valud for `--typed`: {}", typed);
        }
    }

    vector<unique_ptr<ast::Expression>> indexed;
    {
        Timer timeit(console_err, "index");
        indexed = index(gs, inputFiles, opts);
    }

    {
        Timer timeit(console_err, "typecheck");
        typecheck(gs, move(indexed), opts);
    }
    tracer->trace("done");

    if (options.count("store-state") != 0) {
        string outfile = options["store-state"].as<string>();
        File::write(outfile.c_str(), core::serialize::GlobalStateSerializer::store(gs));
    }

    if (options.count("counters") != 0) {
        for (auto e : gs.errorHistogram()) {
            histogramAdd("error", e.first, e.second);
        }
        console_err->warn("" + getCounterStatistics());
    }

    if (options.count("statsd-host") != 0) {
        submitCountersToStatsd(options["statsd-host"].as<string>(), options["statsd-port"].as<int>(),
                               options["statsd-prefix"].as<string>() + ".counters");
    }

    return gs.hadCriticalError() ? 10 : returnCode;
};

} // namespace ruby_typer
