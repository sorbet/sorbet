#ifndef RUBY_TYPER_LSP_LOCALVARFINDER_H
#define RUBY_TYPER_LSP_LOCALVARFINDER_H

#include "ast/ast.h"
#include <vector>

namespace sorbet::realmain::lsp {

class LocalVarFinder {
    core::SymbolRef targetMethod;

    // We go through the effort of keeping track of a method stack so as to not rely on trees having been
    // flattened at this point. (LSP code should try to make minimal assumptions to be robust to changes.)
    std::vector<core::SymbolRef> methodStack;

    std::vector<core::LocalVariable> result_;

public:
    LocalVarFinder(core::SymbolRef targetMethod) : targetMethod(targetMethod) {}

    std::unique_ptr<ast::Assign> postTransformAssign(core::Context ctx, std::unique_ptr<ast::Assign> assign);
    std::unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, std::unique_ptr<ast::MethodDef> methodDef);
    std::unique_ptr<ast::MethodDef> postTransformMethodDef(core::Context ctx,
                                                           std::unique_ptr<ast::MethodDef> methodDef);
    std::unique_ptr<ast::ClassDef> preTransformClassDef(core::Context ctx, std::unique_ptr<ast::ClassDef> classDef);
    std::unique_ptr<ast::ClassDef> postTransformClassDef(core::Context ctx, std::unique_ptr<ast::ClassDef> classDef);

    const std::vector<core::LocalVariable> &result() const;
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LOCALVARFINDER_H
