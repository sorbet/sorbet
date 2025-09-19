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
    std::string minimizeRBI;

    static Expectations getExpectations(std::string singleTest);
};

void CHECK_EQ_DIFF_IMPL(const char *file, int line, std::string_view expected, std::string_view actual,
                        std::string_view errorMessage);

// A variant of CHECK_EQ that prints a diff on failure.
#define CHECK_EQ_DIFF(expected, actual, errorMessage) \
    CHECK_EQ_DIFF_IMPL(__FILE__, __LINE__, (expected), (actual), (errorMessage));

} // namespace sorbet::test

#endif // TEST_HELPERS_EXPECTATIONS_H
