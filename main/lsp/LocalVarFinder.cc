#include "LocalVarFinder.h"
#include "ast/ArgParsing.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::realmain::lsp {

ast::ExpressionPtr LocalVarFinder::preTransformBlock(core::Context ctx, ast::ExpressionPtr tree) {
    ENFORCE(!methodStack.empty());

    auto &block = ast::cast_tree_nonnull<ast::Block>(tree);
    auto loc = core::Loc{ctx.file, block.loc};

    if (methodStack.back() != this->targetMethod) {
        return tree;
    }

    if (!loc.contains(this->queryLoc)) {
        return tree;
    }

    auto parsedArgs = ast::ArgParsing::parseArgs(block.args);
    for (const auto &parsedArg : parsedArgs) {
        this->result_.emplace_back(parsedArg.local._name);
    }

    return tree;
}

ast::ExpressionPtr LocalVarFinder::postTransformAssign(core::Context ctx, ast::ExpressionPtr tree) {
    ENFORCE(!methodStack.empty());

    auto &assign = ast::cast_tree_nonnull<ast::Assign>(tree);

    auto *local = ast::cast_tree<ast::Local>(assign.lhs);
    if (local == nullptr) {
        return tree;
    }

    if (methodStack.back() == this->targetMethod) {
        this->result_.emplace_back(local->localVariable._name);
    }

    return tree;
}

ast::ExpressionPtr LocalVarFinder::preTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
    auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);

    ENFORCE(methodDef.symbol.exists());
    ENFORCE(methodDef.symbol != core::Symbols::todoMethod());

    auto currentMethod = methodDef.symbol;

    if (currentMethod == this->targetMethod) {
        auto parsedArgs = ast::ArgParsing::parseArgs(methodDef.args);
        for (const auto &parsedArg : parsedArgs) {
            this->result_.emplace_back(parsedArg.local._name);
        }
    }

    this->methodStack.emplace_back(currentMethod);

    return tree;
}

ast::ExpressionPtr LocalVarFinder::postTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
    this->methodStack.pop_back();
    return tree;
}

ast::ExpressionPtr LocalVarFinder::preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
    auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
    ENFORCE(classDef.symbol.exists());
    ENFORCE(classDef.symbol != core::Symbols::todo());

    auto currentMethod = classDef.symbol == core::Symbols::root()
                             ? ctx.state.lookupStaticInitForFile(core::Loc(ctx.file, classDef.declLoc))
                             : ctx.state.lookupStaticInitForClass(classDef.symbol);

    this->methodStack.emplace_back(currentMethod);

    return tree;
}

ast::ExpressionPtr LocalVarFinder::postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
    this->methodStack.pop_back();
    return tree;
}

const vector<core::NameRef> &LocalVarFinder::result() const {
    ENFORCE(this->methodStack.empty());
    return this->result_;
}

}; // namespace sorbet::realmain::lsp
