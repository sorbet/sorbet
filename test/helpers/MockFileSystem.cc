#include "test/helpers/MockFileSystem.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "common/FileOps.h"
#include "common/sort/sort.h"

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

string MockFileSystem::readFile(const string &path) const {
    auto file = contents.find(makeAbsolute(rootPath, path));
    if (file == contents.end()) {
        throw sorbet::FileNotFoundException(fmt::format("Cannot find file `{}`", path));
    } else {
        return file->second;
    }
}

void MockFileSystem::writeFile(const string &filename, string_view text) {
    contents[makeAbsolute(rootPath, filename)] = text;
}

void MockFileSystem::deleteFile(string_view filename) {
    auto file = contents.find(makeAbsolute(rootPath, filename));
    if (file == contents.end()) {
        throw sorbet::FileNotFoundException(fmt::format("Cannot find file `{}`", filename));
    } else {
        contents.erase(file);
    }
}

vector<string> MockFileSystem::listFilesInDir(string_view path, const UnorderedSet<string> &extensions, bool recursive,
                                              const vector<string> &absoluteIgnorePatterns,
                                              const vector<string> &relativeIgnorePatterns) const {
    if (!recursive) {
        Exception::raise("MockFileSystem only implements recursive listings.");
    }

    auto dir = makeAbsolute(rootPath, path);
    // `writeFile` stores keys with no trailing slash, so one has to be added to make a directory a prefix.
    auto prefix = absl::EndsWith(dir, "/") ? dir : absl::StrCat(dir, "/");

    vector<string> result;
    for (auto &[filePath, _contents] : contents) {
        if (!absl::StartsWith(filePath, prefix) || !FileOps::hasAllowedExtension(filePath, extensions) ||
            FileOps::isFileIgnored(rootPath, filePath, absoluteIgnorePatterns, relativeIgnorePatterns)) {
            continue;
        }
        result.emplace_back(filePath);
    }

    // The real implementation walks a directory tree, so do not leak this hash map's arbitrary order to callers.
    fast_sort(result);
    return result;
}

vector<string> MockFileSystem::listFilesInDir(string_view path, const UnorderedSet<string> &extensions,
                                              WorkerPool &workerPool, bool recursive,
                                              const vector<string> &absoluteIgnorePatterns,
                                              const vector<string> &relativeIgnorePatterns) const {
    // There is nothing here worth parallelizing.
    return listFilesInDir(path, extensions, recursive, absoluteIgnorePatterns, relativeIgnorePatterns);
}
} // namespace sorbet::test
