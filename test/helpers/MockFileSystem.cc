#include "test/helpers/MockFileSystem.h"

using namespace std;

namespace sorbet::test {
MockFileSystem::MockFileSystem(string_view rootPath) : rootPath(string(rootPath)) {}

string makeAbsolute(string_view rootPath, string_view path) {
    if (path[0] == '/') {
        return string(path);
    } else {
        return fmt::format("{}/{}", rootPath, path);
    }
}

void MockFileSystem::writeFiles(const vector<pair<string, string>> &initialFiles) {
    for (auto &pair : initialFiles) {
        writeFile(pair.first, pair.second);
    }
}

string MockFileSystem::readFile(string_view path, size_t nullPadding) const {
    auto file = contents.find(makeAbsolute(rootPath, path));
    if (file == contents.end()) {
        throw sorbet::FileNotFoundException();
    } else {
        return file->second + string(nullPadding, '\0');
    }
}

void MockFileSystem::writeFile(string_view filename, string_view text) {
    contents[makeAbsolute(rootPath, filename)] = text;
}

void MockFileSystem::deleteFile(string_view filename) {
    auto file = contents.find(makeAbsolute(rootPath, filename));
    if (file == contents.end()) {
        throw sorbet::FileNotFoundException();
    } else {
        contents.erase(file);
    }
}

vector<string> MockFileSystem::listFilesInDir(string_view path, const UnorderedSet<string> &extensions, bool recursive,
                                              const vector<string> &absoluteIgnorePatterns,
                                              const vector<string> &relativeIgnorePatterns) const {
    Exception::raise("Not implemented.");
}
} // namespace sorbet::test
