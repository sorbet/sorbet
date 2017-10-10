#include "ast/Trees.h"
#include "common/common.h"
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

    std::ifstream fin("parser/fixtures/gerald.rb");
    ASSERT_TRUE(fin.good());
    std::string src;
    fin.seekg(0, std::ios::end);
    src.reserve(fin.tellg());
    fin.seekg(0, std::ios::beg);

    src.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());

    ruby_typer::parser::parse_ruby(ctx, src);
}
