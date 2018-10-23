#include "gtest/gtest.h"
// has to go first as it violates are requirements
#include "common/common.h"
#include "core/BufferedErrorQueue.h"
#include "core/Errors.h"
#include "core/Unfreeze.h"
#include "core/core.h"
#include "parser/Dedenter.h"
#include "parser/parser.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <string>
#include <vector>

namespace spd = spdlog;
using sorbet::u4;
using namespace std;

auto logger = spd::stderr_color_mt("parser_test");
auto errorQueue = make_shared<sorbet::core::BufferedErrorQueue>(*logger, *logger);

TEST(ParserTest, SimpleParse) { // NOLINT
    sorbet::core::GlobalState gs(errorQueue);
    gs.initEmpty();
    sorbet::core::UnfreezeNameTable nameTableAccess(gs);
    sorbet::core::UnfreezeFileTable ft(gs);
    sorbet::parser::Parser::run(gs, "<test1>", "def hello_world; p :hello; end");
    sorbet::parser::Parser::run(gs, "<test2>", "class A; class B; end; end");
    sorbet::parser::Parser::run(gs, "<test3>", "class A::B; module B; end; end");
}

struct DedentTest {
    int level;
    string_view in;
    string_view out;
};

TEST(ParserTest, TestDedent) { // NOLINT
    vector<DedentTest> cases = {
        {2, "    hi"sv, "  hi"sv},
        {10, "  \t    hi"sv, "  hi"sv},
        {2, "  a\n   b\n  c\n"sv, "a\n   b\n  c\n"sv},
    };
    for (auto &tc : cases) {
        sorbet::parser::Dedenter dedent(tc.level);
        string got = dedent.dedent(tc.in);
        EXPECT_EQ(got, tc.out);
    }
}
