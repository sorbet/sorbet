#include "DefLocSaver.h"

using namespace std;
namespace sorbet::realmain::lsp {

void pushDefinitionResponse(core::Context ctx, core::DispatchResult::ComponentVec dispatchComponents, core::Loc termLoc,
                            core::NameRef name, core::TypeAndOrigins tp) {
    core::QueryResponse::pushQueryResponse(ctx, core::QueryResponse::Kind::DEFINITION, ctx.owner,
                                           std::move(dispatchComponents), nullptr, termLoc, core::LocalVariable(), name,
                                           tp, tp);
}

unique_ptr<ast::MethodDef> DefLocSaver::postTransformMethodDef(core::Context ctx,
                                                               unique_ptr<ast::MethodDef> methodDef) {
    const core::Query &lspQuery = ctx.state.lspQuery;
    bool lspQueryMatch = lspQuery.matchesLoc(methodDef->declLoc);

    if (lspQueryMatch) {
        core::TypeAndOrigins tp;
        core::DispatchComponent dispatchComponent;
        core::DispatchResult::ComponentVec dispatchComponents;
        dispatchComponent.method = methodDef->symbol;
        dispatchComponents.emplace_back(std::move(dispatchComponent));
        tp.type = methodDef->symbol.data(ctx)->resultType;
        pushDefinitionResponse(ctx, std::move(dispatchComponents), methodDef->declLoc, methodDef->name, tp);
    }

    return methodDef;
}
} // namespace sorbet::realmain::lsp
