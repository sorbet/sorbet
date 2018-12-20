#ifndef TEST_EXPECTATIONS_H
#define TEST_EXPECTATIONS_H

#include "common/common.h"
#include "core/Files.h"

namespace sorbet::test {
struct Expectations {
    std::string folder;
    std::string basename;
    std::string testName;
    std::vector<std::string> sourceFiles;
    // folder + sourceFile => file
    UnorderedMap<std::string, std::shared_ptr<core::File>> sourceFileContents;
    UnorderedMap<std::string, std::string> expectations;
};
} // namespace sorbet::test

#endif // TEST_EXPECTATIONS_H