#include "doctest.h"
// has to go first as it violates our requirements
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
// has to come before the next one. This comment stops formatter from reordering them
#include "spdlog/sinks/stdout_color_sinks.h"
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

auto logger = spdlog::stderr_color_mt("desugar_test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

TEST_CASE("SimpleDesugar") { // NOLINT
    sorbet::core::GlobalState gs(errorQueue);
    gs.initEmpty();
    sorbet::core::UnfreezeNameTable nameTableAccess(gs);
    sorbet::core::UnfreezeFileTable ft(gs);

    sorbet::core::FileRef fileId = gs.enterFile("<test>", "def hello_world; p :hello; end");
    auto settings = sorbet::parser::Parser::Settings{};
    auto ast = sorbet::parser::Parser::run(gs, fileId, settings);
    sorbet::core::MutableContext ctx(gs, sorbet::core::Symbols::root(), fileId);
    auto o1 = sorbet::ast::desugar::node2Tree(ctx, move(ast));
}
