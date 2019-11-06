#include "LocalVarFinder.h"
#include "ast/ArgParsing.h"

using namespace std;

namespace sorbet::realmain::lsp {

unique_ptr<ast::Assign> LocalVarFinder::postTransformAssign(core::Context ctx, unique_ptr<ast::Assign> assign) {
    // TODO(jez) We can't guarantee that methodStack is not empty, because we don't run on flattened trees
    // This means that querys to find the local variables of <static-init> methods will return no results.

    if (methodStack.empty()) {
        return assign;
    }

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

const vector<core::LocalVariable> &LocalVarFinder::result() const {
    ENFORCE(this->methodStack.empty());
    return this->result_;
}

}; // namespace sorbet::realmain::lsp
