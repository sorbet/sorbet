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

    ast::TreePtr postTransformAssign(core::Context ctx, ast::TreePtr assign);
    ast::TreePtr preTransformMethodDef(core::Context ctx, ast::TreePtr methodDef);
    ast::TreePtr postTransformMethodDef(core::Context ctx, ast::TreePtr methodDef);
    ast::TreePtr preTransformClassDef(core::Context ctx, ast::TreePtr classDef);
    ast::TreePtr postTransformClassDef(core::Context ctx, ast::TreePtr classDef);

    const std::vector<core::LocalVariable> &result() const;
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LOCALVARFINDER_H
