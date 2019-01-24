#include "DefLocSaver.h"
#include "core/lsp/Query.h"
#include "core/lsp/QueryResponse.h"

using namespace std;
namespace sorbet::realmain::lsp {

unique_ptr<ast::MethodDef> DefLocSaver::postTransformMethodDef(core::Context ctx,
                                                               unique_ptr<ast::MethodDef> methodDef) {
    const core::lsp::Query &lspQuery = ctx.state.lspQuery;
    bool lspQueryMatch = lspQuery.matchesLoc(methodDef->declLoc);

    if (lspQueryMatch) {
        core::TypeAndOrigins tp;
        core::DispatchComponent dispatchComponent;
        core::DispatchResult::ComponentVec dispatchComponents;
        dispatchComponent.method = methodDef->symbol;
        dispatchComponents.emplace_back(std::move(dispatchComponent));
        tp.type = methodDef->symbol.data(ctx)->resultType;
        core::lsp::QueryResponse::pushQueryResponse(
            ctx, core::lsp::DefinitionResponse(std::move(dispatchComponents), methodDef->declLoc, methodDef->name, tp));
    }

    return methodDef;
}
} // namespace sorbet::realmain::lsp
