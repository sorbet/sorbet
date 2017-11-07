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
    auto parsed = ruby_typer::parser::parse_ruby(gs, "<test input>", "a");
    parsed.ast()->loc = ast::Loc::none(0);

    try {
        auto desugared = ruby_typer::ast::desugar::node2Tree(context, parsed.ast());
    } catch (...) {
    }
}

} // namespace ruby_typer
