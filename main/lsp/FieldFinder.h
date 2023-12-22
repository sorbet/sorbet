#ifndef RUBY_TYPER_LSP_FIELDFINDER_H
#define RUBY_TYPER_LSP_FIELDFINDER_H

#include "ast/ast.h"
#include <vector>

namespace sorbet::realmain::lsp {

class FieldFinder {
private:
    const core::ClassOrModuleRef targetClass;
    ast::UnresolvedIdent::Kind queryKind;

    std::vector<core::ClassOrModuleRef> classStack;

    std::vector<core::NameRef> result_;

public:
    FieldFinder(core::ClassOrModuleRef target, ast::UnresolvedIdent::Kind queryKind);

    void postTransformUnresolvedIdent(core::Context ctx, const ast::UnresolvedIdent &ident);
    void preTransformClassDef(core::Context ctx, const ast::ClassDef &classDef);
    void postTransformClassDef(core::Context ctx, const ast::ClassDef &classDef);

    const std::vector<core::NameRef> &result() const;
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_FIELDFINDER_H
