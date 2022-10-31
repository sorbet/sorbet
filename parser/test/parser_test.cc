#include "doctest.h"
// has to go first as it violates our requirements
#include "common/common.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "core/core.h"
#include "parser/Dedenter.h"
#include "parser/parser.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <optional>
#include <string>
#include <vector>

using namespace std;

auto logger = spdlog::stderr_color_mt("parser_test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

TEST_CASE("SimpleParse") { // NOLINT
    sorbet::core::GlobalState gs(errorQueue);
    gs.initEmpty();
    sorbet::core::UnfreezeNameTable nameTableAccess(gs);
    sorbet::core::UnfreezeFileTable ft(gs);
    auto settings = sorbet::parser::Parser::Settings{};
    sorbet::core::FileRef fileId1 = gs.enterFile("<test1>", "def hello_world; p :hello; end");
    sorbet::parser::Parser::run(gs, fileId1, settings);
    sorbet::core::FileRef fileId2 = gs.enterFile("<test2>", "class A; class B; end; end");
    sorbet::parser::Parser::run(gs, fileId2, settings);
    sorbet::core::FileRef fileId3 = gs.enterFile("<test3>", "class A::B; module B; end; end");
    sorbet::parser::Parser::run(gs, fileId3, settings);
}

struct DedentTest {
    int level;
    string_view in;
    string_view out;
};

TEST_CASE("TestDedent") { // NOLINT
    vector<DedentTest> cases = {
        {2, "    hi"sv, "  hi"sv},
        {10, "  \t    hi"sv, "  hi"sv},
        {2, "  a\n   b\n  c\n"sv, "a\n   b\n  c\n"sv},
        {4, "  a\n   b\n  c\n"sv, "a\n   b\n  c\n"sv},
    };
    for (auto &tc : cases) {
        sorbet::parser::Dedenter dedent(tc.level);
        optional<string> got = dedent.dedent(tc.in);
        CHECK(got.has_value());
        CHECK_EQ(*got, tc.out);
    }
}
