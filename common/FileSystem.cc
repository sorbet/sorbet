#include "common/FileSystem.h"
#include "common/FileOps.h"

namespace sorbet {
using namespace std;

string OSFileSystem::readFile(string_view path) const {
    return FileOps::read(path);
}

void OSFileSystem::writeFile(string_view filename, string_view text) {
    return FileOps::write(filename, text);
}

vector<string> OSFileSystem::listFilesInDir(string_view path, const UnorderedSet<std::string> &extensions,
                                            bool recursive, const std::vector<std::string> &absoluteIgnorePatterns,
                                            const std::vector<std::string> &relativeIgnorePatterns,
                                            const std::vector<std::string> &absoluteUnignorePatterns,
                                            const std::vector<std::string> &relativeUnignorePatterns) const {
    return FileOps::listFilesInDir(path, extensions, recursive, absoluteIgnorePatterns, relativeIgnorePatterns,
                                   absoluteUnignorePatterns, relativeUnignorePatterns);
}

} // namespace sorbet
