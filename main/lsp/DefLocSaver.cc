#include "DefLocSaver.h"

using namespace std;
namespace sorbet {
namespace realmain {
namespace lsp {
unique_ptr<ast::MethodDef> DefLocSaver::postTransformMethodDef(core::Context ctx,
                                                               unique_ptr<ast::MethodDef> methodDef) {
    // This TypeAndOrigins object is currently unused so we just let the default constructor make tp.type
    // an empty shared_ptr and tp.origins an empty vector
    bool lspQueryMatch = ctx.state.lspInfoQueryLoc.exists() && methodDef->declLoc.contains(ctx.state.lspInfoQueryLoc);

    if (lspQueryMatch) {
        core::TypeAndOrigins tp;
        core::DispatchComponent dispatchComponent;
        core::DispatchResult::ComponentVec dispatchComponents;
        dispatchComponent.method = methodDef->symbol;
        dispatchComponents.emplace_back(move(dispatchComponent));
        tp.type = methodDef->symbol.data(ctx).resultType;
        core::QueryResponse::setQueryResponse(ctx, core::QueryResponse::Kind::DEFINITION, move(dispatchComponents),
                                              nullptr, methodDef->declLoc, methodDef->name, tp, tp);
    }

    return methodDef;
}
} // namespace lsp
} // namespace realmain
} // namespace sorbet
