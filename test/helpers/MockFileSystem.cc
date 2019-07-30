#include "test/helpers/MockFileSystem.h"

using namespace sorbet::test;

MockFileSystem::MockFileSystem(std::string_view rootPath) : rootPath(std::string(rootPath)) {}

std::string makeAbsolute(std::string_view rootPath, std::string_view path) {
    if (path[0] == '/') {
        return std::string(path);
    } else {
        return fmt::format("{}/{}", rootPath, path);
    }
}

void MockFileSystem::writeFiles(const std::vector<std::pair<std::string, std::string>> &initialFiles) {
    for (auto &pair : initialFiles) {
        writeFile(pair.first, pair.second);
    }
}

std::string MockFileSystem::readFile(std::string_view path) const {
    auto file = contents.find(makeAbsolute(rootPath, path));
    if (file == contents.end()) {
        throw sorbet::FileNotFoundException();
    } else {
        return file->second;
    }
}

void MockFileSystem::writeFile(std::string_view filename, std::string_view text) {
    contents[makeAbsolute(rootPath, filename)] = text;
}

void MockFileSystem::deleteFile(std::string_view filename) {
    auto file = contents.find(makeAbsolute(rootPath, filename));
    if (file == contents.end()) {
        throw sorbet::FileNotFoundException();
    } else {
        contents.erase(file);
    }
}

std::vector<std::string> MockFileSystem::listFilesInDir(std::string_view path,
                                                        const UnorderedSet<std::string> &extensions, bool recursive,
                                                        const std::vector<std::string> &absoluteIgnorePatterns,
                                                        const std::vector<std::string> &relativeIgnorePatterns) const {
    Exception::raise("Not implemented.");
}
