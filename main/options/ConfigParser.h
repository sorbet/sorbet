#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H
#include "spdlog/spdlog.h"
#include <cxxopts.hpp>
#include <vector>

namespace sorbet::realmain::options {
/**
 * Parse configuration for Sorbet
 *
 * Sorbet's configuration is split into three supports in order of precedence:
 *
 * 1. CLI options: passed directly to Sorbet such `sorbet --typed=false`
 * 2. File options: written in files passed to Sorbet such as `sorbet @my_config`
 * 3. Default config file options: written in `sorbet/config`
 *
 * This class parses and merges all these options and handle the precedence between them.
 *
 * See `ConfigParser::parseConfig()`.
 */
class ConfigParser {
    /**
     * Read `@file` arguments and put them explicitly into `stringArgs`
     *
     * `depth` is the number of `@file` includes that led to this call, and is
     * used to reject include cycles (`@`-ing a file that `@`-s itself,
     * directly or transitively) and unbounded include chains, which would
     * otherwise recurse until the stack overflows.
     */
    static void readArgsFromFile(std::shared_ptr<spdlog::logger> logger, std::string_view filename,
                                 std::vector<std::string> &stringArgs, unsigned int depth = 0);

public:
    /**
     * Parse the config CLI args, config files and default `sorbet/config` file
     */
    static cxxopts::ParseResult parseConfig(std::shared_ptr<spdlog::logger> logger, int &argc, char **&argv,
                                            cxxopts::Options options);
};
} // namespace sorbet::realmain::options
#endif
