#include "ast/Trees.h"
#include "common/common.h"
#include "parser/Result.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"

namespace spd = spdlog;

TEST(ParserTest, SimpleParse) {
    auto console = spd::stdout_color_mt("parse");
    sruby::ast::ContextBase ctx(*console);
    sruby::parser::parse_ruby(ctx, "def hello_world; p :hello; end");
}
