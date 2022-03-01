#ifndef RUBY_TYPER_LSP_FIELDFINDER_H
#define RUBY_TYPER_LSP_FIELDFINDER_H

#include "ast/ast.h"
#include <vector>

namespace sorbet::realmain::lsp {

class FieldFinder {
private:
    const core::ClassOrModuleRef targetClass;
    core::Loc queryLoc;
    ast::UnresolvedIdent::Kind queryKind;

    bool insideSurroundingClass = false;

    std::vector<core::NameRef> result_;

public:
    FieldFinder(core::ClassOrModuleRef target, core::Loc queryLoc, ast::UnresolvedIdent::Kind queryKind);

    ast::ExpressionPtr postTransformUnresolvedIdent(core::Context ctx, ast::ExpressionPtr ident);
    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr classDef);
    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr classDef);

    const std::vector<core::NameRef> &result() const;
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_FIELDFINDER_H
