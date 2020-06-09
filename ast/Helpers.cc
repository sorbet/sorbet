#include "ast/Helpers.h"

using namespace std;

namespace sorbet::ast {

bool definesBehavior(const TreePtr &expr) {
    if (BehaviorHelpers::checkEmptyDeep(expr)) {
        return false;
    }
    bool result = true;

    typecase(
        expr.get(),

        [&](ast::ClassDef *klass) {
            auto *id = ast::cast_tree<ast::UnresolvedIdent>(klass->name);
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

        [&](ast::Assign *asgn) {
            if (ast::isa_tree<ast::ConstantLit>(asgn->lhs)) {
                result = false;
            } else {
                result = true;
            }
        },

        [&](ast::InsSeq *seq) {
            result = absl::c_any_of(seq->stats, [](auto &child) { return definesBehavior(child); }) ||
                     definesBehavior(seq->expr);
        },

        // Ignore code synthesized by Rewriter pass.
        [&](ast::Send *send) { result = !send->flags.isRewriterSynthesized; },
        [&](ast::MethodDef *methodDef) { result = !methodDef->flags.isRewriterSynthesized; },
        [&](ast::Literal *methodDef) { result = false; },

        [&](ast::Expression *klass) { result = true; });
    return result;
}

bool BehaviorHelpers::checkClassDefinesBehavior(const TreePtr &expr) {
    auto *klass = ast::cast_tree_const<ast::ClassDef>(expr);
    ENFORCE(klass);
    return BehaviorHelpers::checkClassDefinesBehavior(*klass);
}

bool BehaviorHelpers::checkClassDefinesBehavior(const ast::ClassDef &klass) {
    for (auto &ancst : klass.ancestors) {
        auto *cnst = ast::cast_tree_const<ast::ConstantLit>(ancst);
        if (cnst && cnst->original != nullptr) {
            return true;
        }
    }
    for (auto &ancst : klass.singletonAncestors) {
        auto *cnst = ast::cast_tree_const<ast::ConstantLit>(ancst);
        if (cnst && cnst->original != nullptr) {
            return true;
        }
    }
    return absl::c_any_of(klass.rhs, [](auto &tree) { return definesBehavior(tree); });
}

bool BehaviorHelpers::checkEmptyDeep(const TreePtr &expr) {
    bool result = false;

    typecase(
        expr.get(),

        [&](ast::Send *send) {
            result = send->fun == core::Names::keepForIde() || send->fun == core::Names::keepDef() ||
                     send->fun == core::Names::keepSelfDef() || send->fun == core::Names::include() ||
                     send->fun == core::Names::extend();
        },

        [&](ast::EmptyTree *) { result = true; },

        [&](ast::InsSeq *seq) {
            result = absl::c_all_of(seq->stats, [](auto &child) { return checkEmptyDeep(child); }) &&
                     checkEmptyDeep(seq->expr);
        },

        [&](ast::Expression *klass) { result = false; });
    return result;
}

} // namespace sorbet::ast
