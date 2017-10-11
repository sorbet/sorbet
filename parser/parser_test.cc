#include "ast/Trees.h"
#include "common/common.h"
#include "parser/Node.h"
#include "parser/Result.h"
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

TEST(ParserTest, FixtureParse) {
    auto console = spd::stdout_color_mt("fixtures");
    ruby_typer::ast::ContextBase ctx(*console);

    for (auto &path : std::vector<std::string>({
             "parser/fixtures/gerald.rb",
             "parser/fixtures/misc.rb",
         })) {
        auto expPath = path + ".exp";

        SCOPED_TRACE(path);

        auto src = ruby_typer::File::read(path.c_str());
        auto exp = ruby_typer::File::read(expPath.c_str());
        auto got = ruby_typer::parser::parse_ruby(ctx, src);
        EXPECT_EQ(0, got.diagnostics().size());
        EXPECT_EQ(exp, got.ast()->toString(ctx) + "\n");
    }
}
