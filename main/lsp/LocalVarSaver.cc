#include "LocalVarSaver.h"
#include "ast/Helpers.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet::realmain::lsp {
namespace {
core::MethodRef enclosingMethod(core::Context ctx) {
    core::MethodRef enclosingMethod;

    if (ctx.owner.isMethod()) {
        enclosingMethod = ctx.owner.asMethodRef();
    } else if (ctx.owner == core::Symbols::root()) {
        enclosingMethod = ctx.state.lookupStaticInitForFile(ctx.file);
    } else {
        enclosingMethod = ctx.state.lookupStaticInitForClass(ctx.owner.asClassOrModuleRef());
    }

    return enclosingMethod;
}
} // namespace

ast::ExpressionPtr LocalVarSaver::postTransformBlock(core::Context ctx, ast::ExpressionPtr tree) {
    auto &block = ast::cast_tree_nonnull<ast::Block>(tree);
    auto method = enclosingMethod(ctx);

    for (auto &arg : block.args) {
        if (auto *localExp = ast::MK::arg2Local(arg)) {
            bool lspQueryMatch = ctx.state.lspQuery.matchesVar(method, localExp->localVariable);
            if (lspQueryMatch) {
                core::TypeAndOrigins tp;
                core::lsp::QueryResponse::pushQueryResponse(
                    ctx,
                    core::lsp::IdentResponse(core::Loc(ctx.file, localExp->loc), localExp->localVariable, tp, method));
            }
        }
    }

    return tree;
}

ast::ExpressionPtr LocalVarSaver::postTransformLocal(core::Context ctx, ast::ExpressionPtr tree) {
    auto &local = ast::cast_tree_nonnull<ast::Local>(tree);
    auto method = enclosingMethod(ctx);

    bool lspQueryMatch = ctx.state.lspQuery.matchesVar(method, local.localVariable);
    if (lspQueryMatch) {
        // No need for type information; this is for a reference request.
        // Let the default constructor make tp.type an empty shared_ptr and tp.origins an empty vector
        core::TypeAndOrigins tp;
        core::lsp::QueryResponse::pushQueryResponse(
            ctx, core::lsp::IdentResponse(core::Loc(ctx.file, local.loc), local.localVariable, tp, method));
    }

    return tree;
}

ast::ExpressionPtr LocalVarSaver::postTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
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
