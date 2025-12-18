#include "LocalVarFinder.h"
#include "ast/ParamParsing.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::realmain::lsp {

void LocalVarFinder::preTransformBlock(core::Context ctx, const ast::Block &block) {
    auto loc = ctx.locAt(block.loc);

    if (ctx.owner != this->targetMethod) {
        return;
    }

    if (!loc.contains(this->queryLoc)) {
        return;
    }

    auto parsedParams = ast::ParamParsing::parseParams(block.params);
    for (const auto &parsedParam : parsedParams) {
        this->result_.emplace_back(parsedParam.local._name);
    }
}

void LocalVarFinder::postTransformAssign(core::Context ctx, const ast::Assign &assign) {
    auto local = ast::cast_tree<ast::Local>(assign.lhs);
    if (local == nullptr) {
        return;
    }

    if (ctx.owner == this->targetMethod) {
        this->result_.emplace_back(local->localVariable._name);
    }
}

void LocalVarFinder::preTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef) {
    ENFORCE(methodDef.symbol.exists());
    ENFORCE(methodDef.symbol != core::Symbols::todoMethod());

    if (ctx.owner == this->targetMethod) {
        auto parsedParams = ast::ParamParsing::parseParams(methodDef.params);
        for (const auto &parsedParam : parsedParams) {
            this->result_.emplace_back(parsedParam.local._name);
        }
    }
}

}; // namespace sorbet::realmain::lsp
