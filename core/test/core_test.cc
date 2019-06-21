#include "gtest/gtest.h"
// has to go first as it violates are requirements
#include "core/Error.h"
#include "core/GlobalSubstitution.h"
#include "core/Unfreeze.h"
#include "core/core.h"
#include "core/errors/internal.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace spd = spdlog;
using namespace std;

namespace sorbet::core {
auto logger = spd::stderr_color_mt("parse");
auto errorQueue = make_shared<ErrorQueue>(*logger, *logger);

struct Offset2PosTest {
    string src;
    u4 off;
    u4 line;
    u4 col;
};

TEST(ASTTest, TestOffset2Pos) { // NOLINT
    GlobalState gs(errorQueue);
    gs.initEmpty();
    UnfreezeFileTable fileTableAccess(gs);

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
        auto name = string("case: ") + to_string(i);
        SCOPED_TRACE(name);
        FileRef f = gs.enterFile(move(name), tc.src);

        auto detail = Loc::offset2Pos(f.data(gs), tc.off);

        EXPECT_EQ(tc.col, detail.column);
        EXPECT_EQ(tc.line, detail.line);
        i++;
    }
}

TEST(ASTTest, Errors) { // NOLINT
    GlobalState gs(errorQueue);
    gs.initEmpty();
    UnfreezeFileTable fileTableAccess(gs);
    FileRef f = gs.enterFile(string("a/foo.rb"), string("def foo\n  hi\nend\n"));
    if (auto e = gs.beginError(Loc{f, 0, 3}, errors::Internal::InternalError)) {
        e.setHeader("Use of metavariable: `{}`", "foo");
    }
    ASSERT_TRUE(gs.hadCriticalError());
    auto errors = errorQueue->drainAllErrors();
    ASSERT_EQ(1, errors.size());
}

TEST(ASTTest, SymbolRef) { // NOLINT
    GlobalState gs(errorQueue);
    gs.initEmpty();
    SymbolRef ref = Symbols::Object();
    EXPECT_EQ(ref, ref.data(gs)->ref(gs));
}

struct FileIsTypedCase {
    string_view src;
    StrictLevel strict;
};

TEST(CoreTest, FileIsTyped) { // NOLINT
    vector<FileIsTypedCase> cases = {
        {"", StrictLevel::None},
        {"# typed: true", StrictLevel::True},
        {"\n# typed: true\n", StrictLevel::True},
        {"not a typed: pragma\n# typed: true\n", StrictLevel::True},
        {"typed:\n# typed: nonsense\n", StrictLevel::None},
        {"# typed: strict\n", StrictLevel::Strict},
        {"# typed: strong\n", StrictLevel::Strong},
        {"# typed: autogenerated\n", StrictLevel::Autogenerated},
        {"# typed: false\n", StrictLevel::False},
        {"# typed: lax\n", StrictLevel::None},
        {"# typed: ignore\n", StrictLevel::Ignore},
        {"#    typed:      true\n", StrictLevel::True},
        {"typed: true\n", StrictLevel::None},

        // We no longer support the old pragma
        {"# @typed", StrictLevel::None},
        {"\n# @typed\n", StrictLevel::None},
    };
    for (auto &tc : cases) {
        EXPECT_EQ(tc.strict, File::filePragma(tc.src));
    }
}

TEST(CoreTest, Substitute) { // NOLINT
    GlobalState gs1(errorQueue);
    gs1.initEmpty();

    GlobalState gs2(errorQueue);
    gs2.initEmpty();

    NameRef foo1, bar1, other1;
    NameRef foo2, bar2;
    {
        UnfreezeNameTable thaw1(gs1);
        UnfreezeNameTable thaw2(gs2);

        foo1 = gs1.enterNameUTF8("foo");
        bar1 = gs1.enterNameUTF8("bar");
        other1 = gs1.enterNameUTF8("other");

        foo2 = gs2.enterNameUTF8("foo");
        gs1.enterNameUTF8("name");
        bar2 = gs1.enterNameUTF8("bar");
    }

    GlobalSubstitution subst(gs1, gs2);

    EXPECT_EQ(subst.substitute(foo1), foo2);
    EXPECT_EQ(subst.substitute(bar1), bar2);

    auto other2 = subst.substitute(other1);
    ASSERT_TRUE(other2.exists());
    ASSERT_TRUE(other2.data(gs2)->kind == UTF8);
    ASSERT_EQ("<U other>", other2.showRaw(gs2));
}

TEST(CoreTest, LocTest) { // NOLINT
    constexpr auto maxFileId = 0xffff - 1;
    constexpr auto maxOffset = 0xffffff - 1;
    for (auto fileRef = 0; fileRef < maxFileId; fileRef = fileRef * 2 + 1) {
        for (auto beginPos = 0; beginPos < maxOffset; beginPos = beginPos * 2 + 1) {
            for (auto endPos = beginPos; endPos < maxOffset; endPos = endPos * 2 + 1) {
                Loc loc(core::FileRef(fileRef), beginPos, endPos);
                EXPECT_EQ(loc.file().id(), fileRef);
                EXPECT_EQ(loc.beginPos(), beginPos);
                EXPECT_EQ(loc.endPos(), endPos);
            }
        }
    }
    for (auto fileRef = 0; fileRef < maxFileId; fileRef = fileRef * 2 + 1) {
        for (auto beginPos = 0; beginPos < maxOffset; beginPos = beginPos * 2 + 1) {
            for (auto endPos = beginPos; endPos < maxOffset; endPos = endPos * 2 + 1) {
                Loc loc(core::FileRef(fileRef), beginPos, endPos);
                auto [low, high] = loc.getAs2u4();
                Loc loc2;
                loc2.setFrom2u4(low, high);
                EXPECT_EQ(loc.file().id(), loc2.file().id());
                EXPECT_EQ(loc.beginPos(), loc2.beginPos());
                EXPECT_EQ(loc.endPos(), loc2.endPos());
            }
        }
    }
}

} // namespace sorbet::core
