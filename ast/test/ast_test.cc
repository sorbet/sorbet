#include "ast/ast.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"

namespace spd = spdlog;
using namespace std;

namespace ruby_typer {
namespace ast {

auto console = spd::stderr_color_mt("parse");

Loc::Detail offset2Pos(ast::UTF8Desc source, u4 off);

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

        auto detail = offset2Pos(tc.src, tc.off);

        EXPECT_EQ(tc.col, detail.column);
        EXPECT_EQ(tc.line, detail.line);
        i++;
    }
}

TEST(ASTTest, ErrorReporter) {
    GlobalState ctx(*console);
    FileRef f = ctx.enterFile(string("a/foo.rb"), string("def foo\n  hi\nend\n"));
    ctx.errors.error(Loc{f, 0, 3}, ErrorClass::Internal, "Use of metavariable: {}", "foo");
}

TEST(ASTTest, SymbolRef) {
    GlobalState ctx(*console);
    SymbolRef ref = ctx.defn_object();
    EXPECT_EQ(ref, ref.info(ctx).ref(ctx));
}

} // namespace ast
} // namespace ruby_typer
