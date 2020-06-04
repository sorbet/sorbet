#include "LocalVarFinder.h"
#include "ast/ArgParsing.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::realmain::lsp {

ast::TreePtr LocalVarFinder::postTransformAssign(core::Context ctx, ast::TreePtr tree) {
    ENFORCE(!methodStack.empty());

    auto &assign = ast::cast_tree_nonnull<ast::Assign>(tree);

    auto *local = ast::cast_tree<ast::Local>(assign.lhs);
    if (local == nullptr) {
        return tree;
    }

    if (methodStack.back() == this->targetMethod) {
        this->result_.emplace_back(local->localVariable);
    }

    return tree;
}

ast::TreePtr LocalVarFinder::preTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
    auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);

    ENFORCE(methodDef.symbol.exists());
    ENFORCE(methodDef.symbol != core::Symbols::todo());

    auto currentMethod = methodDef.symbol;

    if (currentMethod == this->targetMethod) {
        auto parsedArgs = ast::ArgParsing::parseArgs(methodDef.args);
        for (const auto &parsedArg : parsedArgs) {
            this->result_.emplace_back(parsedArg.local);
        }
    }

    this->methodStack.emplace_back(currentMethod);

    return tree;
}

ast::TreePtr LocalVarFinder::postTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
    this->methodStack.pop_back();
    return tree;
}

ast::TreePtr LocalVarFinder::preTransformClassDef(core::Context ctx, ast::TreePtr tree) {
    auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
    ENFORCE(classDef.symbol.exists());
    ENFORCE(classDef.symbol != core::Symbols::todo());

    auto currentMethod = classDef.symbol == core::Symbols::root() ? ctx.state.lookupStaticInitForFile(classDef.declLoc)
                                                                  : ctx.state.lookupStaticInitForClass(classDef.symbol);

    this->methodStack.emplace_back(currentMethod);

    return tree;
}

ast::TreePtr LocalVarFinder::postTransformClassDef(core::Context ctx, ast::TreePtr tree) {
    this->methodStack.pop_back();
    return tree;
}

const vector<core::LocalVariable> &LocalVarFinder::result() const {
    ENFORCE(this->methodStack.empty());
    return this->result_;
}

}; // namespace sorbet::realmain::lsp
