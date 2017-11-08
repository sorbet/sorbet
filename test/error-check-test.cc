#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"

using namespace std;

namespace ruby_typer {

TEST(ErrorTest, RawCheck) {
    try {
        Error::check(false);
    } catch (...) {
    }
}

TEST(ErrorTest, ParserCheck) {
    auto console = spdlog::stderr_color_mt("Error Test");
    ruby_typer::ast::GlobalState gs(*console);
    ruby_typer::ast::Context context(gs, gs.defn_root());
    auto ast = ruby_typer::parser::Parser::run(gs, "<test input>", "a");
    ast->loc = ast::Loc::none(0);

    try {
        auto desugared = ruby_typer::ast::desugar::node2Tree(context, ast);
    } catch (...) {
    }
}

} // namespace ruby_typer
