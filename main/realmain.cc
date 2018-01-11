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

class CFG_Collector_and_Typer {
    bool printCFGs;
    bool printCFGRaw;

public:
    CFG_Collector_and_Typer(bool printCFGs, bool printCFGRaw) : printCFGs(printCFGs), printCFGRaw(printCFGRaw){};

    ruby_typer::ast::MethodDef *preTransformMethodDef(ruby_typer::core::Context ctx, ruby_typer::ast::MethodDef *m) {
        if (m->loc.file.file(ctx).source_type == ruby_typer::core::File::Untyped) {
            return m;
        }

        auto cfg = ruby_typer::cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);
        if (printCFGRaw) {
            ruby_typer::cfg::CFGBuilder::addDebugEnvironment(ctx.withOwner(m->symbol), cfg);
        }
        ruby_typer::infer::Inference::run(ctx.withOwner(m->symbol), cfg);
        if (printCFGs) {
            cout << cfg->toString(ctx) << endl << endl;
        }
        return m;
    }
};

static bool removeOption(vector<string> &prints, string option) {
    auto it = find(prints.begin(), prints.end(), option);
    if (it != prints.end()) {
        prints.erase(it);
        return true;
    } else {
        return false;
    }
}
shared_ptr<spd::logger> console_err;
shared_ptr<spd::logger> tracer;
shared_ptr<spd::logger> console;
int returnCode = 0;
bool showProgress = false;

struct thread_result {
    unique_ptr<ruby_typer::core::GlobalState> gs;
    ruby_typer::CounterState counters;
    vector<unique_ptr<ruby_typer::ast::Expression>> trees;
};

vector<unique_ptr<ruby_typer::ast::Expression>> index(ruby_typer::core::GlobalState &gs,
                                                      std::vector<ruby_typer::core::FileRef> frs,
                                                      vector<string> &prints, cxxopts::Options &opts,
                                                      bool silenceErrors = false) {
    vector<unique_ptr<ruby_typer::ast::Expression>> result;
    vector<unique_ptr<ruby_typer::ast::Expression>> empty;
    bool oldErrors = gs.errors.keepErrorsInMemory;
    gs.errors.keepErrorsInMemory = oldErrors || silenceErrors;
    bool printParseTree = removeOption(prints, "parse-tree");
    bool printDesugared = removeOption(prints, "ast");
    bool printDesugaredRaw = removeOption(prints, "ast-raw");

    progressbar *progress;

    if (showProgress) {
        progress = progressbar_new("Indexing", frs.size());
    }

    vector<unique_ptr<ruby_typer::ast::Expression>> trees;

    int nthreads = opts["threads"].as<unsigned int>();
    ENFORCE(nthreads > 0);
    ruby_typer::ThreadQueue<ruby_typer::core::FileRef> fileq;
    ruby_typer::ThreadQueue<thread_result> resultq;

    gs.sanityCheck();
    const auto &cgs = gs;
    {
        vector<unique_ptr<Joinable>> threads;
        for (int i = 0; i < nthreads; ++i) {
            threads.emplace_back(
                runInAThread([&cgs, &fileq, &resultq, printParseTree, printDesugared, printDesugaredRaw]() {
                    auto lgs = cgs.deepCopy();
                    thread_result result;
                    ruby_typer::core::FileRef file;

                    while (fileq.pop(&file)) {
                        file = ruby_typer::core::FileRef(*lgs, file.id());

                        std::unique_ptr<ruby_typer::parser::Node> nodes;
                        {
                            tracer->trace("Parsing: {}", file.file(*lgs).path());
                            ruby_typer::core::UnfreezeNameTable nameTableAccess(
                                *lgs); // enters strings from source code as names
                            nodes = ruby_typer::parser::Parser::run(*lgs, file);
                        }
                        if (printParseTree) {
                            cout << nodes->toString(*lgs, 0) << endl;
                        }

                        ruby_typer::core::Context context(*lgs, lgs->defn_root());
                        std::unique_ptr<ruby_typer::ast::Expression> ast;
                        {
                            tracer->trace("Desugaring: {}", file.file(*lgs).path());
                            ruby_typer::core::UnfreezeNameTable nameTableAccess(
                                *lgs); // creates temporaries during desugaring
                            ast = ruby_typer::ast::desugar::node2Tree(context, nodes);
                        }
                        if (printDesugared) {
                            cout << ast->toString(*lgs, 0) << endl;
                        }

                        if (printDesugaredRaw) {
                            cout << ast->showRaw(*lgs) << endl;
                        }

                        result.trees.emplace_back(move(ast));
                    }

                    result.counters = ruby_typer::getAndClearThreadCounters();
                    result.gs = move(lgs);
                    resultq.push(move(result));
                }));
        }

        for (auto f : frs) {
            tracer->trace("enqueue: {}", f.file(gs).path());
            fileq.push(f);
        }
        fileq.close(); // close does not wait for threads to finish.
    }

    {
        thread_result result;
        while (resultq.try_pop(&result)) {
            ruby_typer::core::GlobalSubstitution substitution(*result.gs, gs);
            ruby_typer::counterConsume(move(result.counters));
            ruby_typer::core::Context context(gs, gs.defn_root());
            for (auto &tree : result.trees) {
                trees.emplace_back(ruby_typer::ast::Substitute::run(context, substitution, move(tree)));
            }
        }
    }

    ENFORCE(frs.size() == trees.size());

    try {
        for (auto &tree : trees) {
            auto file = tree->loc.file;
            try {
                unique_ptr<ruby_typer::ast::Expression> ast;
                {
                    ruby_typer::core::Context context(gs, gs.defn_root());
                    tracer->trace("Naming: {}", file.file(gs).path());
                    ruby_typer::core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
                    ruby_typer::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
                    ast = ruby_typer::namer::Namer::run(context, move(tree));
                }
                result.emplace_back(move(ast));
                if (showProgress) {
                    progressbar_inc(progress);
                }
            } catch (...) {
                console_err->error("Exception on file: {} (backtrace is above)", file.file(gs).path());
            }
        }
    } catch (...) {
        gs.errors.keepErrorsInMemory = oldErrors;
        throw;
    }
    if (silenceErrors) {
        gs.errors.getAndEmptyErrors();
    }
    if (showProgress) {
        progressbar_finish(progress);
    }
    gs.errors.keepErrorsInMemory = oldErrors;
    return result;
}

