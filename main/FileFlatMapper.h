#ifndef FLAT_FILE_MAPPER_H
#define FLAT_FILE_MAPPER_H

#include <vector>

namespace ruby_typer {
namespace realmain {
/** read @file arguments and put them explicitly
 *  Steals the original arguments and will put them back on destruction.
 * */
class FileFlatMapper {
    int origArgc;
    const char **origArgv;
    int &argc;
    const char **&argv;
    std::vector<const char *> args;

public:
    FileFlatMapper(int &argc, const char **&argv);
    ~FileFlatMapper();
};
} // namespace realmain
} // namespace ruby_typer
#endif
