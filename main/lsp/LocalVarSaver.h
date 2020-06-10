#ifndef RUBY_TYPER_LSP_IDENTSAVER_H
#define RUBY_TYPER_LSP_IDENTSAVER_H

#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {

class LocalVarSaver {
public:
    ast::TreePtr postTransformLocal(core::Context ctx, ast::TreePtr local);
    ast::TreePtr postTransformMethodDef(core::Context ctx, ast::TreePtr methodDef);
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_IDENTSAVER_H
