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
    std::shared_ptr<spdlog::logger> logger;
    int origArgc;
    char **origArgv;
    int &argc;
    char **&argv;
    std::vector<char *> args;
    // Pointers into those args will be passed in argv
    std::vector<std::string> stringArgs;
    cxxopts::Options options;

    /**
     * Read `@file` arguments and put them explicitly into `stringArgs`
     */
    void readArgsFromFile(std::string_view filename);

public:
    /**
     * Steal the original `argc` and `argv` and will put them back on destruction
     */
    ConfigParser(int &argc, char **&argv, std::shared_ptr<spdlog::logger> logger, cxxopts::Options options);

    /**
     * Parse the config CLI args, config files and default `sorbet/config` file
     */
    cxxopts::ParseResult parseConfig();

    ~ConfigParser();
};
} // namespace sorbet::realmain::options
#endif
