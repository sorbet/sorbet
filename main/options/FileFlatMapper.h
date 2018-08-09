#ifndef FLAT_FILE_MAPPER_H
#define FLAT_FILE_MAPPER_H
#include "spdlog/spdlog.h"
#include <vector>

namespace sorbet {
namespace realmain {
namespace options {
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
    FileFlatMapper(int &argc, char **&argv, std::shared_ptr<spdlog::logger> logger);

    ~FileFlatMapper();
};
}; // namespace options
} // namespace realmain
} // namespace sorbet
#endif
