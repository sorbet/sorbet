#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"

using namespace std;

namespace spd = spdlog;

auto logger = spd::stderr_color_mt("error-check-test");
auto errorQueue = std::make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

namespace sorbet {

TEST(ErrorTest, RawCheck) { // NOLINT
    try {
        ENFORCE(false, "intentional failure");
    } catch (SRubyException &) {
    }
}

TEST(ErrorTest, ParserCheck) { // NOLINT
    sorbet::core::GlobalState gs(errorQueue);
    gs.initEmpty();
    sorbet::core::UnfreezeNameTable nt(gs);
    sorbet::core::UnfreezeSymbolTable st(gs);
    sorbet::core::UnfreezeFileTable ft(gs);
    sorbet::core::MutableContext ctx(gs, core::Symbols::root());
    auto ast = sorbet::parser::Parser::run(gs, "<test input>", "a");

    try {
        auto desugared = sorbet::ast::desugar::node2Tree(ctx, move(ast));
    } catch (SRubyException &) {
    }

    EXPECT_EQ(0, errorQueue->drainErrors().size());
}

} // namespace sorbet
