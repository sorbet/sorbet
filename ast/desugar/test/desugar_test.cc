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

TEST(DesugarTest, SimpleDesugar) {
    auto console = spd::stderr_color_mt("console");
    ruby_typer::ast::ContextBase ctx(*console);
    auto i1 = ruby_typer::parser::parse_ruby(ctx, "<test>", "def hello_world; p :hello; end");
    ruby_typer::ast::Context context(ctx, ctx.defn_root());
    auto o1 = ruby_typer::ast::desugar::node2Tree(context, i1.ast());
}
