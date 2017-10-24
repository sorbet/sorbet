#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "cfg/CFG.h"
#include "namer/namer.h"
#include "parser/parser.h"
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

class CFG_Collector {
public:
    std::vector<std::string> cfgs;
    ruby_typer::ast::MethodDef *preTransformMethodDef(ruby_typer::ast::Context ctx, ruby_typer::ast::MethodDef *m) {
        cfgs.push_back(ruby_typer::cfg::CFG::buildFor(ctx, *m)->toString(ctx));
        return m;
    }
};

static bool removeOption(std::vector<std::string> &prints, std::string option) {
    auto it = find(prints.begin(), prints.end(), option);
    if (it != prints.end()) {
        prints.erase(it);
        return true;
    } else {
        return false;
    }
}

static bool startsWith(const string &str, const string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix.c_str(), prefix.size());
}

void parse_and_print(ruby_typer::ast::ContextBase &ctx, cxxopts::Options &opts, const string &path, const string &src,
                     std::vector<std::string> &prints) {
    auto r = ruby_typer::parser::parse_ruby(ctx, path, src);
    auto ast = r.ast();
    if (r.diagnostics().size() > 0) {
        vector<int> counts(static_cast<int>(ruby_parser::dlevel::FATAL) + 1);
        for (auto &diag : r.diagnostics()) {
            counts[static_cast<int>(diag.level())]++;
        }
        cerr << "parser reported " << r.diagnostics().size() << " errors:" << endl;
        cerr << " NOTE: " << counts[static_cast<int>(ruby_parser::dlevel::NOTE)] << endl;
        cerr << " WARNING: " << counts[static_cast<int>(ruby_parser::dlevel::WARNING)] << endl;
        cerr << " ERROR: " << counts[static_cast<int>(ruby_parser::dlevel::ERROR)] << endl;
        cerr << " FATAL: " << counts[static_cast<int>(ruby_parser::dlevel::FATAL)] << endl;
    }
    if (ast) {
        if (removeOption(prints, "parse-tree")) {
            cout << ast->toString(ctx, 0) << endl;
            if (prints.empty())
                return;
        }

        ruby_typer::ast::Context context(ctx, ctx.defn_root());
        auto desugared = ruby_typer::ast::desugar::node2Tree(context, ast);
        if (removeOption(prints, "ast")) {
            cout << desugared->toString(ctx) << endl;
            if (prints.empty())
                return;
        }

        desugared = ruby_typer::namer::Namer::run(context, std::move(desugared));
        if (removeOption(prints, "name-table")) {
            cout << ctx.toString() << endl;
            if (prints.empty())
                return;
        }
        if (removeOption(prints, "name-tree")) {
            cout << desugared->toString(ctx) << endl;
            if (prints.empty())
                return;
        }

        CFG_Collector collector;

        auto r = ruby_typer::ast::TreeMap<CFG_Collector>::apply(context, collector, std::move(desugared));
        std::stringstream buf;
        for (auto &cfg : collector.cfgs) {
            buf << cfg << std::endl << std::endl;
        }
        auto got = buf.str();
        if (removeOption(prints, "cfg")) {
            cout << got << endl;
            if (prints.empty())
                return;
        }
    } else {
        cout << " got null" << endl;
    }
}

int main(int argc, char **argv) {
    std::vector<std::string> files;
    std::vector<std::string> prints;
    //    spd::set_async_mode(1024);
    auto color_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_st>();
    color_sink->set_color(spd::level::info, color_sink->white);
    color_sink->set_color(spd::level::debug, color_sink->magenta);
    std::shared_ptr<spd::logger> console = spd::details::registry::instance().create("console", color_sink);
    std::shared_ptr<spd::logger> console_err = spd::stderr_color_st("");

    cxxopts::Options options("ruby_typer", "Parse ruby code, desguar it, build control flow graph and print it");
    options.add_options()("l,long", "Show long detailed output")("v,verbose", "Verbosity level [0-3]");
    options.add_options()("h,help", "Show help");
    options.add_options()("n,name-table", "Show name table");
    options.add_options()("p,print", "Print [parse-tree, ast, name-table, name-tree, cfg]",
                          cxxopts::value<std::vector<std::string>>(prints));
    options.add_options()("e", "Parse an inline ruby fragment", cxxopts::value<std::string>());
    options.add_options()("files", "Input files", cxxopts::value<std::vector<std::string>>(files));
    options.parse_positional("files");

    try {
        options.parse(argc, argv);
    } catch (cxxopts::option_not_exists_exception e) {
        console->info("{} \n {} \n", e.what(), options.help());
        return 0;
    }

    if (options["h"].as<bool>() || (options.count("e") == 0 && files.empty())) {
        console->info("{}\n", options.help());
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

    ruby_typer::ast::ContextBase ctx(*console);

    stats st;
    clock_t begin = clock();
    if (options.count("e")) {
        string src = options["e"].as<string>();
        st.files++;
        st.lines++;
        st.bytes += src.size();
        parse_and_print(ctx, options, "-e", src, prints);
    } else {

        for (auto &fileName : files) {
            console->debug("Parsing {}...", fileName);
            string src = ruby_typer::File::read(fileName.c_str());
            parse_and_print(ctx, options, fileName, src, prints);
        }
    }
    clock_t end = clock();

    if (!options.count("e")) {
        st.files = files.size();
        for (auto &fileName : files) {
            string src = ruby_typer::File::read(fileName.c_str());
            st.bytes += src.size();
            st.lines += count(src.begin(), src.end(), '\n');
        }
    }

    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;

    console_err->info("Total {} files. Done in {} ms, lines: {}, bytes: {}\n", st.files, elapsed_secs, st.lines,
                      st.bytes);

    return 0;
}
