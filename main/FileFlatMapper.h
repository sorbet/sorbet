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
    char **origArgv;
    int &argc;
    char **&argv;
    std::vector<char *> args;

public:
    FileFlatMapper(int &argc, char **&argv);
    ~FileFlatMapper();
};
} // namespace realmain
} // namespace ruby_typer
#endif
