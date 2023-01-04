#ifndef RUBY_TYPER_LSP_IDENTSAVER_H
#define RUBY_TYPER_LSP_IDENTSAVER_H

#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"
#include "main/sig_finder/sig_finder.h"

namespace sorbet::realmain::lsp {

class LocalVarSaver {
    std::optional<resolver::ParsedSig> signature;
    std::vector<core::Loc> enclosingMethodDefLoc;

public:
    LocalVarSaver(std::optional<resolver::ParsedSig> &&signature) : signature(move(signature)) {}

    void postTransformBlock(core::Context ctx, ast::ExpressionPtr &local);
    void postTransformLocal(core::Context ctx, ast::ExpressionPtr &local);
    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &methodDef);
    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &methodDef);
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_IDENTSAVER_H
