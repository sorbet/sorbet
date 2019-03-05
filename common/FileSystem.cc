#include "common/FileSystem.h"
#include "common/common.h"

namespace sorbet {
using namespace std;

string OSFileSystem::readFile(string_view path) const {
    return FileOps::read(path);
}

void OSFileSystem::writeFile(string_view filename, string_view text) {
    return FileOps::write(filename, text);
}

vector<string> OSFileSystem::listFilesInDir(string_view path, bool recursive) const {
    // TODO: Implement once Sorbet supports folder inputs.
    Exception::raise("listFilesInDir not yet supported");
}

} // namespace sorbet