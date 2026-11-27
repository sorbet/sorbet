#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/desugar/prism/Desugar.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "parser/parser.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace std;

namespace sorbet::ast::Desugar::Prism {
namespace {

auto logger = spdlog::stderr_color_mt("prism_concrete_syntax_test");

void checkPreservedSyntax(string_view source) {
    auto errors = make_shared<core::ErrorQueue>(*logger, *logger);
    core::GlobalState gs(errors);
    gs.initEmpty();
    core::UnfreezeNameTable names(gs);
    core::UnfreezeFileTable files(gs);
    auto file = gs.enterFile("test.rb", string(source));
    core::MutableContext ctx(gs, core::Symbols::root(), file);
    auto original = desugar::node2Tree(ctx, parser::Parser::run(gs, file, {}).tree, true);
    auto prism = node2Tree(ctx, parser::Prism::Parser::run(ctx), true);

    // Source locations matter for deciding where Extract Variable can insert an
    // assignment; structural equality deliberately ignores those locations.
    CHECK_MESSAGE(prism.showRawWithLocs(gs, file) == original.showRawWithLocs(gs, file), "source: ", source,
                  "\noriginal: ", original.showRawWithLocs(gs, file), "\nPrism: ", prism.showRawWithLocs(gs, file));
}

} // namespace

TEST_CASE("Prism preserves compound assignment operands for refactoring") {
    for (auto lhs : {"local", "@instance", "@@class_variable", "$global", "object.value", "array[index]"}) {
        for (auto op : {"||=", "&&=", "+="}) {
            checkPreservedSyntax(fmt::format("{} {} rhs", lhs, op));
        }
    }
}

TEST_CASE("Prism preserves compound constant assignments during error recovery") {
    for (auto lhs : {"Constant", "Scope::Constant", "::Constant"}) {
        for (auto op : {"||=", "&&=", "+="}) {
            checkPreservedSyntax(fmt::format("{} {} rhs", lhs, op));
            checkPreservedSyntax(fmt::format("def method; {} {} rhs; end", lhs, op));
        }
    }
}

} // namespace sorbet::ast::Desugar::Prism
