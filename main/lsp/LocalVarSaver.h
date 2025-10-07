#ifndef RUBY_TYPER_LSP_IDENTSAVER_H
#define RUBY_TYPER_LSP_IDENTSAVER_H

#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"
#include "core/sig_finder/sig_finder.h"

namespace sorbet::realmain::lsp {

class LocalVarSaver {
    std::vector<core::Loc> enclosingMethodDefLoc;
    std::optional<resolver::ParsedSig> signature;
    // Technically this is redundant with what's in `ctx.state.lspQuery`, but we only ever run
    // `LocalVarSaver` if the lspQuery is currently holding a Query::Var. Rather than assert that at
    // runtime, we accept a `LocalVariable` when we construct this class so that the caller has to
    // prove they've already done that check.
    core::LocalVariable variable;

public:
    LocalVarSaver(core::Loc rootLoc, std::optional<resolver::ParsedSig> &&signature, core::LocalVariable variable)
        : enclosingMethodDefLoc({rootLoc}), signature(move(signature)), variable(variable) {}

    void postTransformBlock(core::Context ctx, const ast::Block &local);
    void postTransformLocal(core::Context ctx, const ast::Local &local);
    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef);
    void postTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef);
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_IDENTSAVER_H
