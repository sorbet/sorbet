#include "ast/ast.h"
#include "common/common.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <fstream>
#include <string>
#include <vector>

namespace spd = spdlog;

TEST(ParserTest, SimpleParse) {
    auto console = spd::stdout_color_mt("parse");
    ruby_typer::ast::ContextBase ctx(*console);
    ruby_typer::parser::parse_ruby(ctx, "def hello_world; p :hello; end");
    ruby_typer::parser::parse_ruby(ctx, "class A; class B; end; end");
    ruby_typer::parser::parse_ruby(ctx, "class A::B; module B; end; end");
}
