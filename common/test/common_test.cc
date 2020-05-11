#include "doctest.h"
// violates our requirements, thus has to go first
#include "common/FileOps.h"
#include "common/Levenstein.h"
#include "common/common.h"

namespace sorbet::common {

TEST_CASE("Levenstein") { // NOLINT
    CHECK_EQ(2, Levenstein::distance("Mama", "Papa", 10));
    CHECK_EQ(5, Levenstein::distance("Ruby", "Scala", 10));
    CHECK_EQ(3, Levenstein::distance("Java", "Scala", 10));
    CHECK_EQ(INT_MAX, Levenstein::distance("Java", "S", 1));
}

TEST_CASE("FileOps::ensureDir") {
    if (FileOps::dirExists("common_test_dir")) {
        FileOps::removeDir("common_test_dir");
    }

    CHECK(FileOps::ensureDir("common_test_dir"));
    CHECK_FALSE(FileOps::ensureDir("common_test_dir"));

    FileOps::removeDir("common_test_dir");
}

} // namespace sorbet::common
