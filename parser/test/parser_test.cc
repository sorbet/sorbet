#include "common/common.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "core/core.h"
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

auto logger = spd::stderr_color_mt("parser_test");
auto errorQueue = std::make_shared<ruby_typer::core::ErrorQueue>(*logger);

TEST(ParserTest, SimpleParse) { // NOLINT
    ruby_typer::core::GlobalState gs(errorQueue);
    gs.initEmpty();
    ruby_typer::core::UnfreezeNameTable nameTableAccess(gs);
    ruby_typer::core::UnfreezeFileTable ft(gs);
    ruby_typer::parser::Parser::run(gs, "<test>", "def hello_world; p :hello; end");
    ruby_typer::parser::Parser::run(gs, "<test>", "class A; class B; end; end");
    ruby_typer::parser::Parser::run(gs, "<test>", "class A::B; module B; end; end");
}

struct DedentTest {
    int level;
    string in;
    string out;
};

TEST(ParserTest, TestDedent) { // NOLINT
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
