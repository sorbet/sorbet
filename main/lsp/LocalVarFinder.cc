#include "LocalVarFinder.h"
#include "ast/ArgParsing.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::realmain::lsp {

void LocalVarFinder::preTransformBlock(core::Context ctx, const ast::Block &block) {
    ENFORCE(!methodStack.empty());
    auto loc = ctx.locAt(block.loc);

    if (methodStack.back() != this->targetMethod) {
        return;
    }

    if (!loc.contains(this->queryLoc)) {
        return;
    }

    auto parsedArgs = ast::ArgParsing::parseArgs(block.args);
    for (const auto &parsedArg : parsedArgs) {
        this->result_.emplace_back(parsedArg.local._name);
    }
}

void LocalVarFinder::postTransformAssign(core::Context ctx, const ast::Assign &assign) {
    ENFORCE(!methodStack.empty());

    auto *local = ast::cast_tree<ast::Local>(assign.lhs);
    if (local == nullptr) {
        return;
    }

    if (methodStack.back() == this->targetMethod) {
        this->result_.emplace_back(local->localVariable._name);
    }
}

void LocalVarFinder::preTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef) {
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
}

void LocalVarFinder::postTransformMethodDef(core::Context ctx, const ast::MethodDef &tree) {
    this->methodStack.pop_back();
}

void LocalVarFinder::preTransformClassDef(core::Context ctx, const ast::ClassDef &classDef) {
    ENFORCE(classDef.symbol.exists());
    ENFORCE(classDef.symbol != core::Symbols::todo());

    auto currentMethod = classDef.symbol == core::Symbols::root() ? ctx.state.lookupStaticInitForFile(ctx.file)
                                                                  : ctx.state.lookupStaticInitForClass(classDef.symbol);

    this->methodStack.emplace_back(currentMethod);
}

void LocalVarFinder::postTransformClassDef(core::Context ctx, const ast::ClassDef &tree) {
    this->methodStack.pop_back();
}

const vector<core::NameRef> &LocalVarFinder::result() const {
    ENFORCE(this->methodStack.empty());
    return this->result_;
}

}; // namespace sorbet::realmain::lsp
