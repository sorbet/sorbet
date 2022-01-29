#ifndef RUBY_TYPER_LSP_LOCALVARFINDER_H
#define RUBY_TYPER_LSP_LOCALVARFINDER_H

#include "ast/ast.h"
#include <vector>

namespace sorbet::realmain::lsp {

class LocalVarFinder {
    core::MethodRef targetMethod;

    core::Loc queryLoc;

    // We go through the effort of keeping track of a method stack so as to not rely on trees having been
    // flattened at this point. (LSP code should try to make minimal assumptions to be robust to changes.)
    std::vector<core::MethodRef> methodStack;

    std::vector<core::LocalVariable> result_;

public:
    LocalVarFinder(core::MethodRef targetMethod, core::Loc queryLoc) : targetMethod(targetMethod), queryLoc(queryLoc) {}

    ast::ExpressionPtr postTransformAssign(core::Context ctx, ast::ExpressionPtr assign);
    ast::ExpressionPtr preTransformMethodDef(core::Context ctx, ast::ExpressionPtr methodDef);
    ast::ExpressionPtr postTransformMethodDef(core::Context ctx, ast::ExpressionPtr methodDef);
    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr classDef);
    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr classDef);

    const std::vector<core::LocalVariable> &result() const;
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LOCALVARFINDER_H
