#ifndef RUBY_TYPER_LSP_IDENTSAVER_H
#define RUBY_TYPER_LSP_IDENTSAVER_H

#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {

class LocalVarSaver {
public:
    void postTransformBlock(core::Context ctx, ast::ExpressionPtr &local);
    void postTransformLocal(core::Context ctx, ast::ExpressionPtr &local);
    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &methodDef);
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_IDENTSAVER_H
