#include "LocalVarSaver.h"
#include "ast/Helpers.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet::realmain::lsp {
unique_ptr<ast::Local> LocalVarSaver::postTransformLocal(core::Context ctx, unique_ptr<ast::Local> local) {
    core::SymbolRef owner;
    if (ctx.owner.data(ctx)->isMethod()) {
        owner = ctx.owner;
    } else if (ctx.owner == core::Symbols::root()) {
        owner = ctx.state.lookupStaticInitForFile(core::Loc(ctx.file, local->loc));
    } else {
        ENFORCE(ctx.owner.data(ctx)->isClassOrModule());
        owner = ctx.state.lookupStaticInitForClass(ctx.owner);
    }

    bool lspQueryMatch = ctx.state.lspQuery.matchesVar(owner, local->localVariable);
    if (lspQueryMatch) {
        // No need for type information; this is for a reference request.
        // Let the default constructor make tp.type an empty shared_ptr and tp.origins an empty vector
        core::TypeAndOrigins tp;

        auto enclosingMethod = ctx.owner;
        if (enclosingMethod.data(ctx)->isClassOrModule()) {
            enclosingMethod = ctx.owner == core::Symbols::root()
                                  ? ctx.state.lookupStaticInitForFile(core::Loc(ctx.file, local->loc))
                                  : ctx.state.lookupStaticInitForClass(ctx.owner);
        }

        core::lsp::QueryResponse::pushQueryResponse(
            ctx, core::lsp::IdentResponse(core::Loc(ctx.file, local->loc), local->localVariable, tp, enclosingMethod));
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
                    ctx, core::lsp::IdentResponse(core::Loc(ctx.file, localExp->loc), localExp->localVariable, tp,
                                                  methodDef->symbol));
            }
        }
    }

    return methodDef;
}
} // namespace sorbet::realmain::lsp
