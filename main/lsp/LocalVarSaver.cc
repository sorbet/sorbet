#include "LocalVarSaver.h"
#include "ast/Helpers.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet::realmain::lsp {
ast::TreePtr LocalVarSaver::postTransformLocal(core::Context ctx, ast::TreePtr tree) {
    auto &local = ast::cast_tree_nonnull<ast::Local>(tree);

    core::MethodRef owner;
    if (ctx.owner.data(ctx)->isMethod()) {
        owner = ctx.owner.asMethodRef();
    } else if (ctx.owner == core::Symbols::root()) {
        owner = ctx.state.lookupStaticInitForFile(core::Loc(ctx.file, local.loc));
    } else {
        owner = ctx.state.lookupStaticInitForClass(ctx.owner.asClassOrModuleRef());
    }

    bool lspQueryMatch = ctx.state.lspQuery.matchesVar(owner, local.localVariable);
    if (lspQueryMatch) {
        // No need for type information; this is for a reference request.
        // Let the default constructor make tp.type an empty shared_ptr and tp.origins an empty vector
        core::TypeAndOrigins tp;

        core::MethodRef enclosingMethod;
        if (ctx.owner.isClassOrModule()) {
            enclosingMethod = ctx.owner == core::Symbols::root()
                                  ? ctx.state.lookupStaticInitForFile(core::Loc(ctx.file, local.loc))
                                  : ctx.state.lookupStaticInitForClass(ctx.owner.asClassOrModuleRef());
        } else {
            enclosingMethod = ctx.owner.asMethodRef();
        }

        core::lsp::QueryResponse::pushQueryResponse(
            ctx, core::lsp::IdentResponse(core::Loc(ctx.file, local.loc), local.localVariable, tp, enclosingMethod));
    }

    return tree;
}

ast::TreePtr LocalVarSaver::postTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
    auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);

    // Check args.
    for (auto &arg : methodDef.args) {
        // nullptrs should never happen, but guard against it anyway.
        if (auto *localExp = ast::MK::arg2Local(arg)) {
            bool lspQueryMatch = ctx.state.lspQuery.matchesVar(methodDef.symbol, localExp->localVariable);
            if (lspQueryMatch) {
                // (Ditto)
                core::TypeAndOrigins tp;
                core::lsp::QueryResponse::pushQueryResponse(
                    ctx, core::lsp::IdentResponse(core::Loc(ctx.file, localExp->loc), localExp->localVariable, tp,
                                                  methodDef.symbol));
            }
        }
    }

    return tree;
}
} // namespace sorbet::realmain::lsp
