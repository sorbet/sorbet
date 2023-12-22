#ifndef RUBY_TYPER_LSP_NEXTMETHODFINDER_H
#define RUBY_TYPER_LSP_NEXTMETHODFINDER_H

#include "ast/ast.h"

namespace sorbet::realmain::lsp {

class NextMethodFinder {
    const core::Loc queryLoc;

    // Track the narrowest location range that still contains the queryLoc.
    //
    // If we find a method that's after queryLoc but it's not in this narrowest range,
    // it means we found a method that's outside the scope where the queryLoc was.
    core::Loc narrowestClassDefRange;

    // Track whether current scope has the queryLoc.
    std::vector<bool> scopeContainsQueryLoc;

    std::pair<core::Loc, core::MethodRef> result_;

public:
    NextMethodFinder(core::Loc queryLoc)
        : queryLoc(queryLoc), narrowestClassDefRange(core::Loc::none()), scopeContainsQueryLoc(std::vector<bool>{}),
          result_(core::Loc::none(), core::Symbols::noMethod()) {}

    void preTransformClassDef(core::Context ctx, const ast::ClassDef &tree);
    void postTransformClassDef(core::Context ctx, const ast::ClassDef &tree);
    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &tree);

    const core::MethodRef result() const;
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_NEXTMETHODFINDER_H
