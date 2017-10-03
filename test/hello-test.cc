#include "gtest/gtest.h"
#include "main/hello-greet.h"
#include "spdlog/spdlog.h"


TEST(HelloTest, GetGreet) {
  EXPECT_EQ(get_greet("Bazel"), "Hello Bazel");
}

namespace spd = spdlog;

TEST(HelloTest, GetSpdlog) {
        auto console = spd::stdout_color_mt("console");
        console->info("Welcome to spdlog!");
}
