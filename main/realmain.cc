#include "../ast/ast.h"
#include "../core/Files.h"
#include "../namer/namer.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "core/serialize/serialize.h"
#include "infer/infer.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "payload/text/text.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include <algorithm> // find
#include <ctime>
#include <cxxopts.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace spd = spdlog;
using namespace std;

struct stats {
    unsigned int files = 0;
    unsigned int lines = 0;
    unsigned long bytes = 0;
};

class CFG_Collector_and_Typer {
    bool shouldType;
    bool printCFGs;

public:
    CFG_Collector_and_Typer(bool shouldType, bool printCFGs) : shouldType(shouldType), printCFGs(printCFGs){};

    ruby_typer::ast::MethodDef *preTransformMethodDef(ruby_typer::core::Context ctx, ruby_typer::ast::MethodDef *m) {
        if (m->loc.file.file(ctx).source_type == ruby_typer::core::File::Untyped)
            return m;

        auto cfg = ruby_typer::cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);
        if (shouldType) {
            ruby_typer::infer::Inference::run(ctx.withOwner(m->symbol), cfg);
        }
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

    try {
        for (auto f : frs) {
            try {
                tracer->trace("Parsing: {}", f.file(gs).path());
                auto nodes = ruby_typer::parser::Parser::run(gs, f);
                if (printParseTree) {
                    cout << nodes->toString(gs, 0) << endl;
                }

                ruby_typer::core::Context context(gs, gs.defn_root());
                tracer->trace("Desugaring: {}", f.file(gs).path());
                auto ast = ruby_typer::ast::desugar::node2Tree(context, nodes);
                if (printDesugared) {
                    cout << ast->toString(gs, 0) << endl;
                }

                if (printDesugaredRaw) {
                    cout << ast->showRaw(gs) << endl;
                }
                nodes = nullptr; // free up space early
                tracer->trace("Naming: {}", f.file(gs).path());
                result.emplace_back(ruby_typer::namer::Namer::run(context, move(ast)));
            } catch (...) {
                console_err->error("Exception on file: {} (backtrace is above)", f.file(gs).path());
            }
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

    try {
        for (auto &namedTree : what) {
            ruby_typer::core::FileRef f = namedTree->loc.file;
            try {
                ruby_typer::core::Context context(gs, gs.defn_root());
                tracer->trace("Resolving: {}", f.file(gs).path());
                auto resolved = ruby_typer::namer::Resolver::run(context, move(namedTree));
                if (printNameTree) {
                    cout << resolved->toString(gs, 0) << endl;
                }
                if (printNameTreeRaw) {
                    cout << resolved->showRaw(gs) << endl;
                }
                namedTree = move(resolved);
            } catch (...) {
                console_err->error("Exception resolving: {} (backtrace is above)", f.file(gs).path());
            }
        }

        ruby_typer::namer::Resolver::finalize(gs);

        for (auto &resolved : what) {
            ruby_typer::core::FileRef f = resolved->loc.file;
            try {
                ruby_typer::core::Context context(gs, gs.defn_root());
                if (printCFG) {
                    cout << "digraph \"" << ruby_typer::File::getFileName(f.file(gs).path()) << "\"{" << endl;
                }
                tracer->trace("CFG+Infer: {}", f.file(gs).path());
                bool doType = opts["typed"].as<string>() != "never";
                CFG_Collector_and_Typer collector(doType, printCFG);
                result.emplace_back(
                    ruby_typer::ast::TreeMap<CFG_Collector_and_Typer>::apply(context, collector, move(resolved)));

                if (printCFG) {
                    cout << "}" << endl << endl;
                }
            } catch (...) {
                console_err->error("Exception resolving: {} (backtrace is above)", f.file(gs).path());
            }
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

std::string print_options("[parse-tree, ast, ast-raw, name-table, name-table-full, name-tree, name-tree-raw, cfg]");

cxxopts::Options buildOptions() {
    cxxopts::Options options("ruby_typer", "Typechecker for Ruby");

    // Common user options in order of use
    options.add_options()("e", "Parse an inline ruby string", cxxopts::value<string>(), "string");
    options.add_options()("files", "Input files", cxxopts::value<vector<string>>());
    options.add_options()("q,quiet", "Silence all non-critical errors");
    options.add_options()("v,verbose", "Verbosity level [0-3]");
    options.add_options()("h,help", "Show long help");

    // Developer options
    options.add_options("dev")("p,print", "Print: " + print_options, cxxopts::value<vector<string>>(), "type");
    options.add_options("dev")("no-stdlib", "Do not load included rbi files for stdlib");
    options.add_options("dev")("typed", "Run full checks and report errors on all/no/only @typed code",
                               cxxopts::value<string>()->default_value("auto"), "{always,never,[auto]}");
    options.add_options("dev")("store-state", "Store state into file", cxxopts::value<string>(), "file");
    options.add_options("dev")("trace", "Trace phases");
    options.add_options("dev")("set-freshNameId", "Start freshNameId at value", cxxopts::value<int>(), "int");
    options.add_options("dev")("error-stats", "Print error statistics");
    options.add_options("dev")("counters", "Print internal counters");

    // Positional params
    options.parse_positional("files");
    options.positional_help("<file1.rb> <file2.rb> ...");
    return options;
}

ruby_typer::core::GlobalState createInitialGlobalState(cxxopts::Options &options) {
    if (!options["no-stdlib"].as<bool>()) {
        const ruby_typer::u4 *const nameTablePayload = getNameTablePayload;
        if (nameTablePayload == nullptr) {
            ruby_typer::core::GlobalState gs(*console);
            clock_t begin = clock();
            vector<ruby_typer::core::FileRef> payloadFiles;
            for (auto &p : ruby_typer::rbi::all()) {
                payloadFiles.push_back(gs.enterFile(p.first, p.second));
            }
            vector<string> emptyPrintsPayload;
            cxxopts::Options emptyOpts = buildOptions();

            typecheck(gs, index(gs, payloadFiles, emptyPrintsPayload, emptyOpts, true), emptyPrintsPayload, emptyOpts,
                      true); // result is thrown away

            clock_t end = clock();
            double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;
            console_err->debug("Indexed payload in {} ms\n", elapsed_secs);
            return gs;
        } else {
            clock_t begin = clock();
            ruby_typer::core::GlobalState gs =
                ruby_typer::core::serialize::GlobalStateSerializer::load(nameTablePayload, *console);
            clock_t end = clock();
            double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;
            console_err->debug("Read payload name-table in {} ms\n", elapsed_secs);
            return gs;
        }
    } else {
        return ruby_typer::core::GlobalState(*console);
    }
}

int realmain(int argc, char **argv) {
    //    spd::set_async_mode(1024);
    auto color_sink = make_shared<spdlog::sinks::ansicolor_stderr_sink_st>();
    color_sink->set_color(spd::level::info, color_sink->white);
    color_sink->set_color(spd::level::debug, color_sink->magenta);
    tracer = spd::details::registry::instance().create("tracer", color_sink);
    tracer->set_pattern("%v");

    console = spd::details::registry::instance().create("console", color_sink);
    console->set_pattern("%v");
    console_err = spd::stderr_color_st("");
    console_err->set_pattern("%v");
    FileFlatMapper flatMapper(argc, argv);

    cxxopts::Options options = buildOptions();

    try {
        options.parse(argc, argv);
    } catch (cxxopts::OptionParseException &e) {
        console->info("{}\n\n{}", e.what(), options.help({"", "dev"}));
        return 1;
    }

    vector<string> files = options["files"].as<vector<string>>();
    vector<string> prints = options["print"].as<vector<string>>();

    if (options["h"].as<bool>()) {
        console->info("{}", options.help({"", "dev"}));
        return 0;
    }
    if (options.count("e") == 0 && files.empty()) {
        console->info("You must pass either `-e` or at least one ruby file.\n\n{} \n", options.help());
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
    } else {
        tracer->set_level(spd::level::off);
    }

    bool forceTyped = options["typed"].as<string>() == "always";

    ruby_typer::core::GlobalState gs = createInitialGlobalState(options);

    stats st;
    clock_t begin = clock();
    vector<ruby_typer::core::FileRef> inputFiles;
    tracer->trace("Files: ");
    for (auto &fileName : files) {
        string src;
        try {
            src = ruby_typer::File::read(fileName.c_str());
        } catch (ruby_typer::FileNotFoundException e) {
            console->error("File Not Found: {}", fileName);
            returnCode = 11;
            continue;
        }
        st.bytes += src.size();
        st.lines += count(src.begin(), src.end(), '\n');
        st.files++;
        inputFiles.push_back(gs.enterFile(fileName, src));
        tracer->trace("{}", fileName);
    }
    if (options.count("e") != 0) {
        string src = options["e"].as<string>();
        st.files++;
        st.lines++;
        st.bytes += src.size();
        inputFiles.push_back(gs.enterFile(string("-e"), src));
        if (options["typed"].as<string>() == "never") {
            console->error("`-e` implies `--typed always` and you passed `--typed never`");
            return 1;
        }
        forceTyped = true;
    }
    if (forceTyped) {
        for (auto &f : inputFiles) {
            f.file(gs).source_type = ruby_typer::core::File::Typed;
        }
    }

    if (options.count("set-freshNameId") != 0) {
        gs.freshNameId = options["set-freshNameId"].as<int>();
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

    console_err->debug("Total {} files. Done in {} ms, lines: {}, bytes: {}\n", st.files, elapsed_secs, st.lines,
                       st.bytes);

    if (options.count("store-state") != 0) {
        string outfile = options["store-state"].as<string>();
        ruby_typer::File::write(outfile.c_str(), ruby_typer::core::serialize::GlobalStateSerializer::store(gs));
    }

    if (options.count("error-stats") != 0) {
        console_err->warn("Statistics of errors:");
        int sum = 0;
        vector<std::string> errors;
        for (auto e : gs.errors.errorHistogram) {
            errors.push_back(to_string(e.first) + " : " + to_string(e.second));
            sum += e.second;
        }
        sort(errors.begin(), errors.end());
        for (auto &l : errors) {
            console_err->warn("{}", l);
        }
        console_err->warn("Total: {}", sum);
    }

    if (options.count("counters") != 0) {
        console_err->warn("\n" + ruby_typer::getCounterStatistics());
    }

    return gs.errors.hadCriticalError() ? 10 : returnCode;
};