vector<unique_ptr<ruby_typer::ast::Expression>> typecheck(ruby_typer::core::GlobalState &gs,
                                                          vector<unique_ptr<ruby_typer::ast::Expression>> what,
                                                          vector<string> &prints, cxxopts::Options &opts,
                                                          bool silenceErrors = false) {
    vector<unique_ptr<ruby_typer::ast::Expression>> result;
    bool oldErrors = gs.errors.keepErrorsInMemory;
    gs.errors.keepErrorsInMemory = oldErrors || silenceErrors;
    bool printNameTable = removeOption(prints, "name-table");
    bool printNameTableFull = removeOption(prints, "name-table-full");
    bool printNameTree = removeOption(prints, "name-tree");
    bool printNameTreeRaw = removeOption(prints, "name-tree-raw");
    bool printCFG = removeOption(prints, "cfg");
    bool printCFGRaw = removeOption(prints, "cfg-raw");
    progressbar *progress;
    statusbar *status;

    try {
        if (showProgress) {
            status = statusbar_new("Resolving");
        }
        try {
            ruby_typer::core::Context context(gs, gs.defn_root());
            {
                tracer->trace("Resolving (global pass)...");
                ruby_typer::core::UnfreezeNameTable nameTableAccess(gs);     // Resolver::defineAttr
                ruby_typer::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters stubs
                what = ruby_typer::resolver::Resolver::run(context, move(what));
            }
        } catch (...) {
            console_err->error("Exception resolving (backtrace is above)");
        }
        if (showProgress) {
            statusbar_finish(status);
        }

        for (auto &resolved : what) {
            if (printNameTree) {
                cout << resolved->toString(gs, 0) << endl;
            }
            if (printNameTreeRaw) {
                cout << resolved->showRaw(gs) << endl;
            }
        }

        if (showProgress) {
            progress = progressbar_new("CFG+Typechecking", what.size());
        }

        for (auto &resolved : what) {
            ruby_typer::core::FileRef f = resolved->loc.file;
            try {
                ruby_typer::core::Context context(gs, gs.defn_root());
                if (printCFG || printCFGRaw) {
                    cout << "digraph \"" << ruby_typer::File::getFileName(f.file(gs).path()) << "\"{" << endl;
                }
                CFG_Collector_and_Typer collector(printCFG || printCFGRaw, printCFGRaw);
                {
                    tracer->trace("CFG+Infer: {}", f.file(gs).path());
                    ruby_typer::core::UnfreezeNameTable nameTableAccess(gs); // creates names for temporaries in CFG
                    result.emplace_back(
                        ruby_typer::ast::TreeMap<CFG_Collector_and_Typer>::apply(context, collector, move(resolved)));
                }

                if (printCFG || printCFGRaw) {
                    cout << "}" << endl << endl;
                }
                if (showProgress) {
                    progressbar_inc(progress);
                }
            } catch (...) {
                console_err->error("Exception resolving: {} (backtrace is above)", f.file(gs).path());
            }
        }
        if (showProgress) {
            progressbar_finish(progress);
        }

        if (printNameTable) {
            cout << gs.toString() << endl;
        }
        if (printNameTableFull) {
            cout << gs.toString(true) << endl;
        }
    } catch (...) {
        gs.errors.keepErrorsInMemory = oldErrors;
        throw;
    }
    if (silenceErrors) {
        gs.errors.getAndEmptyErrors();
    }
    gs.errors.keepErrorsInMemory = oldErrors;
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
                    string argsP = ruby_typer::File::read(argv[i] + 1);
                    for (string arg : split(argsP, '\n')) {
                        char *c_arg = (char *)malloc(arg.size() + 1);
                        memcpy(c_arg, arg.c_str(), arg.size() + 1);
                        args.push_back(c_arg);
                    }
                } catch (ruby_typer::FileNotFoundException e) {
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

std::string
    print_options("[parse-tree, ast, ast-raw, name-table, name-table-full, name-tree, name-tree-raw, cfg, cfg-raw]");

cxxopts::Options buildOptions() {
    cxxopts::Options options("ruby_typer", "Typechecker for Ruby");

    // Common user options in order of use
    options.add_options()("e", "Parse an inline ruby string", cxxopts::value<string>(), "string");
    options.add_options()("files", "Input files", cxxopts::value<vector<string>>());
    options.add_options()("q,quiet", "Silence all non-critical errors");
    options.add_options()("P,progress", "Draw progressbar");
    options.add_options()("v,verbose", "Verbosity level [0-3]");
    options.add_options()("h,help", "Show long help");

    auto ncpu = std::thread::hardware_concurrency();
    if (ncpu == 0) {
        ncpu = 4;
    }

    // Developer options
    options.add_options("dev")("p,print", "Print: " + print_options, cxxopts::value<vector<string>>(), "type");
    options.add_options("dev")("no-stdlib", "Do not load included rbi files for stdlib");
    options.add_options("dev")("typed", "Run full checks and report errors on all/no/only @typed code",
                               cxxopts::value<string>()->default_value("auto"), "{always,never,[auto]}");
    options.add_options("dev")("store-state", "Store state into file", cxxopts::value<string>(), "file");
    options.add_options("dev")("trace", "Trace phases");
    options.add_options("dev")("threads", "Set number of threads",
                               cxxopts::value<unsigned int>()->default_value(to_string(ncpu)), "int");
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

void createInitialGlobalState(ruby_typer::core::GlobalState &gs, cxxopts::Options &options) {
    if (options["no-stdlib"].as<bool>()) {
        gs.initEmpty();
        return;
    }

    const ruby_typer::u4 *const nameTablePayload = getNameTablePayload;
    if (nameTablePayload == nullptr) {
        gs.initEmpty();
        clock_t begin = clock();
        vector<ruby_typer::core::FileRef> payloadFiles;
        {
            ruby_typer::core::UnfreezeFileTable fileTableAccess(gs);
            for (auto &p : ruby_typer::rbi::all()) {
                payloadFiles.push_back(gs.enterFile(p.first, p.second));
            }
        }
        vector<string> emptyPrintsPayload;
        cxxopts::Options emptyOpts = buildOptions();
        int argc = 1;
        char **argv = {nullptr};
        // parse() populates default values
        emptyOpts.parse(argc, argv);

        typecheck(gs, index(gs, payloadFiles, emptyPrintsPayload, emptyOpts, true), emptyPrintsPayload, emptyOpts,
                  true); // result is thrown away

        clock_t end = clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;
        console_err->debug("Indexed payload in {} ms\n", elapsed_secs);
    } else {
        clock_t begin = clock();
        ruby_typer::core::serialize::GlobalStateSerializer::load(gs, nameTablePayload);
        clock_t end = clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;
        console_err->debug("Read payload name-table in {} ms\n", elapsed_secs);
    }
}

int realmain(int argc, char **argv) {
    FileFlatMapper flatMapper(argc, argv);

    auto color_sink = make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>();
    color_sink->set_color(spd::level::info, color_sink->white);
    color_sink->set_color(spd::level::debug, color_sink->magenta);
    tracer = spd::details::registry::instance().create("tracer", color_sink);
    tracer->set_pattern("T%t %v");

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
    vector<string> prints = options["print"].as<vector<string>>();
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

    console_err->debug("Using {} threads", options["threads"].as<unsigned int>());

    if (options.count("q") != 0) {
        console->set_level(spd::level::critical);
    }

    if (options.count("P") != 0) {
        showProgress = true;
    }

    if (options.count("trace") != 0) {
        tracer->set_level(spd::level::trace);
    } else {
        tracer->set_level(spd::level::off);
    }

    string typed = options["typed"].as<string>();
    bool forceTyped = typed == "always";
    ruby_typer::core::GlobalState gs(*console);
    createInitialGlobalState(gs, options);

    clock_t begin = clock();
    vector<ruby_typer::core::FileRef> inputFiles;
    tracer->trace("Files: ");
    {
        ruby_typer::core::UnfreezeFileTable fileTableAccess(gs);
        for (auto &fileName : files) {
            string src;
            try {
                src = ruby_typer::File::read(fileName.c_str());
            } catch (ruby_typer::FileNotFoundException e) {
                console->error("File Not Found: {}", fileName);
                returnCode = 11;
                continue;
            }
            ruby_typer::counterAdd("types.input.bytes", src.size());
            ruby_typer::counterAdd("types.input.lines", count(src.begin(), src.end(), '\n'));
            ruby_typer::counterInc("types.input.files");
            inputFiles.push_back(gs.enterFile(fileName, src));
            tracer->trace("{}", fileName);
        }
        if (options.count("e") != 0) {
            string src = options["e"].as<string>();
            ruby_typer::counterAdd("types.input.bytes", src.size());
            ruby_typer::counterInc("types.input.lines");
            ruby_typer::counterInc("types.input.files");
            inputFiles.push_back(gs.enterFile(string("-e"), src));
            if (typed == "never") {
                console->error("`-e` implies `--typed always` and you passed `--typed never`");
                return 1;
            }
            forceTyped = true;
        }
        if (forceTyped) {
            for (auto &f : inputFiles) {
                f.file(gs).source_type = ruby_typer::core::File::Typed;
            }
        } else if (typed == "never") {
            for (auto &f : inputFiles) {
                f.file(gs).source_type = ruby_typer::core::File::Untyped;
            }
        } else if (typed == "auto") {
            // Use the annotation in the file
        } else {
            console->error("Invalid valud for `--typed`: {}", typed);
        }
    }

    typecheck(gs, index(gs, inputFiles, prints, options), prints, options);

    clock_t end = clock();

    if (!prints.empty()) {
        for (auto option : prints) {
            console_err->error("Unknown --print option: {}\nValid values: {}", option, print_options);
            return 1;
        }
    }

    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;

    console_err->debug("Done in {} ms", elapsed_secs);

    if (options.count("store-state") != 0) {
        string outfile = options["store-state"].as<string>();
        ruby_typer::File::write(outfile.c_str(), ruby_typer::core::serialize::GlobalStateSerializer::store(gs));
    }

    if (options.count("counters") != 0) {
        for (auto e : gs.errors.errorHistogram) {
            ruby_typer::categoryCounterAdd("error", to_string(e.first), e.second);
        }
        console_err->warn("" + ruby_typer::getCounterStatistics());
    }

    if (options.count("statsd-host") != 0) {
        ruby_typer::submitCountersToStatsd(options["statsd-host"].as<string>(), options["statsd-port"].as<int>(),
                                           options["statsd-prefix"].as<string>() + ".counters");
    }

    return gs.errors.hadCriticalError() ? 10 : returnCode;
};
