#include "main/lsp/ExtractMethod.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "ast/treemap/treemap.h"
#include "common/sort/sort.h"
#include "main/lsp/CreateMissingMethod.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

void setExprLoc(ast::ExpressionPtr &expr, core::LocOffsets loc) {
    switch (expr.tag()) {
        case ast::Tag::ClassDef:
            ast::cast_tree_nonnull<ast::ClassDef>(expr).loc = loc;
            break;
        case ast::Tag::MethodDef:
            ast::cast_tree_nonnull<ast::MethodDef>(expr).loc = loc;
            break;
        case ast::Tag::If:
            ast::cast_tree_nonnull<ast::If>(expr).loc = loc;
            break;
        case ast::Tag::While:
            ast::cast_tree_nonnull<ast::While>(expr).loc = loc;
            break;
        case ast::Tag::Break:
            ast::cast_tree_nonnull<ast::Break>(expr).loc = loc;
            break;
        case ast::Tag::Retry:
            ast::cast_tree_nonnull<ast::Retry>(expr).loc = loc;
            break;
        case ast::Tag::Next:
            ast::cast_tree_nonnull<ast::Next>(expr).loc = loc;
            break;
        case ast::Tag::Return:
            ast::cast_tree_nonnull<ast::Return>(expr).loc = loc;
            break;
        case ast::Tag::RescueCase:
            ast::cast_tree_nonnull<ast::RescueCase>(expr).loc = loc;
            break;
        case ast::Tag::Rescue:
            ast::cast_tree_nonnull<ast::Rescue>(expr).loc = loc;
            break;
        case ast::Tag::Local:
            ast::cast_tree_nonnull<ast::Local>(expr).loc = loc;
            break;
        case ast::Tag::UnresolvedIdent:
            ast::cast_tree_nonnull<ast::UnresolvedIdent>(expr).loc = loc;
            break;
        case ast::Tag::RestParam:
            ast::cast_tree_nonnull<ast::RestParam>(expr).loc = loc;
            break;
        case ast::Tag::KeywordArg:
            ast::cast_tree_nonnull<ast::KeywordArg>(expr).loc = loc;
            break;
        case ast::Tag::OptionalParam:
            ast::cast_tree_nonnull<ast::OptionalParam>(expr).loc = loc;
            break;
        case ast::Tag::BlockParam:
            ast::cast_tree_nonnull<ast::BlockParam>(expr).loc = loc;
            break;
        case ast::Tag::ShadowArg:
            ast::cast_tree_nonnull<ast::ShadowArg>(expr).loc = loc;
            break;
        case ast::Tag::Assign:
            ast::cast_tree_nonnull<ast::Assign>(expr).loc = loc;
            break;
        case ast::Tag::Send:
            ast::cast_tree_nonnull<ast::Send>(expr).loc = loc;
            break;
        case ast::Tag::Cast:
            ast::cast_tree_nonnull<ast::Cast>(expr).loc = loc;
            break;
        case ast::Tag::Hash:
            ast::cast_tree_nonnull<ast::Hash>(expr).loc = loc;
            break;
        case ast::Tag::Array:
            ast::cast_tree_nonnull<ast::Array>(expr).loc = loc;
            break;
        case ast::Tag::Literal:
            ast::cast_tree_nonnull<ast::Literal>(expr).loc = loc;
            break;
        case ast::Tag::UnresolvedConstantLit:
            ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(expr).loc = loc;
            break;
        case ast::Tag::ConstantLit:
            ast::cast_tree_nonnull<ast::ConstantLit>(expr).setLoc(loc);
            break;
        case ast::Tag::ZSuperArgs:
            ast::cast_tree_nonnull<ast::ZSuperArgs>(expr).loc = loc;
            break;
        case ast::Tag::Block:
            ast::cast_tree_nonnull<ast::Block>(expr).loc = loc;
            break;
        case ast::Tag::InsSeq:
            ast::cast_tree_nonnull<ast::InsSeq>(expr).loc = loc;
            break;
        case ast::Tag::RuntimeMethodDefinition:
            ast::cast_tree_nonnull<ast::RuntimeMethodDefinition>(expr).loc = loc;
            break;
        case ast::Tag::Self:
            ast::cast_tree_nonnull<ast::Self>(expr).loc = loc;
            break;
        case ast::Tag::EmptyTree:
            // There is only one EmptyTree created, so the loc is not mutable
            break;
    }
}

enum class IterResult {
    Continue,
    Stop,
};

