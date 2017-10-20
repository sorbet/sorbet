#include "ast/ast.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"

namespace spd = spdlog;
using namespace std;

namespace ruby_typer {
namespace ast {

auto console = spd::stdout_color_mt("parse");

void offset2Pos(ast::UTF8Desc source, u4 off, u4 &line, u4 &col);

struct Offset2PosTest {
    string src;
    u4 off;
    u4 line;
    u4 col;
};

TEST(ASTTest, TestOffset2Pos) {
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
        offset2Pos(tc.src, tc.off, line, col);

        EXPECT_EQ(tc.col, col);
        EXPECT_EQ(tc.line, line);
        i++;
    }
}

} // namespace ast
} // namespace ruby_typer
