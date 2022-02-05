#ifndef TEST_HELPERS_MOCKFILESYSTEM_H
#define TEST_HELPERS_MOCKFILESYSTEM_H

#include "common/FileSystem.h"

namespace sorbet::test {
using namespace sorbet;

class MockFileSystem final : public FileSystem {
private:
    UnorderedMap<std::string, std::string> contents;
    std::string rootPath;

public:
    MockFileSystem(std::string_view rootPath);
    void writeFiles(const std::vector<std::pair<std::string, std::string>> &files);
    std::string readFile(std::string_view path, size_t nullPadding = 0) const override;
    void writeFile(std::string_view filename, std::string_view text) override;
    void deleteFile(std::string_view filename);
    std::vector<std::string> listFilesInDir(std::string_view path, const UnorderedSet<std::string> &extensions,
                                            bool recursive, const std::vector<std::string> &absoluteIgnorePatterns,
                                            const std::vector<std::string> &relativeIgnorePatterns) const override;
};

} // namespace sorbet::test

#endif // TEST_HELPERS_MOCKFILESYSTEM_H
