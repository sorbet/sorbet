#include "../Desugar.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace spd = spdlog;

TEST(DesugarTest, SimpleParse) {
    auto console = spd::stdout_color_mt("parse");
    ruby_typer::ast::ContextBase ctx(*console);
    auto i1 = ruby_typer::parser::parse_ruby(ctx, "def hello_world; p :hello; end");
    auto i2 = ruby_typer::parser::parse_ruby(ctx, "class A; class B; end; end");
    auto i3 = ruby_typer::parser::parse_ruby(ctx, "class A::B; module B; end; end");
    ruby_typer::ast::Context context(ctx, ctx.defn_root());
    auto o1 = ruby_typer::ast::desugar::node2Tree(context, i1.ast());
}
