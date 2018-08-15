#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "common/common.h"
#include "core/BufferedErrorQueue.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "dsl/dsl.h"
#include "infer/infer.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "payload/text/text.h"
#include "resolver/resolver.h"
#include "spdlog/spdlog.h"

#include <iostream>

#include <emscripten.h>

using namespace std;
using namespace sorbet;

class CFG_Collector_and_Typer {
public:
    unique_ptr<sorbet::ast::MethodDef> preTransformMethodDef(sorbet::core::Context ctx,
                                                             unique_ptr<sorbet::ast::MethodDef> m) {
        if (m->symbol.data(ctx).isOverloaded()) {
            return m;
        }

        auto cfg = sorbet::cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);
        sorbet::infer::Inference::run(ctx.withOwner(m->symbol), move(cfg));
        return m;
    }
};

extern const sorbet::u1 *const getNameTablePayload;
std::shared_ptr<spdlog::logger> logger = spd::stderr_logger_st("logger");

extern "C" {
void EMSCRIPTEN_KEEPALIVE typecheck(const char *rubySrc) {
    auto errorQueue = std::make_shared<sorbet::core::BufferedErrorQueue>(*logger, *logger);
    core::ErrorColors::enableColors();
    sorbet::core::GlobalState gs(errorQueue);
    core::MutableContext ctx(gs, core::Symbols::root());

    core::serialize::Serializer::loadGlobalState(gs, getNameTablePayload);

    core::UnfreezeNameTable nameTableAccess(gs);
    core::UnfreezeSymbolTable symbolTableAccess(gs);
    core::UnfreezeFileTable fileTableAccess(gs);

    auto file = gs.enterFile("<ruby>", rubySrc);
    file.data(gs).strict = core::StrictLevel::Typed;
    auto node = parser::Parser::run(ctx.state, file);
    auto ast = ast::desugar::node2Tree(ctx, move(node));
    auto dslsInlined = dsl::DSL::run(ctx, move(ast));
    auto named = namer::Namer::run(ctx, move(dslsInlined));
    vector<unique_ptr<ast::Expression>> trees;
    trees.emplace_back(move(named));
    trees = resolver::Resolver::run(ctx, move(trees));

    for (auto &resolvedTree : trees) {
        CFG_Collector_and_Typer collector;
        sorbet::ast::TreeMap::apply(ctx, collector, move(resolvedTree));
        resolvedTree.reset();
    }

    for (auto &err : errorQueue->drainAllErrors()) {
        logger->log(spdlog::level::err, "{}", err->toString(gs));
    }
}

int main(int argc, char **argv) {}
}
