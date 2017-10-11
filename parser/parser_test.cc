#include "ast/Trees.h"
#include "common/common.h"
#include "parser/Node.h"
#include "parser/Result.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <fstream>

namespace spd = spdlog;

TEST(ParserTest, SimpleParse) {
    auto console = spd::stdout_color_mt("parse");
    ruby_typer::ast::ContextBase ctx(*console);
    ruby_typer::parser::parse_ruby(ctx, "def hello_world; p :hello; end");
    ruby_typer::parser::parse_ruby(ctx, "class A; class B; end; end");
    ruby_typer::parser::parse_ruby(ctx, "class A::B; module B; end; end");
}

TEST(ParserTest, FixtureParse) {
    auto console = spd::stdout_color_mt("fixtures");
    ruby_typer::ast::ContextBase ctx(*console);

    auto src = ruby_typer::File::read("parser/fixtures/gerald.rb");
    auto exp = ruby_typer::File::read("parser/fixtures/gerald.rb.exp");

    auto got = ruby_typer::parser::parse_ruby(ctx, src);
    ASSERT_EQ(got.ast()->toString(ctx), exp);
}
