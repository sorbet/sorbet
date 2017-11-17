#include "../ast/ast.h"
#include "../core/Files.h"
#include "../namer/namer.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "cfg/CFG.h"
#include "infer/infer.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "rbi/payload.h"
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
        auto cfg = ruby_typer::cfg::CFG::buildFor(ctx.withOwner(m->symbol), *m);
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
                tracer->critical("Parsing: {}", f.file(gs).path().toString());
                auto nodes = ruby_typer::parser::Parser::run(gs, f);
                if (printParseTree) {
                    cout << nodes->toString(gs, 0) << endl;
                }

                ruby_typer::core::Context context(gs, gs.defn_root());
                tracer->critical("Desugaring: {}", f.file(gs).path().toString());
                auto ast = ruby_typer::ast::desugar::node2Tree(context, nodes);
                if (printDesugared) {
                    cout << ast->toString(gs, 0) << endl;
                }

                if (printDesugaredRaw) {
                    cout << ast->showRaw(gs) << endl;
                }
                nodes = nullptr; // free up space early
                tracer->critical("Naming: {}", f.file(gs).path().toString());
                result.emplace_back(ruby_typer::namer::Namer::run(context, move(ast)));
            } catch (...) {
                console_err->error("Exception on file: {} (backtrace is above)", f.file(gs).path().toString());
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
    bool printNameTree = removeOption(prints, "name-tree");
    bool printNameTreeRaw = removeOption(prints, "name-tree-raw");
    bool printCFG = removeOption(prints, "cfg");

    try {
        for (auto &namedTree : what) {
            ruby_typer::core::FileRef f = namedTree->loc.file;
            try {
                ruby_typer::core::Context context(gs, gs.defn_root());
                tracer->critical("Resolving: {}", f.file(gs).path().toString());
                auto resolved = ruby_typer::namer::Resolver::run(context, move(namedTree));
                if (printNameTree) {
                    cout << resolved->toString(gs, 0) << endl;
                }
                if (printNameTreeRaw) {
                    cout << resolved->showRaw(gs) << endl;
                }

                if (printCFG) {
                    cout << "digraph \"" + ruby_typer::File::getFileName(f.file(gs).path().toString()) + "\"{" << endl;
                }
                tracer->critical("CFG+Infer: {}", f.file(gs).path().toString());
                CFG_Collector_and_Typer collector(!opts["no-typer"].as<bool>(), printCFG);
                result.emplace_back(
                    ruby_typer::ast::TreeMap<CFG_Collector_and_Typer>::apply(context, collector, move(resolved)));

                if (printCFG) {
                    cout << "}" << endl << endl;
                }
            } catch (...) {
                console_err->error("Exception on file: {} (backtrace is above)", f.file(gs).path().toString());
            }
        }
        if (printNameTable) {
            cout << gs.toString() << endl;
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

int realmain(int argc, char **argv) {
    vector<string> files;
    vector<string> prints;
    //    spd::set_async_mode(1024);
    auto color_sink = make_shared<spdlog::sinks::ansicolor_stderr_sink_st>();
    color_sink->set_color(spd::level::info, color_sink->white);
    color_sink->set_color(spd::level::debug, color_sink->magenta);
    tracer = spd::details::registry::instance().create("tracer", color_sink);
    tracer->set_pattern("%v");

    shared_ptr<spd::logger> console = spd::details::registry::instance().create("console", color_sink);
    console->set_pattern("%v");
    console_err = spd::stderr_color_st("");
    console_err->set_pattern("%v");

    cxxopts::Options options("ruby_typer", "Parse ruby code, desguar it, build control flow graph and print it");
    options.add_options()("v,verbose", "Verbosity level [0-3]");
    options.add_options()("h,help", "Show help");
    options.add_options()("no-stdlib", "Do not load included rbi files for stdlib");
    options.add_options()("no-typer", "Do not type the CFG");
    options.add_options()("trace", "trace phases");
    options.add_options()("q,quiet", "Silence all non-critical errors");
    options.add_options()("p,print", "Print [parse-tree, ast, ast-raw, name-table, name-tree, name-tree-raw, cfg]",
                          cxxopts::value<vector<string>>(prints));
    options.add_options()("e", "Parse an inline ruby fragment", cxxopts::value<string>());
    options.add_options()("files", "Input files", cxxopts::value<vector<string>>(files));
    options.parse_positional("files");

    try {
        options.parse(argc, argv);
    } catch (cxxopts::option_not_exists_exception e) {
        console->info("{}\n\n{}", e.what(), options.help());
        return 0;
    }

    if (options["h"].as<bool>()) {
        console->info("{}", options.help());
        return 0;
    }
    if (options.count("e") == 0 && files.empty()) {
        console->info("You must pass either `-e` or at least one ruby file.\n {} \n", options.help());
        return 0;
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

    if (options.count("q")) {
        spd::set_level(spd::level::critical);
    }

    if (options.count("q")) {
        tracer->set_level(spd::level::trace);
    } else {
        tracer->set_level(spd::level::off);
    }

    ruby_typer::core::GlobalState gs(*console);

    if (!options["no-stdlib"].as<bool>()) {
        clock_t begin = clock();
        vector<ruby_typer::core::FileRef> payloadFiles;
        for (auto &p : ruby_typer::rbi::all()) {
            payloadFiles.push_back(gs.enterFile(p.first, p.second));
        }
        vector<string> emptyPrintsPayload;
        cxxopts::Options emptyOpts("");

        index(gs, payloadFiles, emptyPrintsPayload, emptyOpts, true); // result is thrown away

        clock_t end = clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;
        console_err->debug("Indexed payload in {} ms\n", elapsed_secs);
    }
    stats st;
    clock_t begin = clock();
    vector<ruby_typer::core::FileRef> inputFiles;
    console->critical("Files: ");
    for (auto &fileName : files) {
        console->debug("Reading {}...", fileName);
        string src;
        try {
            src = ruby_typer::File::read(fileName.c_str());
        } catch (ruby_typer::FileNotFoundException e) {
            console->error("File Not Found: {}", fileName);
            return 1;
        }
        st.bytes += src.size();
        st.lines += count(src.begin(), src.end(), '\n');
        st.files++;
        inputFiles.push_back(gs.enterFile(fileName, src));
        console->critical("{}", fileName);
    }
    if (options.count("e")) {
        string src = options["e"].as<string>();
        st.files++;
        st.lines++;
        st.bytes += src.size();
        inputFiles.push_back(gs.enterFile(string("-e"), src));
    }

    typecheck(gs, index(gs, inputFiles, prints, options), prints, options);

    clock_t end = clock();

    if (!prints.empty()) {
        for (auto option : prints) {
            console_err->error("Unknown --print option: {}", option);
            return 1;
        }
    }

    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;

    console_err->debug("Total {} files. Done in {} ms, lines: {}, bytes: {}\n", st.files, elapsed_secs, st.lines,
                       st.bytes);

    return gs.errors.hadCriticalError() ? 10 : 0;
};