// This should match the behavior of treemap.h exactly with regard to the children iterated over
// The order of iteration should be left to right to match Ruby's order of evaluation when it matters.
template <typename F> void iterChildrenUntil(const ast::ExpressionPtr &expr, F &&fn) {
    switch (expr.tag()) {
        case ast::Tag::InsSeq: {
            auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(expr);
            for (auto &stat : insSeq.stats) {
                if (fn(stat) == IterResult::Stop)
                    return;
            }
            if (fn(insSeq.expr) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::ClassDef: {
            auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(expr);
            for (auto &stat : classDef.rhs) {
                if (fn(stat) == IterResult::Stop)
                    return;
            }
            break;
        }
        case ast::Tag::MethodDef: {
            auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(expr);
            for (auto &param : methodDef.params) {
                if (auto optArg = ast::cast_tree<ast::OptionalParam>(param)) {
                    if (fn(optArg->default_) == IterResult::Stop)
                        return;
                }
            }
            if (fn(methodDef.rhs) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::If: {
            auto &if_ = ast::cast_tree_nonnull<ast::If>(expr);
            if (fn(if_.cond) == IterResult::Stop)
                return;
            if (fn(if_.thenp) == IterResult::Stop)
                return;
            if (fn(if_.elsep) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::While: {
            auto &while_ = ast::cast_tree_nonnull<ast::While>(expr);
            if (fn(while_.cond) == IterResult::Stop)
                return;
            if (fn(while_.body) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::Break: {
            auto &break_ = ast::cast_tree_nonnull<ast::Break>(expr);
            if (fn(break_.expr) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::Next: {
            auto &next = ast::cast_tree_nonnull<ast::Next>(expr);
            if (fn(next.expr) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::Return: {
            auto &return_ = ast::cast_tree_nonnull<ast::Return>(expr);
            if (fn(return_.expr) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::RescueCase: {
            auto &rescueCase = ast::cast_tree_nonnull<ast::RescueCase>(expr);
            for (auto &exception : rescueCase.exceptions) {
                if (fn(exception) == IterResult::Stop)
                    return;
            }
            if (fn(rescueCase.var) == IterResult::Stop)
                return;
            if (fn(rescueCase.body) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::Rescue: {
            auto &rescue = ast::cast_tree_nonnull<ast::Rescue>(expr);
            if (fn(rescue.body) == IterResult::Stop)
                return;
            for (auto &rescueCase : rescue.rescueCases) {
                if (fn(rescueCase) == IterResult::Stop)
                    return;
            }
            if (fn(rescue.else_) == IterResult::Stop)
                return;
            if (fn(rescue.ensure) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::Assign: {
            auto &assign = ast::cast_tree_nonnull<ast::Assign>(expr);
            if (fn(assign.lhs) == IterResult::Stop)
                return;
            if (fn(assign.rhs) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::Send: {
            auto &send = ast::cast_tree_nonnull<ast::Send>(expr);
            if (fn(send.recv) == IterResult::Stop)
                return;
            for (auto &arg : send.nonBlockArgs()) {
                if (fn(arg) == IterResult::Stop)
                    return;
            }
            if (auto *block = send.rawBlock()) {
                if (fn(*block) == IterResult::Stop)
                    return;
            }
            break;
        }
        case ast::Tag::Cast: {
            auto &cast = ast::cast_tree_nonnull<ast::Cast>(expr);
            if (fn(cast.arg) == IterResult::Stop)
                return;
            if (fn(cast.typeExpr) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::Hash: {
            auto &hash = ast::cast_tree_nonnull<ast::Hash>(expr);
            for (auto &key : hash.keys) {
                if (fn(key) == IterResult::Stop)
                    return;
            }
            for (auto &value : hash.values) {
                if (fn(value) == IterResult::Stop)
                    return;
            }
            break;
        }
        case ast::Tag::Array: {
            auto &array = ast::cast_tree_nonnull<ast::Array>(expr);
            for (auto &elem : array.elems) {
                if (fn(elem) == IterResult::Stop)
                    return;
            }
            break;
        }
        case ast::Tag::Block: {
            auto &block = ast::cast_tree_nonnull<ast::Block>(expr);
            for (auto &param : block.params) {
                if (auto optArg = ast::cast_tree<ast::OptionalParam>(param)) {
                    if (fn(optArg->default_) == IterResult::Stop)
                        return;
                }
            }
            if (fn(block.body) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::UnresolvedConstantLit: {
            auto &lit = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(expr);
            if (fn(lit.scope) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::RestParam: {
            auto &param = ast::cast_tree_nonnull<ast::RestParam>(expr);
            if (fn(param.expr) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::KeywordArg: {
            auto &kwarg = ast::cast_tree_nonnull<ast::KeywordArg>(expr);
            if (fn(kwarg.expr) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::OptionalParam: {
            auto &param = ast::cast_tree_nonnull<ast::OptionalParam>(expr);
            if (fn(param.expr) == IterResult::Stop)
                return;
            if (fn(param.default_) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::BlockParam: {
            auto &param = ast::cast_tree_nonnull<ast::BlockParam>(expr);
            if (fn(param.expr) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::ShadowArg: {
            auto &shadow = ast::cast_tree_nonnull<ast::ShadowArg>(expr);
            if (fn(shadow.expr) == IterResult::Stop)
                return;
            break;
        }
        case ast::Tag::Retry:
        case ast::Tag::ZSuperArgs:
        case ast::Tag::Local:
        case ast::Tag::UnresolvedIdent:
        case ast::Tag::Literal:
        case ast::Tag::ConstantLit:
        case ast::Tag::RuntimeMethodDefinition:
        case ast::Tag::Self:
        case ast::Tag::EmptyTree:
            break;
    }
}

template <typename F> void iterChildren(const ast::ExpressionPtr &expr, F &&fn) {
    iterChildrenUntil(expr, [&](const ast::ExpressionPtr &child) {
        fn(child);
        return IterResult::Continue;
    });
}

struct LocSumComputer {
    void postTransformExpressionPtr(core::Context ctx, ast::ExpressionPtr &tree) {
        core::LocOffsets merged;
        iterChildren(tree, [&](const ast::ExpressionPtr &child) { merged = merged.join(child.loc()); });
        setExprLoc(tree, tree.loc().join(merged));
    }
};

// Restore the invariant that locations of parent nodes enclose child nodes. We may still have locations that don't
// exist, but we are guaranteed that child nodes will of parent nodes with non-existing locations will also have
// non-existing locations
void computeLocSums(const core::GlobalState &gs, ast::ExpressionPtr &expr) {
    LocSumComputer computer;
    core::Context ctx(gs, core::Symbols::root(), core::FileRef());
    ast::TreeWalk::apply(ctx, computer, expr);
}

optional<pair<int, int>> getStatsContainedInTarget(const vector<const ast::ExpressionPtr *> &stats,
                                                   const core::LocOffsets target) {
    int i = -1;
    int j = -1;
    for (int k = 0; k < stats.size(); k++) {
        if (stats[k]->loc().exists() && target.beginPos() <= stats[k]->loc().beginPos()) {
            i = k;
            break;
        }
    }
    if (i == -1)
        return nullopt;
    for (int k = stats.size() - 1; k >= 0; k--) {
        if (stats[k]->loc().exists() && stats[k]->loc().endPos() <= target.endPos()) {
            j = k + 1;
            break;
        }
    }
    if (j == -1)
        return nullopt;
    if (i >= j)
        return nullopt;
    for (int k = 0; k < i; k++) {
        if (stats[k]->loc().exists() && stats[k]->loc().intersection(target).exists())
            return nullopt;
    }
    for (int k = j; k < stats.size(); k++) {
        if (stats[k]->loc().exists() && stats[k]->loc().intersection(target).exists())
            return nullopt;
    }
    return {{i, j}};
}

// A selection of nodes is defined as the largest sequence of nodes contained in a target loc
// the return type is optional<nonempty_vector<const ast::ExpressionPtr *>>
// which is equivalent to vector<const ast::ExpressionPtr *>
// empty vector means this wasn't a valid selection.
vector<const ast::ExpressionPtr *> getSelection(const ast::ExpressionPtr &expr, const core::LocOffsets target) {
    if (!expr.loc().exists()) {
        return {};
    }
    // We shouldn't be able to select empty nodes, usually these are synthetic.
    // For example a method send with implicit self creates a self node with a loc that exists but is empty
    if (expr.loc().empty()) {
        return {};
    }
    if (target.contains(expr.loc())) {
        return {&expr};
    }
    switch (expr.tag()) {
        case ast::Tag::InsSeq: {
            auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(expr);
            vector<const ast::ExpressionPtr *> stats;
            for (auto &stat : insSeq.stats) {
                stats.push_back(&stat);
            }
            stats.push_back(&insSeq.expr);
            auto result = getStatsContainedInTarget(stats, target);
            if (result.has_value()) {
                auto [i, j] = result.value();
                vector<const ast::ExpressionPtr *> selection;
                for (int k = i; k < j; k++) {
                    selection.push_back(stats[k]);
                }
                return selection;
            }
            for (auto *stat : stats) {
                if (auto selection = getSelection(*stat, target); !selection.empty()) {
                    return selection;
                }
            }
            break;
        }
        case ast::Tag::Assign: {
            auto &assign = ast::cast_tree_nonnull<ast::Assign>(expr);
            if (auto selection = getSelection(assign.rhs, target); !selection.empty()) {
                return selection;
            }
            break;
        }
        default: {
            vector<const ast::ExpressionPtr *> selection;
            iterChildrenUntil(expr, [&selection, target](const ast::ExpressionPtr &child) {
                if (selection = getSelection(child, target); !selection.empty()) {
                    return IterResult::Stop;
                }
                return IterResult::Continue;
            });
            if (!selection.empty()) {
                return selection;
            }
            break;
        }
    }
    return {};
}

// TODO(bshu) better way to do this
static const ast::ExpressionPtr emptyTreeStorage = ast::make_expression<ast::EmptyTree>();

struct ContItem {
    InlinedVector<const ast::ExpressionPtr *, 1> branches;
    const ast::ExpressionPtr *lastExtraStat = nullptr;

    ContItem(const ast::ExpressionPtr *expr) : branches{expr} {}
    ContItem(const ast::ExpressionPtr *branch1, const ast::ExpressionPtr *branch2) : branches{branch1, branch2} {}

    bool isStat() const {
        return branches.size() == 1;
    }

    string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const {
        if (isStat()) {
            return branches[0]->toStringWithTabs(gs, tabs);
        }
        string result = string(tabs * 2, ' ') + "Branch(\n";
        for (int i = 0; i < branches.size(); i++) {
            if (i > 0) {
                result += ",\n";
            }
            result += branches[i]->toStringWithTabs(gs, tabs + 1);
        }
        if (lastExtraStat != nullptr) {
            result += ",\n" + lastExtraStat->toStringWithTabs(gs, tabs + 1);
        }
        result += ")";
        return result;
    }
};

// We could merge these two functions together as getSelectionAndContinuation, but we choose not to so that in the
// common case that a selection isn't valid, we don't have to allocate for the continuation
// In this case we use optional in the return type since the continuation can actually be empty, but this doesn't
// indicate that the selection is invalid
// This function essentially duplicates the logic of getSelection but gets the continuation after the
// deepest node contained in the target.
// The continuation is an approximation of what code can run after the selection for the purposes of calculating
// liveness.
// invariant: if getSelection is nonempty, then getContinuation is nonnull
optional<vector<ContItem>> getContinuation(const ast::ExpressionPtr &expr, const core::LocOffsets target) {
    if (!expr.loc().exists()) {
        return nullopt;
    }
    if (expr.loc().empty()) {
        return nullopt;
    }
    if (target.contains(expr.loc())) {
        return {{}};
    }
    switch (expr.tag()) {
        case ast::Tag::InsSeq: {
            auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(expr);
            vector<const ast::ExpressionPtr *> stats;
            for (auto &stat : insSeq.stats) {
                stats.push_back(&stat);
            }
            stats.push_back(&insSeq.expr);
            auto result = getStatsContainedInTarget(stats, target);
            if (result.has_value()) {
                auto [i, j] = result.value();
                vector<ContItem> continuation;
                for (int k = j; k < stats.size(); k++) {
                    continuation.emplace_back(stats[k]);
                }
                return {continuation};
            }
            int i = 0;
            for (auto stat : stats) {
                if (auto continuation = getContinuation(*stat, target); continuation.has_value()) {
                    for (auto j = i + 1; j < stats.size(); j++) {
                        continuation->emplace_back(stats[j]);
                    }
                    return {continuation};
                }
                i++;
            }
            break;
        }
        case ast::Tag::Assign: {
            auto &assign = ast::cast_tree_nonnull<ast::Assign>(expr);
            if (auto continuation = getContinuation(assign.rhs, target); continuation.has_value()) {
                return continuation;
            }
            break;
        }
        case ast::Tag::If: {
            auto &if_ = ast::cast_tree_nonnull<ast::If>(expr);
            if (auto continuation = getContinuation(if_.cond, target); continuation.has_value()) {
                continuation->emplace_back(&if_.thenp, &if_.elsep);
                return continuation;
            }
            if (auto continuation = getContinuation(if_.thenp, target); continuation.has_value()) {
                return continuation;
            }
            if (auto continuation = getContinuation(if_.elsep, target); continuation.has_value()) {
                return continuation;
            }
            break;
        }
        case ast::Tag::While: {
            auto &while_ = ast::cast_tree_nonnull<ast::While>(expr);
            if (auto continuation = getContinuation(while_.cond, target); continuation.has_value()) {
                continuation->emplace_back(&emptyTreeStorage, &while_.body);
                continuation->back().lastExtraStat = &while_.cond;
                return continuation;
            }
            if (auto continuation = getContinuation(while_.body, target); continuation.has_value()) {
                continuation->emplace_back(&while_.cond);
                continuation->emplace_back(&emptyTreeStorage, &while_.body);
                return continuation;
            }
            break;
        }
        case ast::Tag::RescueCase: {
            auto &rescueCase = ast::cast_tree_nonnull<ast::RescueCase>(expr);
            if (auto continuation = getContinuation(rescueCase.body, target); continuation.has_value()) {
                return continuation;
            }
            break;
        }
        case ast::Tag::Rescue: {
            auto &rescue = ast::cast_tree_nonnull<ast::Rescue>(expr);
            if (auto continuation = getContinuation(rescue.body, target); continuation.has_value()) {
                ContItem item(&rescue.else_);
                for (auto &rescueCase : rescue.rescueCases) {
                    item.branches.push_back(&rescueCase);
                }
                continuation->push_back(std::move(item));
                continuation->emplace_back(&rescue.ensure);
                return continuation;
            }
            for (auto &rescueCase : rescue.rescueCases) {
                if (auto continuation = getContinuation(rescueCase, target); continuation.has_value()) {
                    continuation->emplace_back(&rescue.ensure);
                    return continuation;
                }
            }
            if (auto continuation = getContinuation(rescue.else_, target); continuation.has_value()) {
                continuation->emplace_back(&rescue.ensure);
                return continuation;
            }
            if (auto continuation = getContinuation(rescue.ensure, target); continuation.has_value()) {
                return continuation;
            }
            break;
        }
            // TODO: control flow like return etc.
        default: {
            optional<vector<ContItem>> continuation;
            iterChildren(expr, [&continuation, target](const ast::ExpressionPtr &child) {
                if (continuation.has_value()) {
                    continuation->emplace_back(&child);
                } else {
                    continuation = getContinuation(child, target);
                }
            });
            if (continuation.has_value()) {
                return continuation;
            }
            break;
        }
    }
    return nullopt;
}

struct ComputeWrites {
    UnorderedSet<core::LocalVariable> &writes;

    void postTransformAssign(core::Context ctx, const ast::Assign &assign) {
        auto local = ast::cast_tree<ast::Local>(assign.lhs);
        if (local == nullptr) {
            return;
        }
        writes.emplace(local->localVariable);
    }
};

UnorderedSet<core::LocalVariable> setUnion(const UnorderedSet<core::LocalVariable> &a,
                                           const UnorderedSet<core::LocalVariable> &b) {
    auto result = a;
    result.insert(b.begin(), b.end());
    return result;
}

[[maybe_unused]] void computeWrites(const core::GlobalState &gs, const ast::ExpressionPtr &expr,
                                    UnorderedSet<core::LocalVariable> &writes) {
    ComputeWrites walker{writes};
    core::Context ctx(gs, core::Symbols::root(), core::FileRef());
    ast::ConstTreeWalk::apply(ctx, walker, expr);
}

UnorderedSet<core::LocalVariable> computeExprLiveIn(const ast::ExpressionPtr &expr,
                                                    UnorderedSet<core::LocalVariable> liveOut) {
    switch (expr.tag()) {
        case ast::Tag::If: {
            auto &if_ = ast::cast_tree_nonnull<ast::If>(expr);
            auto liveInThen = computeExprLiveIn(if_.thenp, liveOut);
            auto liveInElse = computeExprLiveIn(if_.elsep, liveOut);
            auto liveOutCond = setUnion(liveInThen, liveInElse);
            return computeExprLiveIn(if_.cond, liveOutCond);
        }
        case ast::Tag::While: {
            auto &while_ = ast::cast_tree_nonnull<ast::While>(expr);
            auto liveInBody = computeExprLiveIn(while_.body, liveOut);
            auto liveOutCond = setUnion(liveInBody, liveOut);
            return computeExprLiveIn(while_.cond, liveOutCond);
        }
        case ast::Tag::InsSeq: {
            auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(expr);
            liveOut = computeExprLiveIn(insSeq.expr, liveOut);
            for (auto it = insSeq.stats.rbegin(); it != insSeq.stats.rend(); it++) {
                liveOut = computeExprLiveIn(*it, liveOut);
            }
            return liveOut;
        }
        case ast::Tag::Assign: {
            auto &assign = ast::cast_tree_nonnull<ast::Assign>(expr);
            if (auto local = ast::cast_tree<ast::Local>(assign.lhs)) {
                liveOut.erase(local->localVariable);
                return computeExprLiveIn(assign.rhs, liveOut);
            } else {
                liveOut = computeExprLiveIn(assign.rhs, liveOut);
                return computeExprLiveIn(assign.lhs, liveOut);
            }
        }
        case ast::Tag::Local: {
            auto &local = ast::cast_tree_nonnull<ast::Local>(expr);
            liveOut.emplace(local.localVariable);
            return liveOut;
        }
        case ast::Tag::RescueCase: {
            auto &rescueCase = ast::cast_tree_nonnull<ast::RescueCase>(expr);
            liveOut = computeExprLiveIn(rescueCase.body, liveOut);
            if (auto local = ast::cast_tree<ast::Local>(rescueCase.var)) {
                liveOut.erase(local->localVariable);
            }
            return liveOut;
        }
        case ast::Tag::Rescue: {
            auto &rescue = ast::cast_tree_nonnull<ast::Rescue>(expr);
            liveOut = computeExprLiveIn(rescue.ensure, liveOut);
            auto branchLiveIn = computeExprLiveIn(rescue.else_, liveOut);
            for (auto &rescueCase : rescue.rescueCases) {
                branchLiveIn = setUnion(branchLiveIn, computeExprLiveIn(rescueCase, liveOut));
            }
            return computeExprLiveIn(rescue.body, branchLiveIn);
        }
            // TODO: control flow like return etc.
        default: {
            // TODO: maybe don't allocate
            vector<const ast::ExpressionPtr *> children;
            iterChildren(expr, [&children](const ast::ExpressionPtr &child) { children.push_back(&child); });
            for (auto it = children.rbegin(); it != children.rend(); it++) {
                liveOut = computeExprLiveIn(**it, liveOut);
            }
            return liveOut;
        }
    }
}

[[maybe_unused]] UnorderedSet<core::LocalVariable> computeContItemLiveIn(const ContItem contItem,
                                                                         UnorderedSet<core::LocalVariable> liveOut) {
    if (contItem.isStat()) {
        return computeExprLiveIn(*contItem.branches[0], liveOut);
    }
    UnorderedSet<core::LocalVariable> result;
    for (int i = 0; i < contItem.branches.size(); i++) {
        auto branchLiveOut = liveOut;
        if (i == contItem.branches.size() - 1 && contItem.lastExtraStat != nullptr) {
            branchLiveOut = computeExprLiveIn(*contItem.lastExtraStat, branchLiveOut);
        }
        result = setUnion(result, computeExprLiveIn(*contItem.branches[i], branchLiveOut));
    }
    return result;
}

// TODO(bshu) AI
string indentString(uint32_t indent, string_view str) {
    string prefix(indent, ' ');
    string result;
    string_view remaining = str;
    while (!remaining.empty()) {
        auto newline = remaining.find('\n');
        if (newline == string_view::npos) {
            result += prefix + string(remaining) + "\n";
            break;
        }
        result += prefix + string(remaining.substr(0, newline)) + "\n";
        remaining = remaining.substr(newline + 1);
    }
    return result;
}

// TODO(bshu) AI
string dedentString(uint32_t indent, string_view str) {
    string result;
    string_view remaining = str;
    while (!remaining.empty()) {
        auto newline = remaining.find('\n');
        string_view line = (newline == string_view::npos) ? remaining : remaining.substr(0, newline);
        uint32_t spaces = 0;
        while (spaces < indent && spaces < line.size() && line[spaces] == ' ') {
            spaces++;
        }
        result += string(line.substr(spaces));
        if (newline == string_view::npos) {
            result += "\n";
            break;
        }
        result += "\n";
        remaining = remaining.substr(newline + 1);
    }
    return result;
}

string tupleValue(const vector<string> &values) {
    if (values.empty()) {
        return "[]";
    }
    if (values.size() == 1) {
        return values[0];
    }
    return fmt::format("[{}]", absl::StrJoin(values, ", "));
}

string tupleLhs(const vector<string> &lhsNames) {
    if (lhsNames.empty()) {
        return "_";
    }
    if (lhsNames.size() == 1) {
        return lhsNames[0];
    }
    return absl::StrJoin(lhsNames, ", ");
}

string parens(string s) {
    return fmt::format("({})", s);
}

string formatAssign(const string &lhs, const string &rhs, bool isInStatementContext) {
    auto assign = fmt::format("{} = {}", lhs, rhs);
    if (!isInStatementContext) {
        assign = parens(assign);
    }
    return assign;
}

string formatReturnSig(const vector<string> &types) {
    if (types.empty()) {
        return ".void()";
    }
    if (types.size() == 1) {
        return fmt::format(".returns({})", types[0]);
    }
    return fmt::format(".returns([{}])", absl::StrJoin(types, ", "));
}

string formatNewMethod(bool isSingletonMethod, string_view selectionSource, const vector<string> &params,
                       const vector<string> &updateReturns, bool isReturnValueNeeded) {
    string methodName = isSingletonMethod ? "self.new_method" : "new_method";

    // Build sig
    auto paramsSig =
        absl::StrJoin(params, ", ", [](string *out, const string &p) { absl::StrAppend(out, p, ": T.untyped"); });
    vector<string> returnTypes;
    if (isReturnValueNeeded) {
        returnTypes.push_back("T.untyped");
    }
    for (int i = 0; i < updateReturns.size(); i++) {
        returnTypes.push_back("T.untyped");
    }
    string returnsSig = formatReturnSig(returnTypes);
    auto sig = fmt::format("sig {{ params({}){} }}", paramsSig, returnsSig);

    auto paramList = absl::StrJoin(params, ", ");

    string result = fmt::format("{}\ndef {}({})\n", sig, methodName, paramList);

    if (updateReturns.empty()) {
        result += indentString(2, selectionSource);
    } else {
        if (isReturnValueNeeded) {
            result += indentString(2, "newMethodReturnValue = begin\n");
            result += indentString(4, selectionSource);
            result += indentString(2, "end\n");
            vector<string> vals{"newMethodReturnValue"};
            for (auto v : updateReturns) {
                vals.push_back(v);
            }
            result += indentString(2, tupleValue(vals));
        } else {
            result += indentString(2, selectionSource);
            auto returnExpr = tupleValue(updateReturns);
            result += indentString(2, returnExpr);
        }
    }

    result += "end";
    return result;
}

string formatCall(bool isSingletonMethod, const vector<string> &args, const vector<string> &updateReturns,
                  bool isInStatementContext, bool isReturnValueNeeded) {
    string methodName = isSingletonMethod ? "self.new_method" : "new_method";
    string call = methodName + "(";
    for (int i = 0; i < args.size(); i++) {
        if (i > 0) {
            call += ", ";
        }
        call += args[i];
    }
    call += ")";

    if (updateReturns.empty()) {
        return call;
    }

    vector<string> lhsNames;
    if (isReturnValueNeeded) {
        lhsNames.emplace_back("_");
    }
    for (auto &ret : updateReturns) {
        lhsNames.push_back(ret);
    }

    auto assign = fmt::format("{} = {}", tupleLhs(lhsNames), call);
    if (isReturnValueNeeded) {
        if (lhsNames.size() > 1) {
            return fmt::format("{}[0]", formatAssign(tupleLhs(lhsNames), call, false));
        } else {
            return formatAssign(tupleLhs(lhsNames), call, isInStatementContext);
        }
    } else {
        return formatAssign(tupleLhs(lhsNames), call, isInStatementContext);
    }
}

optional<bool> isInStatementContext(const ast::ExpressionPtr &expr, const core::LocOffsets target) {
    if (!expr.loc().exists()) {
        return nullopt;
    }
    if (expr.loc().empty()) {
        return nullopt;
    }
    if (target.contains(expr.loc())) {
        return {false};
    }
    switch (expr.tag()) {
        case ast::Tag::InsSeq: {
            auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(expr);
            vector<const ast::ExpressionPtr *> stats;
            for (auto &stat : insSeq.stats) {
                stats.push_back(&stat);
            }
            stats.push_back(&insSeq.expr);
            auto result = getStatsContainedInTarget(stats, target);
            if (result.has_value()) {
                return {true};
            }
            for (auto *stat : stats) {
                if (auto r = isInStatementContext(*stat, target); r.has_value()) {
                    return r;
                }
            }
            break;
        }
        default: {
            optional<bool> found;
            iterChildrenUntil(expr, [&found, target](const ast::ExpressionPtr &child) {
                if (found = isInStatementContext(child, target); found.has_value()) {
                    return IterResult::Stop;
                }
                return IterResult::Continue;
            });
            if (found.has_value()) {
                return found;
            }
            break;
        }
    }
    return nullopt;
}

optional<bool> isReturnValueNeeded(const ast::ExpressionPtr &expr, const core::LocOffsets target) {
    if (!expr.loc().exists()) {
        return nullopt;
    }
    if (expr.loc().empty()) {
        return nullopt;
    }
    if (target.contains(expr.loc())) {
        return {true};
    }
    switch (expr.tag()) {
        case ast::Tag::InsSeq: {
            auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(expr);
            vector<const ast::ExpressionPtr *> stats;
            for (auto &stat : insSeq.stats) {
                stats.push_back(&stat);
            }
            stats.push_back(&insSeq.expr);
            auto result = getStatsContainedInTarget(stats, target);
            if (result.has_value()) {
                auto [i, j] = result.value();
                bool includesLastExpr = (j == stats.size());
                return {includesLastExpr};
            }
            for (auto *stat : stats) {
                if (auto r = isReturnValueNeeded(*stat, target); r.has_value()) {
                    return r;
                }
            }
            break;
        }
        default: {
            optional<bool> found;
            iterChildrenUntil(expr, [&found, target](const ast::ExpressionPtr &child) {
                if (found = isReturnValueNeeded(child, target); found.has_value()) {
                    return IterResult::Stop;
                }
                return IterResult::Continue;
            });
            if (found.has_value()) {
                return found;
            }
            break;
        }
    }
    return nullopt;
}

class EnclosingMethodWalk {
public:
    core::Loc targetLoc;
    const ast::MethodDef *enclosingMethod = nullptr;
    const shared_ptr<spdlog::logger> &logger;
    const core::GlobalState &gs;
    core::FileRef file;

    EnclosingMethodWalk(core::Loc targetLoc, const shared_ptr<spdlog::logger> &logger, const core::GlobalState &gs,
                        core::FileRef file)
        : targetLoc(targetLoc), logger(logger), gs(gs), file(file) {}

    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef) {
        logger->debug("ExtractMethod: visiting method '{}', rhs loc: {}", methodDef.name.toString(gs),
                      methodDef.rhs.loc().showRaw(gs, file));
        logger->debug("ExtractMethod: targetLoc offsets: {}", targetLoc.offsets().showRaw(gs, file));
        // TODO(bshu) make this more precise. If we do rhs->loc instead, it will be annoying since it doesn't include
        // leading and trailing whitespace.
        if (methodDef.loc.contains(targetLoc.offsets())) {
            enclosingMethod = &methodDef;
        }
    }
};
} // namespace

namespace extract_method {

vector<unique_ptr<TextDocumentEdit>> getExtractMethodEdits(LSPTypecheckerDelegate &typechecker,
                                                           const LSPConfiguration &config,
                                                           const core::Loc selectionLoc) {
    ENFORCE(selectionLoc.exists());
    const auto &gs = typechecker.state();
    const auto file = selectionLoc.file();

    auto parsedFile = typechecker.getResolved(file);
    computeLocSums(gs, parsedFile.tree);

    core::Context ctx(gs, core::Symbols::root(), file);
    EnclosingMethodWalk walk(selectionLoc, config.logger, gs, file);
    ast::ConstTreeWalk::apply(ctx, walk, parsedFile.tree);

    if (!walk.enclosingMethod) {
        config.logger->debug("ExtractMethod: no enclosing method found");
        return {};
    }
    config.logger->debug("ExtractMethod: found enclosing method");

    config.logger->debug("Selection loc: {}", selectionLoc.showRaw(gs));
    config.logger->debug("Method rhs loc: {}", walk.enclosingMethod->rhs.showRaw(gs));
    auto rhsLoc = walk.enclosingMethod->rhs.loc();
    auto selOffsets = selectionLoc.offsets();
    auto newStart = max(rhsLoc.beginPos(), selOffsets.beginPos());
    auto newEnd = min(rhsLoc.endPos(), selOffsets.endPos());
    if (newStart > newEnd) {
        config.logger->debug("ExtractMethod: no intersection found");
        return {};
    }
    auto selection = getSelection(parsedFile.tree, selectionLoc.offsets());
    if (selection.empty()) {
        config.logger->debug("ExtractMethod: no selection found");
        return {};
    }
    config.logger->debug("ExtractMethod: selection found, size: {}", selection.size());
    auto continuation = getContinuation(parsedFile.tree, selectionLoc.offsets());
    ENFORCE(continuation.has_value());
    auto isInStat = isInStatementContext(parsedFile.tree, selectionLoc.offsets());
    ENFORCE(isInStat.has_value());
    auto isRetValueNeeded = isReturnValueNeeded(parsedFile.tree, selectionLoc.offsets());
    ENFORCE(isRetValueNeeded.has_value());
    config.logger->debug("ExtractMethod: selection size: {}, continuation size: {}", selection.size(),
                         continuation.has_value() ? continuation->size() : 0);
    config.logger->debug("ExtractMethod: selection loc: {}", selectionLoc.showRaw(gs));
    for (auto &expr : selection) {
        config.logger->debug("ExtractMethod selection: {}", expr->toStringWithTabs(gs));
    }
    for (auto &item : continuation.value()) {
        config.logger->debug("ExtractMethod continuation: {}", item.toStringWithTabs(gs));
    }

    UnorderedSet<core::LocalVariable> writes;
    for (auto &stat : selection) {
        computeWrites(gs, *stat, writes);
    }

    UnorderedSet<core::LocalVariable> readByContinuation;
    auto &cont = continuation.value();
    for (auto it = cont.rbegin(); it != cont.rend(); it++) {
        readByContinuation = computeContItemLiveIn(*it, readByContinuation);
    }

    UnorderedSet<core::LocalVariable> readBySelection;
    for (auto it = selection.rbegin(); it != selection.rend(); it++) {
        readBySelection = computeExprLiveIn(*(*it), readBySelection);
    }

    for (auto &var : readByContinuation) {
        config.logger->debug("ExtractMethod liveOut: {}", var.toString(gs));
    }

    for (auto &var : readBySelection) {
        config.logger->debug("ExtractMethod liveIn: {}", var.toString(gs));
    }

    for (auto &var : writes) {
        config.logger->debug("ExtractMethod writes: {}", var.toString(gs));
    }

    auto enclosingMethodRef = walk.enclosingMethod->symbol;
    auto isSingletonMethod = enclosingMethodRef.data(gs)->owner.data(gs)->isSingletonClass(gs);

    auto res = create_missing_method::getInsertionLocationAfterMethod(gs, parsedFile, enclosingMethodRef);
    if (!res.has_value()) {
        return {};
    }
    auto [insertLoc, indentLength] = res.value();

    auto selectionSource = selectionLoc.source(gs);
    if (!selectionSource.has_value()) {
        return {};
    }

    // Sort params and returns into string vectors
    vector<string> paramNames;
    for (auto &var : readBySelection) {
        if (var == core::LocalVariable::selfVariable()) {
            continue;
        }
        paramNames.emplace_back(string(var._name.shortName(gs)));
    }
    fast_sort(paramNames);

    vector<string> updateReturnNames;
    for (auto &var : writes) {
        if (readByContinuation.contains(var)) {
            updateReturnNames.emplace_back(string(var._name.shortName(gs)));
        }
    }
    fast_sort(updateReturnNames);

    auto [_, selectionIndent] = selectionLoc.findStartOfIndentation(gs);
    auto dedentedSource = dedentString(selectionIndent, selectionSource.value());

    auto newMethodText =
        "\n\n" + indentString(indentLength, formatNewMethod(isSingletonMethod, dedentedSource, paramNames,
                                                            updateReturnNames, isRetValueNeeded.value()));
    auto callText =
        formatCall(isSingletonMethod, paramNames, updateReturnNames, isInStat.value(), isRetValueNeeded.value());

    config.logger->debug("ExtractMethod newMethod:\n{}", newMethodText);
    config.logger->debug("ExtractMethod call:\n{}", callText);

    vector<unique_ptr<TextEdit>> edits;
    edits.emplace_back(make_unique<TextEdit>(Range::fromLoc(gs, insertLoc), newMethodText));
    edits.emplace_back(make_unique<TextEdit>(Range::fromLoc(gs, selectionLoc), callText));

    auto tdi = make_unique<VersionedTextDocumentIdentifier>(config.fileRef2Uri(gs, file), JSONNullObject());
    vector<unique_ptr<TextDocumentEdit>> result;
    result.emplace_back(make_unique<TextDocumentEdit>(move(tdi), move(edits)));
    return result;
}

} // namespace extract_method

} // namespace sorbet::realmain::lsp
