#include "ast/ast.h"
#include "common/common.h"
#include "parser/Dedenter.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <fstream>
#include <string>
#include <vector>

namespace spd = spdlog;
using namespace std;

TEST(ParserTest, SimpleParse) {
    auto console = spd::stdout_color_mt("parse");
    ruby_typer::ast::ContextBase ctx(*console);
    ruby_typer::parser::parse_ruby(ctx, "def hello_world; p :hello; end");
    ruby_typer::parser::parse_ruby(ctx, "class A; class B; end; end");
    ruby_typer::parser::parse_ruby(ctx, "class A::B; module B; end; end");
}

struct DedentTest {
    int level;
    string in;
    string out;
};

TEST(ParserTest, TestDedent) {
    vector<DedentTest> cases = {
        {2, "    hi", "  hi"},
        {10, "  \t    hi", "  hi"},
        {2, "  a\n   b\n  c\n", "a\n   b\n  c\n"},
    };
    for (auto &tc : cases) {
        ruby_typer::parser::Dedenter dedent(tc.level);
        string got = dedent.dedent(tc.in);
        EXPECT_EQ(got, tc.out);
    }
}
