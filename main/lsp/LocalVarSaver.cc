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

void LocalVarSaver::postTransformBlock(core::Context ctx, const ast::Block &block) {
    auto method = enclosingMethod(ctx);

    for (auto &arg : block.args) {
        if (auto *localExp = ast::MK::arg2Local(arg)) {
            bool lspQueryMatch = ctx.state.lspQuery.matchesVar(method, localExp->localVariable);
            if (lspQueryMatch) {
                core::TypeAndOrigins tp;
                core::lsp::QueryResponse::pushQueryResponse(
                    ctx, core::lsp::IdentResponse(ctx.locAt(localExp->loc), localExp->localVariable, tp, method,
                                                  this->enclosingMethodDefLoc.back()));
            }
        }
    }
}

void LocalVarSaver::postTransformLocal(core::Context ctx, const ast::Local &local) {
    auto method = enclosingMethod(ctx);

    bool lspQueryMatch = ctx.state.lspQuery.matchesVar(method, local.localVariable);
    if (lspQueryMatch) {
        // No need for type information; this is for a reference request.
        // Let the default constructor make tp.type an empty shared_ptr and tp.origins an empty vector
        core::TypeAndOrigins tp;
        core::lsp::QueryResponse::pushQueryResponse(ctx, core::lsp::IdentResponse(ctx.locAt(local.loc),
                                                                                  local.localVariable, tp, method,
                                                                                  this->enclosingMethodDefLoc.back()));
    }
}

void LocalVarSaver::preTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef) {
    this->enclosingMethodDefLoc.emplace_back(ctx.locAt(methodDef.loc));
}

void LocalVarSaver::postTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef) {
    this->enclosingMethodDefLoc.pop_back();

    // Check args.
    for (auto &arg : methodDef.args) {
        // nullptrs should never happen, but guard against it anyway.
        if (auto *localExp = ast::MK::arg2Local(arg)) {
            bool lspQueryMatch = ctx.state.lspQuery.matchesVar(methodDef.symbol, localExp->localVariable);
            if (lspQueryMatch) {
                auto methodDefLoc = ctx.locAt(methodDef.loc);
                core::TypeAndOrigins tp;
                core::lsp::QueryResponse::pushQueryResponse(
                    ctx, core::lsp::IdentResponse(ctx.locAt(localExp->loc), localExp->localVariable, tp,
                                                  methodDef.symbol, methodDefLoc));

                if (this->signature.has_value()) {
                    auto it = absl::c_find_if(this->signature->argTypes, [&](const auto &argSpec) {
                        return argSpec.name == ctx.state.lspQuery.variable._name;
                    });
                    if (it != this->signature->argTypes.end()) {
                        core::lsp::QueryResponse::pushQueryResponse(
                            ctx, core::lsp::IdentResponse(it->nameLoc, localExp->localVariable, tp, methodDef.symbol,
                                                          methodDefLoc));
                    }
                }
            }
        }
    }
}
} // namespace sorbet::realmain::lsp
