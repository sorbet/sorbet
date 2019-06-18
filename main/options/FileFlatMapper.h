#ifndef FLAT_FILE_MAPPER_H
#define FLAT_FILE_MAPPER_H
#include "spdlog/spdlog.h"
#include <cxxopts.hpp>
#include <vector>

namespace sorbet::realmain::options {
/** read @file arguments and put them explicitly
 *  Steals the original arguments and will put them back on destruction.
 * */
class FileFlatMapper {
    std::shared_ptr<spdlog::logger> logger;
    int origArgc;
    char **origArgv;
    int &argc;
    char **&argv;
    std::vector<char *> args;
    // Pointers into those args will be passed in argv
    std::vector<std::string> stringArgs;
    cxxopts::Options options;

    void readArgsFromFile(std::string_view filename);

public:
    FileFlatMapper(int &argc, char **&argv, std::shared_ptr<spdlog::logger> logger, cxxopts::Options options);

    /*
     * Parse the config CLI args, config files and default `sorbet/config` file
     */
    cxxopts::ParseResult parseConfig();

    ~FileFlatMapper();
};
} // namespace sorbet::realmain::options
#endif
