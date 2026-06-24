#include "main/lsp/ExtractMethod.h"
#include "ast/treemap/treemap.h"

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

auto getStatsContainedInTarget(const vector<const ast::ExpressionPtr *> &stats, const core::LocOffsets target) {
    auto it1 = absl::c_find_if(stats, [target](const ast::ExpressionPtr *stat) {
        return stat->loc().exists() && target.beginPos() <= stat->loc().beginPos();
    });
    auto it2 = stats.end();
    for (auto it = stats.begin(); it < stats.end(); it++) {
        if ((*it)->loc().exists() && (*it)->loc().endPos() <= target.endPos()) {
            it2 = std::next(it);
        }
    }
    ENFORCE(it1 <= it2);
    return make_pair(it1, it2);
}

// A selection of nodes is defined as the deepest sequence of nodes contained in a target loc
// the return type is optional<nonempty_vector<const ast::ExpressionPtr *>>
// which is equivalent to vector<const ast::ExpressionPtr *>
// empty vector means this wasn't a valid selection.
vector<const ast::ExpressionPtr *> getSelection(const ast::ExpressionPtr &expr, const core::LocOffsets target) {
    if (!expr.loc().exists()) {
        return {};
    }
    switch (expr.tag()) {
        case ast::Tag::InsSeq: {
            // This case is special because InsSeq is essentially a cache friendly variant of the inductive
            // representation of a sequence of expressions `InsSeq(ExpressionPtr, ExpressionPtr)`. If we represented
            // InsSeq inductively, then the default case would naturally cover InsSeq also.
            auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(expr);
            vector<const ast::ExpressionPtr *> stats;
            for (auto &stat : insSeq.stats) {
                stats.push_back(&stat);
            }
            stats.push_back(&insSeq.expr);
            for (auto *stat : stats) {
                if (auto selection = getSelection(*stat, target); !selection.empty()) {
                    return selection;
                }
            }
            auto [it1, it2] = getStatsContainedInTarget(stats, target);
            vector<const ast::ExpressionPtr *> selection;
            for (auto it = it1; it < it2; it++) {
                selection.push_back(*it);
            }
            if (!selection.empty()) {
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
    ENFORCE(expr.loc().exists());
    if (target.contains(expr.loc())) {
        return {&expr};
    } else {
        return {};
    }
}

// We could merge these two functions together as getSelectionAndContinuation, but we choose not to so that in the
// common case that a selection isn't valid, we don't have to allocate for the continuation
// In this case we use optional in the return type since the continuation can actually be empty, but this doesn't
// indicate that the selection is invalid
// This function essentially duplicates the logic of getSelection but gets the continuation after the
// deepest node contained in the target.
// The continuation is an approximation of what code can run after the selection for the purposes of calculating
// liveness.
// invariant: if getSelection is nonempty, then getContinuationAfterSelection is nonnull
optional<vector<const ast::ExpressionPtr *>> getContinuationAfterSelection(const ast::ExpressionPtr &expr,
                                                                           const core::LocOffsets target) {
    if (!expr.loc().exists()) {
        return {};
    }
    switch (expr.tag()) {
        case ast::Tag::InsSeq: {
            auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(expr);
            vector<const ast::ExpressionPtr *> stats;
            for (auto &stat : insSeq.stats) {
                stats.push_back(&stat);
            }
            stats.push_back(&insSeq.expr);
            int i = 0;
            for (auto stat : stats) {
                if (auto continuation = getContinuationAfterSelection(*stat, target); continuation.has_value()) {
                    for (auto j = i + 1; j < stats.size(); j++) {
                        continuation->push_back(stats[j]);
                    }
                    return {continuation};
                }
                i++;
            }
            auto [it1, it2] = getStatsContainedInTarget(stats, target);
            if (it1 != it2) {
                vector<const ast::ExpressionPtr *> continuation;
                for (auto it = it2; it < stats.end(); it++) {
                    continuation.push_back(*it);
                }
                return {continuation};
            }
            break;
        }
        default: {
            optional<vector<const ast::ExpressionPtr *>> continuation;
            iterChildrenUntil(expr, [&](const ast::ExpressionPtr &child) {
                if (continuation = getContinuationAfterSelection(child, target); continuation.has_value()) {
                    return IterResult::Stop;
                }
                return IterResult::Continue;
            });
            if (continuation.has_value()) {
                return continuation;
            }
            break;
        }
    }
    ENFORCE(expr.loc().exists());
    if (target.contains(expr.loc())) {
        return {{}};
    } else {
        return nullopt;
    }
}
} // namespace

namespace extract_method {

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

vector<unique_ptr<TextDocumentEdit>> getExtractMethodEdits(LSPTypecheckerDelegate &typechecker,
                                                           const LSPConfiguration &config,
                                                           const core::Loc selectionLoc) {
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
    auto continuation = getContinuationAfterSelection(parsedFile.tree, selectionLoc.offsets());
    ENFORCE(continuation.has_value());
    config.logger->debug("ExtractMethod: selection size: {}, continuation size: {}", selection.size(),
                         continuation.has_value() ? continuation->size() : 0);
    config.logger->debug("ExtractMethod: selection loc: {}", selectionLoc.showRaw(gs));
    for (auto &expr : selection) {
        config.logger->debug("ExtractMethod selection: {}", expr->toStringWithTabs(gs));
    }
    for (auto &expr : continuation.value()) {
        config.logger->debug("ExtractMethod continuation: {}", expr->toStringWithTabs(gs));
    }
    return {};
}

} // namespace extract_method

} // namespace sorbet::realmain::lsp
