#ifndef TEST_HELPERS_EXPECTATIONS_H
#define TEST_HELPERS_EXPECTATIONS_H

#include "common/common.h"
#include "core/Files.h"

namespace sorbet::test {
struct Expectations {
    std::string folder;
    std::string basename;
    std::string testName;
    std::vector<std::string> sourceFiles;
    // version => [{originalFilename, versionFilePath}, ...]
    UnorderedMap<int, std::vector<std::pair<std::string, std::string>>> sourceFileUpdates;
    // folder + sourceFile => file
    UnorderedMap<std::string, std::shared_ptr<core::File>> sourceFileContents;
    UnorderedMap<std::string, std::string> expectations;
};
} // namespace sorbet::test

#endif // TEST_HELPERS_EXPECTATIONS_H