#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "core/Unfreeze.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"

using namespace std;

namespace ruby_typer {

TEST(ErrorTest, RawCheck) { // NOLINT
    try {
        ENFORCE(false, "intentional failure");
    } catch (...) {
    }
}

TEST(ErrorTest, ParserCheck) { // NOLINT
    auto console = spdlog::stderr_color_mt("Error Test");
    ruby_typer::core::GlobalState gs(*console);
    gs.initEmpty();
    ruby_typer::core::UnfreezeNameTable nt(gs);
    ruby_typer::core::UnfreezeSymbolTable st(gs);
    ruby_typer::core::UnfreezeFileTable ft(gs);
    ruby_typer::core::Context context(gs, gs.defn_root());
    auto ast = ruby_typer::parser::Parser::run(gs, "<test input>", "a");

    try {
        auto desugared = ruby_typer::ast::desugar::node2Tree(context, move(ast));
    } catch (...) {
    }

    EXPECT_EQ(0, gs.drainErrors().size());
}

} // namespace ruby_typer
