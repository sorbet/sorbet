#ifndef RUBY_TYPER_LSP_LOCALVARFINDER_H
#define RUBY_TYPER_LSP_LOCALVARFINDER_H

#include "ast/ast.h"
#include <vector>

namespace sorbet::realmain::lsp {

class LocalVarFinder {
    core::SymbolRef targetMethod;

    core::Loc queryLoc;

    std::vector<core::NameRef> &result_;

public:
    LocalVarFinder(core::SymbolRef targetMethod, core::Loc queryLoc, std::vector<core::NameRef> &result)
        : targetMethod(targetMethod), queryLoc(queryLoc), result_{result} {}

    void postTransformAssign(core::Context ctx, const ast::Assign &assign);
    void preTransformBlock(core::Context ctx, const ast::Block &block);
    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef);
    void postTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef);
    void preTransformClassDef(core::Context ctx, const ast::ClassDef &classDef);
    void postTransformClassDef(core::Context ctx, const ast::ClassDef &classDef);
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LOCALVARFINDER_H
