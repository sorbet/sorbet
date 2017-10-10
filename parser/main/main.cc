#include "ast/ast.h"
#include "parser/Node.h"
#include "parser/Result.h"
#include "spdlog/spdlog.h"
#include <ctime>
#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace spd = spdlog;
using namespace std;

int main(int argc, char **argv) {
    std::vector<std::string> files;
    //    spd::set_async_mode(1024);
    auto color_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_st>();
    color_sink->set_color(spd::level::info, color_sink->white);
    color_sink->set_color(spd::level::debug, color_sink->magenta);
    std::shared_ptr<spd::logger> console = spd::details::registry::instance().create("console", color_sink);
    std::shared_ptr<spd::logger> console_err = spd::stderr_color_st("");

    cxxopts::Options options("parse_ruby", "Parse ruby code and print it");
    options.add_options()("l,long", "Show long detailed output")("v,verbose", "Verbosity level [0-3]");
    options.add_options()("h,help", "Show help");
    options.add_options()("files", "Input files", cxxopts::value<std::vector<std::string>>(files));
    options.parse_positional("files");

    try {
        options.parse(argc, argv);
    } catch (cxxopts::option_not_exists_exception e) {
        console->info("{} \n {} \n", e.what(), options.help());
        return 0;
    }

    if (options["h"].as<bool>() || files.empty()) {
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

    clock_t begin = clock();
    for (auto &fileName : files) {
        std::ifstream fin(fileName);
        // Determine the file length
        ruby_typer::Error::check(fin.good());
        std::string src;
        fin.seekg(0, std::ios::end);
        src.reserve(fin.tellg());
        fin.seekg(0, std::ios::beg);

        src.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        auto r = ruby_typer::parser::parse_ruby(ctx, src);
        auto ast = r.ast();
        if (ast) {
            cout << ast->toString(ctx, 0);
        } else {
            cout << " got null" << endl;
        }
    }
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000;

    console_err->info("Total {} files. Done in {} ms, nt size: {}, st size: {}\n", files.size(), elapsed_secs, -1, -1);

    return 0;
}
