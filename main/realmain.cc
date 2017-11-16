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

public:
    vector<string> cfgs;
    CFG_Collector_and_Typer(bool shouldType) : shouldType(shouldType){};

    ruby_typer::ast::MethodDef *preTransformMethodDef(ruby_typer::core::Context ctx, ruby_typer::ast::MethodDef *m) {
        auto cfg = ruby_typer::cfg::CFG::buildFor(ctx.withOwner(m->symbol), *m);
        if (shouldType) {
            ruby_typer::infer::Inference::run(ctx.withOwner(m->symbol), cfg);
        }
        cfgs.push_back(cfg->toString(ctx));
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

void index(ruby_typer::core::GlobalState &gs, const vector<pair<string, string>> nameAndSource) {
    ruby_typer::parser::NodeVec empty;
    ruby_typer::core::Loc emptyLoc(0, 0, 0);
    unique_ptr<ruby_typer::parser::Begin> join = make_unique<ruby_typer::parser::Begin>(emptyLoc, move(empty));
    for (auto &pair : nameAndSource) {
        auto ast = ruby_typer::parser::Parser::run(gs, pair.first, pair.second);
        if (ast) {
            join->stmts.emplace_back(move(ast));
        } else {
            ruby_typer::Error::raise("Failed to index " + pair.first);
        }
    }
    gs.errors.keepErrorsInMemory = true; // silence errors for stdlib.
    ruby_typer::core::Context context(gs, gs.defn_root());
    unique_ptr<ruby_typer::parser::Node> node(move(join));
    auto desugared = ruby_typer::ast::desugar::node2Tree(context, node);
    ruby_typer::namer::Namer::run(context, move(desugared));
    gs.errors.getAndEmptyErrors();
    gs.errors.keepErrorsInMemory = false;
}

void parse_and_print(ruby_typer::core::GlobalState &gs, cxxopts::Options &opts, const string &path, const string &src,
                     vector<string> &prints) {
    auto ast = ruby_typer::parser::Parser::run(gs, path, src);
    if (!ast) {
        ruby_typer::Error::raise("Parse Error");
    }

    if (removeOption(prints, "parse-tree")) {
        cout << ast->toString(gs, 0) << endl;
        if (prints.empty())
            return;
    }

    ruby_typer::core::Context context(gs, gs.defn_root());
    auto desugared = ruby_typer::ast::desugar::node2Tree(context, ast);
    if (removeOption(prints, "ast")) {
        cout << desugared->toString(gs, 0) << endl;
        if (prints.empty())
            return;
    }
    if (removeOption(prints, "ast-raw")) {
        cout << desugared->showRaw(gs) << endl;
        if (prints.empty())
            return;
    }

    desugared = ruby_typer::namer::Namer::run(context, move(desugared));
    if (removeOption(prints, "name-table")) {
        cout << gs.toString() << endl;
        if (prints.empty())
            return;
    }
    if (removeOption(prints, "name-tree")) {
        cout << desugared->toString(gs, 0) << endl;
        if (prints.empty())
            return;
    }
    if (removeOption(prints, "name-tree-raw")) {
        cout << desugared->showRaw(gs) << endl;
        if (prints.empty())
            return;
    }

    CFG_Collector_and_Typer collector(!opts["no-typer"].as<bool>());

    auto cfg_and_throw_out_result =
        ruby_typer::ast::TreeMap<CFG_Collector_and_Typer>::apply(context, collector, move(desugared));
    if (removeOption(prints, "cfg")) {
        stringstream buf;

        buf << "digraph \"" + ruby_typer::File::getFileName(path) + "\"{" << endl;
        for (auto &cfg : collector.cfgs) {
            buf << cfg << endl << endl;
        }
        buf << "}" << endl;
        auto got = buf.str();
        cout << got << endl;
        if (prints.empty())
            return;
    }

    // All good!
}

int realmain(int argc, char **argv) {
    vector<string> files;
    vector<string> prints;
    //    spd::set_async_mode(1024);
    auto color_sink = make_shared<spdlog::sinks::ansicolor_stderr_sink_st>();
    color_sink->set_color(spd::level::info, color_sink->white);
    color_sink->set_color(spd::level::debug, color_sink->magenta);
    shared_ptr<spd::logger> console = spd::details::registry::instance().create("console", color_sink);
    console->set_pattern("%v");
    shared_ptr<spd::logger> console_err = spd::stderr_color_st("");
    console_err->set_pattern("%v");

    cxxopts::Options options("ruby_typer", "Parse ruby code, desguar it, build control flow graph and print it");
    options.add_options()("v,verbose", "Verbosity level [0-3]");
    options.add_options()("h,help", "Show help");
    options.add_options()("no-stdlib", "Do not load included rbi files for stdlib");
    options.add_options()("no-typer", "Do not type the CFG");
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

    ruby_typer::core::GlobalState gs(*console);

    if (!options["no-typer"].as<bool>()) {
        clock_t begin = clock();

        index(gs, ruby_typer::rbi::all());

        clock_t end = clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;
        console_err->debug("Indexed payload in {} ms\n", elapsed_secs);
    }
    stats st;
    clock_t begin = clock();
    if (options.count("e")) {
        string src = options["e"].as<string>();
        st.files++;
        st.lines++;
        st.bytes += src.size();
        parse_and_print(gs, options, "-e", src, prints);
    } else {
        for (auto &fileName : files) {
            console->debug("Parsing {}...", fileName);
            string src;
            try {
                src = ruby_typer::File::read(fileName.c_str());
            } catch (ruby_typer::FileNotFoundException e) {
                console->error("File Not Found: {}", fileName);
                return 1;
            }
            parse_and_print(gs, options, fileName, src, prints);
        }
    }
    clock_t end = clock();

    if (!prints.empty()) {
        for (auto option : prints) {
            console_err->error("Unknown --print option: {}", option);
            return 1;
        }
    }

    if (!options.count("e")) {
        st.files = files.size();
        for (auto &fileName : files) {
            string src = ruby_typer::File::read(fileName.c_str());
            st.bytes += src.size();
            st.lines += count(src.begin(), src.end(), '\n');
        }
    }

    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;

    console_err->debug("Total {} files. Done in {} ms, lines: {}, bytes: {}\n", st.files, elapsed_secs, st.lines,
                       st.bytes);

    return 0;
};
