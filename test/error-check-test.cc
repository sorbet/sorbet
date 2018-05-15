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
auto errorQueue = std::make_shared<ruby_typer::core::ErrorQueue>(*logger, *logger);

namespace ruby_typer {

TEST(ErrorTest, RawCheck) { // NOLINT
    try {
        ENFORCE(false, "intentional failure");
    } catch (SRubyException &) {
    }
}

TEST(ErrorTest, ParserCheck) { // NOLINT
    ruby_typer::core::GlobalState gs(errorQueue);
    gs.initEmpty();
    ruby_typer::core::UnfreezeNameTable nt(gs);
    ruby_typer::core::UnfreezeSymbolTable st(gs);
    ruby_typer::core::UnfreezeFileTable ft(gs);
    ruby_typer::core::MutableContext ctx(gs, core::Symbols::root());
    auto ast = ruby_typer::parser::Parser::run(gs, "<test input>", "a");

    try {
        auto desugared = ruby_typer::ast::desugar::node2Tree(ctx, move(ast));
    } catch (SRubyException &) {
    }

    EXPECT_EQ(0, errorQueue->drainErrors().size());
}

} // namespace ruby_typer
