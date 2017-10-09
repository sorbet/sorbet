#include "ast/Trees.h"
#include "common/common.h"
#include "main/hello-greet.h"
#include "parser/Result.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <cxxopts.hpp>

TEST(HelloTest, GetGreet) {
    EXPECT_EQ(get_greet("Bazel"), "Hello Bazel");
}

namespace spd = spdlog;

auto console = spd::stdout_color_mt("console");

TEST(HelloTest, GetSpdlog) {
    console->info("Welcome to spdlog!");
}

TEST(HelloTest, SimpleParse) {
    sruby::ast::ContextBase ctx(*console);
    sruby::parser::parse_ruby(ctx, "def hello_world; p :hello; end");
}

TEST(HelloTest, GetCXXopts) {
    cxxopts::Options options("MyProgram", "One line description of MyProgram");
}
