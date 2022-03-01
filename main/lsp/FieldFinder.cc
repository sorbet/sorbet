#include "FieldFinder.h"
#include "ast/ArgParsing.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::realmain::lsp {

FieldFinder::FieldFinder(core::ClassOrModuleRef target, core::Loc queryLoc, ast::UnresolvedIdent::Kind queryKind)
    : targetClass(target), queryLoc(queryLoc), queryKind(queryKind)
{
    ENFORCE(queryKind != ast::UnresolvedIdent::Kind::Local);
}

ast::ExpressionPtr FieldFinder::postTransformUnresolvedIdent(core::Context ctx, ast::ExpressionPtr tree) {
    if (!this->insideSurroundingClass) {
        return tree;
    }

    auto &ident = ast::cast_tree_nonnull<ast::UnresolvedIdent>(tree);

    if (ident.kind != this->queryKind) {
        return tree;
    }

    this->result_.emplace_back(ident.name);
    return tree;
}

ast::ExpressionPtr FieldFinder::preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
    auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);

    ENFORCE(classDef.symbol.exists());
    ENFORCE(classDef.symbol != core::Symbols::todo());

    if (classDef.symbol == this->targetClass) {
        this->insideSurroundingClass = true;
    }

    return tree;
}

ast::ExpressionPtr FieldFinder::postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
    this->insideSurroundingClass = false;
    return tree;
}

const vector<core::NameRef> &FieldFinder::result() const {
    return this->result_;
}

}; // namespace sorbet::realmain::lsp
