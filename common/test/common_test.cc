#include "gtest/gtest.h"
// violates our requirements, thus has to go first
#include "common/FileOps.h"
#include "common/Levenstein.h"
#include "common/common.h"

namespace sorbet::common {

TEST(CommonTest, Levenstein) { // NOLINT
    EXPECT_EQ(2, Levenstein::distance("Mama", "Papa", 10));
    EXPECT_EQ(5, Levenstein::distance("Ruby", "Scala", 10));
    EXPECT_EQ(3, Levenstein::distance("Java", "Scala", 10));
    EXPECT_EQ(INT_MAX, Levenstein::distance("Java", "S", 1));
}

class EnsureDirTest : public ::testing::Test {
protected:
    EnsureDirTest() {
        if (FileOps::dirExists("common_test_dir")) {
            FileOps::removeDir("common_test_dir");
        }
    }

    void TearDown() override {
        FileOps::removeDir("common_test_dir");
    }
};

TEST(EnsureDirTest, Test) { // NOLINT
    EXPECT_TRUE(FileOps::ensureDir("common_test_dir"));
    EXPECT_FALSE(FileOps::ensureDir("common_test_dir"));
}

} // namespace sorbet::common
