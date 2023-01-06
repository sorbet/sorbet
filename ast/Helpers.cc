#include "ast/Helpers.h"

using namespace std;

namespace sorbet::ast {

bool definesBehavior(const ExpressionPtr &expr) {
    if (BehaviorHelpers::checkEmptyDeep(expr)) {
        return false;
    }
    bool result = true;

    typecase(
        expr,

        [&](const ast::ClassDef &klass) {
            auto *id = ast::cast_tree<ast::UnresolvedIdent>(klass.name);
            if (id && id->name == core::Names::singleton()) {
                // class << self; We consider this
                // behavior-defining. We could opt to recurse inside
                // the inner class, but we consider there to be no
                // valid use of `class << self` solely for namespacing,
                // so there's no need to support that use case.
                result = true;
            } else {
                result = false;
            }
        },

        [&](const ast::Assign &asgn) {
            // this check can fire before the namer converts lhs constants in assignments from UnresolvedConstantLit ->
            // ConstantLit, so we have to allow for both types.
            if (ast::isa_tree<ast::ConstantLit>(asgn.lhs) || ast::isa_tree<ast::UnresolvedConstantLit>(asgn.lhs)) {
                result = false;
            } else {
                result = true;
            }
        },

        [&](const ast::InsSeq &seq) {
            result = absl::c_any_of(seq.stats, [](auto &child) { return definesBehavior(child); }) ||
                     definesBehavior(seq.expr);
        },

        // Ignore code synthesized by Rewriter pass.
        [&](const ast::Send &send) { result = !send.flags.isRewriterSynthesized; },
        [&](const ast::MethodDef &methodDef) { result = !methodDef.flags.isRewriterSynthesized; },
        [&](const ast::Literal &methodDef) { result = false; },

        [&](const ExpressionPtr &klass) { result = true; });
    return result;
}

bool BehaviorHelpers::checkClassDefinesBehavior(const ExpressionPtr &expr) {
    auto *klass = ast::cast_tree<ast::ClassDef>(expr);
    ENFORCE(klass);
    return BehaviorHelpers::checkClassDefinesBehavior(*klass);
}

bool BehaviorHelpers::checkClassDefinesBehavior(const ast::ClassDef &klass) {
    for (auto &ancst : klass.ancestors) {
        auto *cnst = ast::cast_tree<ast::ConstantLit>(ancst);
        if (cnst && cnst->original != nullptr) {
            return true;
        }
    }
    for (auto &ancst : klass.singletonAncestors) {
        auto *cnst = ast::cast_tree<ast::ConstantLit>(ancst);
        if (cnst && cnst->original != nullptr) {
            return true;
        }
    }
    return absl::c_any_of(klass.rhs, [](auto &tree) { return definesBehavior(tree); });
}

bool BehaviorHelpers::checkEmptyDeep(const ExpressionPtr &expr) {
    bool result = false;

    typecase(
        expr,

        [&](const ast::Send &send) {
            result = send.fun == core::Names::keepForIde() || send.fun == core::Names::keepDef() ||
                     send.fun == core::Names::keepSelfDef() || send.fun == core::Names::include() ||
                     send.fun == core::Names::extend();
        },

        [&](const ast::EmptyTree &) { result = true; },

        [&](const ast::InsSeq &seq) {
            result = absl::c_all_of(seq.stats, [](auto &child) { return checkEmptyDeep(child); }) &&
                     checkEmptyDeep(seq.expr);
        },

        [&](const ExpressionPtr &klass) { result = false; });
    return result;
}

} // namespace sorbet::ast
