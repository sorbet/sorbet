#include "LocalVarSaver.h"
#include "ast/Helpers.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet::realmain::lsp {
unique_ptr<ast::Local> LocalVarSaver::postTransformLocal(core::Context ctx, unique_ptr<ast::Local> local) {
    bool lspQueryMatch = ctx.state.lspQuery.matchesVar(ctx.owner, local->localVariable);
    if (lspQueryMatch) {
        // No need for type information; this is for a reference request.
        // Let the default constructor make tp.type an empty shared_ptr and tp.origins an empty vector
        core::TypeAndOrigins tp;
        core::lsp::QueryResponse::pushQueryResponse(
            ctx, core::lsp::IdentResponse(ctx.owner, local->loc, local->localVariable, tp));
    }

    return local;
}

unique_ptr<ast::MethodDef> LocalVarSaver::postTransformMethodDef(core::Context ctx,
                                                                 unique_ptr<ast::MethodDef> methodDef) {
    // Check args.
    for (auto &arg : methodDef->args) {
        // nullptrs should never happen, but guard against it anyway.
        if (auto *localExp = ast::MK::arg2Local(arg.get())) {
            bool lspQueryMatch = ctx.state.lspQuery.matchesVar(methodDef->symbol, localExp->localVariable);
            if (lspQueryMatch) {
                // (Ditto)
                core::TypeAndOrigins tp;
                core::lsp::QueryResponse::pushQueryResponse(
                    ctx, core::lsp::IdentResponse(methodDef->symbol, localExp->loc, localExp->localVariable, tp));
            }
        }
    }

    return methodDef;
}
} // namespace sorbet::realmain::lsp