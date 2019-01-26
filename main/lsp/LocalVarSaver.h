#ifndef RUBY_TYPER_LSP_IDENTSAVER_H
#define RUBY_TYPER_LSP_IDENTSAVER_H

#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {

class LocalVarSaver {
public:
    std::unique_ptr<ast::Local> postTransformLocal(core::Context ctx, std::unique_ptr<ast::Local> local);
    std::unique_ptr<ast::MethodDef> postTransformMethodDef(core::Context ctx,
                                                           std::unique_ptr<ast::MethodDef> methodDef);
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_IDENTSAVER_H