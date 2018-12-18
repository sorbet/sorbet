#ifndef TEST_EXPECTATIONS_H
#define TEST_EXPECTATIONS_H

#include "common/common.h"

struct Expectations {
    std::string folder;
    std::string basename;
    std::string testName;
    std::vector<std::string> sourceFiles;
    sorbet::UnorderedMap<std::string, std::string> expectations;
};

#endif // TEST_EXPECTATIONS_H