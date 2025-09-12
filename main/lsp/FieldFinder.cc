#include "FieldFinder.h"
#include "ast/ParamParsing.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::realmain::lsp {

FieldFinder::FieldFinder(core::ClassOrModuleRef target, ast::UnresolvedIdent::Kind queryKind,
                         vector<core::NameRef> &result)
    : targetClass(target), queryKind(queryKind), result_{result} {
    ENFORCE(queryKind != ast::UnresolvedIdent::Kind::Local);
}

void FieldFinder::postTransformUnresolvedIdent(core::Context ctx, const ast::UnresolvedIdent &ident) {
    ENFORCE(!this->classStack.empty());

    if (this->classStack.back() != this->targetClass) {
        return;
    }

    if (ident.kind != this->queryKind) {
        return;
    }

    if (ident.name == core::Names::ivarNameMissing() || ident.name == core::Names::cvarNameMissing()) {
        return;
    }

    this->result_.emplace_back(ident.name);
}

void FieldFinder::preTransformClassDef(core::Context ctx, const ast::ClassDef &classDef) {
    ENFORCE(classDef.symbol.exists());
    ENFORCE(classDef.symbol != core::Symbols::todo());

    this->classStack.push_back(classDef.symbol);
}

void FieldFinder::postTransformClassDef(core::Context ctx, const ast::ClassDef &classDef) {
    this->classStack.pop_back();
}

}; // namespace sorbet::realmain::lsp
