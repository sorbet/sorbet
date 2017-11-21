#include "../Context.h"
#include "core/core.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"

namespace spd = spdlog;
using namespace std;

namespace ruby_typer {
namespace core {
auto console = spd::stderr_color_mt("parse");

struct Offset2PosTest {
    string src;
    u4 off;
    u4 line;
    u4 col;
};

TEST(ASTTest, TestOffset2Pos) {
    core::GlobalState gs(*console);

    vector<Offset2PosTest> cases = {{"hello", 0, 1, 1},
                                    {"line 1\nline 2", 1, 1, 2},
                                    {"line 1\nline 2", 7, 2, 1},
                                    {"line 1\nline 2", 11, 2, 5},
                                    {"a long line with no newlines\n", 20, 1, 21},
                                    {"line 1\nline 2\nline3\n", 7, 2, 1},
                                    {"line 1\nline 2\nline3", 6, 1, 7},
                                    {"line 1\nline 2\nline3", 7, 2, 1}};
    int i = 0;
    for (auto &tc : cases) {
        SCOPED_TRACE(string("case: ") + to_string(i));
        core::FileRef f = gs.enterFile(string(""), tc.src);

        auto detail = Loc::offset2Pos(f, tc.off, gs);

        EXPECT_EQ(tc.col, detail.column);
        EXPECT_EQ(tc.line, detail.line);
        i++;
    }
}

TEST(ASTTest, ErrorReporter) {
    core::GlobalState gs(*console);
    core::FileRef f = gs.enterFile(string("a/foo.rb"), string("def foo\n  hi\nend\n"));
    gs.errors.error(core::Loc{f, 0, 3}, core::ErrorClass::Internal, "Use of metavariable: {}", "foo");
    ASSERT_TRUE(gs.errors.hadCriticalError());
}

TEST(ASTTest, SymbolRef) {
    core::GlobalState gs(*console);
    core::SymbolRef ref = gs.defn_object();
    EXPECT_EQ(ref, ref.info(gs).ref(gs));
}

} // namespace core
} // namespace ruby_typer
