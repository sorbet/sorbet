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
using ruby_typer::u4;
using namespace std;

TEST(ParserTest, SimpleParse) {
    auto console = spd::stdout_color_mt("parse");
    ruby_typer::ast::ContextBase ctx(*console);
    ruby_typer::parser::parse_ruby(ctx, "<test>", "def hello_world; p :hello; end");
    ruby_typer::parser::parse_ruby(ctx, "<test>", "class A; class B; end; end");
    ruby_typer::parser::parse_ruby(ctx, "<test>", "class A::B; module B; end; end");
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

namespace ruby_typer {
namespace parser {
void offset2Pos(ast::UTF8Desc source, u4 off, u4 &line, u4 &col);
}
} // namespace ruby_typer

struct Offset2PosTest {
    string src;
    u4 off;
    u4 line;
    u4 col;
};

TEST(ParserTest, TestOffset2Pos) {
    vector<Offset2PosTest> cases = {
        {"hello", 0, 1, 0},
        {"line 1\nline 2", 1, 1, 1},
        {"line 1\nline 2", 7, 2, 0},
        {"line 1\nline 2", 11, 2, 4},
        {"a long line with no newlines\n", 20, 1, 20},
    };
    int i = 0;
    for (auto &tc : cases) {
        SCOPED_TRACE(string("case: ") + to_string(i));

        u4 col = -1, line = -1;
        ruby_typer::parser::offset2Pos(tc.src, tc.off, line, col);
        EXPECT_EQ(tc.col, col);
        EXPECT_EQ(tc.line, line);
        i++;
    }
}
