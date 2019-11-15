#ifndef RUBY_TYPER_LSP_NEXTMETHODFINDER_H
#define RUBY_TYPER_LSP_NEXTMETHODFINDER_H

#include "ast/ast.h"

namespace sorbet::realmain::lsp {

class NextMethodFinder {
    const core::Loc queryLoc;

    core::SymbolRef result_;

public:
    NextMethodFinder(core::Loc queryLoc) : queryLoc(queryLoc), result_(core::Symbols::noSymbol()) {}

    std::unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, std::unique_ptr<ast::MethodDef> methodDef);

    const core::SymbolRef result() const;
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_NEXTMETHODFINDER_H
