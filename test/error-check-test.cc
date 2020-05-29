#include "doctest.h"
// has to go first as it violates our requirements

#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "parser/parser.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace std;

namespace spd = spdlog;

auto logger = spd::stderr_color_mt("error-check-test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

namespace sorbet {

TEST_CASE("RawCheck") {
    try {
        ENFORCE(false, "intentional failure");
        CHECK(false);
    } catch (SorbetException &) {
    }
}

TEST_CASE("ParserCheck") {
    sorbet::core::GlobalState gs(errorQueue);
    gs.initEmpty();
    sorbet::core::UnfreezeNameTable nt(gs);
    sorbet::core::UnfreezeSymbolTable st(gs);
    sorbet::core::UnfreezeFileTable ft(gs);

    core::FileRef fileId = gs.enterFile("<test input>", "a");
    auto ast = sorbet::parser::Parser::run(gs, fileId);

    try {
        sorbet::core::MutableContext ctx(gs, core::Symbols::root(), fileId);
        auto desugared = sorbet::ast::desugar::node2Tree(ctx, move(ast));
    } catch (SorbetException &) {
    }

    CHECK_EQ(0, errorQueue->drainAllErrors().size());
}

} // namespace sorbet
