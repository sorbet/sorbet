#ifndef RUBY_TYPER_LSP_IDENTSAVER_H
#define RUBY_TYPER_LSP_IDENTSAVER_H

#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {

class LocalVarSaver {
public:
    ast::ExpressionPtr postTransformBlock(core::Context ctx, ast::ExpressionPtr local);
    ast::ExpressionPtr postTransformLocal(core::Context ctx, ast::ExpressionPtr local);
    ast::ExpressionPtr postTransformMethodDef(core::Context ctx, ast::ExpressionPtr methodDef);
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_IDENTSAVER_H
