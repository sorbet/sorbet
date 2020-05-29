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
    UnorderedMap<int, std::vector<std::pair<std::string, std::string>>> sourceLSPFileUpdates;
    // folder + sourceFile => file
    UnorderedMap<std::string, std::shared_ptr<core::File>> sourceFileContents;
    // expectations type => file => expectations for file
    UnorderedMap<std::string, UnorderedMap<std::string, std::string>> expectations;

    static Expectations getExpectations(std::string singleTest);
};

// A variant of CHECK_EQ that prints a diff on failure.
void CHECK_EQ_DIFF(std::string_view expected, std::string_view actual, std::string_view errorMessage);

} // namespace sorbet::test

#endif // TEST_HELPERS_EXPECTATIONS_H