#include "FieldFinder.h"
#include "ast/ArgParsing.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::realmain::lsp {

FieldFinder::FieldFinder(core::ClassOrModuleRef target, ast::UnresolvedIdent::Kind queryKind)
    : targetClass(target), queryKind(queryKind) {
    ENFORCE(queryKind != ast::UnresolvedIdent::Kind::Local);
}

ast::ExpressionPtr FieldFinder::postTransformUnresolvedIdent(core::Context ctx, ast::ExpressionPtr tree) {
    ENFORCE(!this->classStack.empty());

    if (this->classStack.back() != this->targetClass) {
        return tree;
    }

    auto &ident = ast::cast_tree_nonnull<ast::UnresolvedIdent>(tree);

    if (ident.kind != this->queryKind) {
        return tree;
    }

    if (ident.name == core::Names::ivarNameMissing() || ident.name == core::Names::cvarNameMissing()) {
        return tree;
    }

    this->result_.emplace_back(ident.name);
    return tree;
}

ast::ExpressionPtr FieldFinder::preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
    auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);

    ENFORCE(classDef.symbol.exists());
    ENFORCE(classDef.symbol != core::Symbols::todo());

    this->classStack.push_back(classDef.symbol);

    return tree;
}

ast::ExpressionPtr FieldFinder::postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
    this->classStack.pop_back();
    return tree;
}

const vector<core::NameRef> &FieldFinder::result() const {
    return this->result_;
}

}; // namespace sorbet::realmain::lsp
