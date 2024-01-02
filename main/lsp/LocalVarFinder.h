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

    std::vector<core::NameRef> result_;

public:
    LocalVarFinder(core::MethodRef targetMethod, core::Loc queryLoc) : targetMethod(targetMethod), queryLoc(queryLoc) {}

    void postTransformAssign(core::Context ctx, const ast::Assign &assign);
    void preTransformBlock(core::Context ctx, const ast::Block &block);
    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef);
    void postTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef);
    void preTransformClassDef(core::Context ctx, const ast::ClassDef &classDef);
    void postTransformClassDef(core::Context ctx, const ast::ClassDef &classDef);

    const std::vector<core::NameRef> &result() const;
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LOCALVARFINDER_H
