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

TEST(DesugarTest, FixtureParse) {
    auto console = spd::stdout_color_mt("fixtures");

    for (auto &path : std::vector<std::string>({
             "ast/desugar/fixtures/gerald.rb",
             "ast/desugar/fixtures/misc.rb",
         })) {
        auto expPath = path + ".exp";
        ruby_typer::ast::ContextBase ctx(*console);
        ruby_typer::ast::Context context(ctx, ctx.defn_root());

        SCOPED_TRACE(path);

        auto src = ruby_typer::File::read(path.c_str());
        auto exp = ruby_typer::File::read(expPath.c_str());
        auto parsed = ruby_typer::parser::parse_ruby(ctx, src);
        auto desugared = ruby_typer::ast::desugar::node2Tree(context, parsed.ast());
        EXPECT_EQ(0, parsed.diagnostics().size());
        EXPECT_EQ(exp, desugared->toString(ctx) + "\n");
    }
}
