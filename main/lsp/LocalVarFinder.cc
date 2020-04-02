#include "LocalVarFinder.h"
#include "ast/ArgParsing.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::realmain::lsp {

unique_ptr<ast::Assign> LocalVarFinder::postTransformAssign(core::Context ctx, unique_ptr<ast::Assign> assign) {
    ENFORCE(!methodStack.empty());

    auto *local = ast::cast_tree<ast::Local>(assign->lhs.get());
    if (local == nullptr) {
        return assign;
    }

    if (methodStack.back() == this->targetMethod) {
        this->result_.emplace_back(local->localVariable);
    }

    return assign;
}

unique_ptr<ast::MethodDef> LocalVarFinder::preTransformMethodDef(core::Context ctx,
                                                                 unique_ptr<ast::MethodDef> methodDef) {
    ENFORCE(methodDef->symbol.exists());
    ENFORCE(methodDef->symbol != core::Symbols::todo());

    auto currentMethod = methodDef->symbol;

    if (currentMethod == this->targetMethod) {
        auto parsedArgs = ast::ArgParsing::parseArgs(ctx, methodDef->args);
        for (const auto &parsedArg : parsedArgs) {
            this->result_.emplace_back(parsedArg.local);
        }
    }

    this->methodStack.emplace_back(currentMethod);

    return methodDef;
}

unique_ptr<ast::MethodDef> LocalVarFinder::postTransformMethodDef(core::Context ctx,
                                                                  unique_ptr<ast::MethodDef> methodDef) {
    this->methodStack.pop_back();
    return methodDef;
}

unique_ptr<ast::ClassDef> LocalVarFinder::preTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> classDef) {
    ENFORCE(classDef->symbol.exists());
    ENFORCE(classDef->symbol != core::Symbols::todo());

    auto currentMethod = classDef->symbol == core::Symbols::root()
                             ? ctx.state.lookupStaticInitForFile(classDef->declLoc)
                             : ctx.state.lookupStaticInitForClass(classDef->symbol);

    this->methodStack.emplace_back(currentMethod);

    return classDef;
}

unique_ptr<ast::ClassDef> LocalVarFinder::postTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> classDef) {
    this->methodStack.pop_back();
    return classDef;
}

const vector<core::LocalVariable> &LocalVarFinder::result() const {
    ENFORCE(this->methodStack.empty());
    return this->result_;
}

}; // namespace sorbet::realmain::lsp
