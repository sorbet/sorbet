#include "main/lsp/ExtractMethod.h"
#include "ast/treemap/treemap.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

template <typename F> void iterChildren(const ast::ExpressionPtr &expr, F &&fn) {
    switch (expr.tag()) {
        case ast::Tag::ClassDef: {
            auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(expr);
            fn(classDef.name);
            for (auto &rhs : classDef.rhs) {
                fn(rhs);
            }
            for (auto &ancestor : classDef.ancestors) {
                fn(ancestor);
            }
            for (auto &ancestor : classDef.singletonAncestors) {
                fn(ancestor);
            }
            break;
        }
        case ast::Tag::MethodDef: {
            auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(expr);
            for (auto &param : methodDef.params) {
                fn(param);
            }
            fn(methodDef.rhs);
            break;
        }
        case ast::Tag::If: {
            auto &if_ = ast::cast_tree_nonnull<ast::If>(expr);
            fn(if_.cond);
            fn(if_.thenp);
            fn(if_.elsep);
            break;
        }
        case ast::Tag::While: {
            auto &while_ = ast::cast_tree_nonnull<ast::While>(expr);
            fn(while_.cond);
            fn(while_.body);
            break;
        }
        case ast::Tag::Break: {
            fn(ast::cast_tree_nonnull<ast::Break>(expr).expr);
            break;
        }
        case ast::Tag::Next: {
            fn(ast::cast_tree_nonnull<ast::Next>(expr).expr);
            break;
        }
        case ast::Tag::Return: {
            fn(ast::cast_tree_nonnull<ast::Return>(expr).expr);
            break;
        }
        case ast::Tag::RescueCase: {
            auto &rescueCase = ast::cast_tree_nonnull<ast::RescueCase>(expr);
            for (auto &exception : rescueCase.exceptions) {
                fn(exception);
            }
            fn(rescueCase.var);
            fn(rescueCase.body);
            break;
        }
        case ast::Tag::Rescue: {
            auto &rescue = ast::cast_tree_nonnull<ast::Rescue>(expr);
            fn(rescue.body);
            for (auto &rescueCase : rescue.rescueCases) {
                fn(rescueCase);
            }
            fn(rescue.else_);
            fn(rescue.ensure);
            break;
        }
        case ast::Tag::Assign: {
            auto &assign = ast::cast_tree_nonnull<ast::Assign>(expr);
            fn(assign.lhs);
            fn(assign.rhs);
            break;
        }
        case ast::Tag::Send: {
            auto &send = ast::cast_tree_nonnull<ast::Send>(expr);
            fn(send.recv);
            for (auto &arg : send.nonBlockArgs()) {
                fn(arg);
            }
            if (auto *block = send.rawBlock()) {
                fn(*block);
            }
            break;
        }
        case ast::Tag::Hash: {
            auto &hash = ast::cast_tree_nonnull<ast::Hash>(expr);
            for (auto &key : hash.keys) {
                fn(key);
            }
            for (auto &val : hash.values) {
                fn(val);
            }
            break;
        }
        case ast::Tag::Array: {
            auto &array = ast::cast_tree_nonnull<ast::Array>(expr);
            for (auto &elem : array.elems) {
                fn(elem);
            }
            break;
        }
        case ast::Tag::Cast: {
            auto &cast = ast::cast_tree_nonnull<ast::Cast>(expr);
            fn(cast.arg);
            fn(cast.typeExpr);
            break;
        }
        case ast::Tag::Block: {
            auto &block = ast::cast_tree_nonnull<ast::Block>(expr);
            for (auto &param : block.params) {
                fn(param);
            }
            fn(block.body);
            break;
        }
        case ast::Tag::InsSeq: {
            auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(expr);
            for (auto &stat : insSeq.stats) {
                fn(stat);
            }
            fn(insSeq.expr);
            break;
        }
        case ast::Tag::UnresolvedConstantLit: {
            fn(ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(expr).scope);
            break;
        }
        case ast::Tag::RestParam: {
            fn(ast::cast_tree_nonnull<ast::RestParam>(expr).expr);
            break;
        }
        case ast::Tag::KeywordArg: {
            fn(ast::cast_tree_nonnull<ast::KeywordArg>(expr).expr);
            break;
        }
        case ast::Tag::OptionalParam: {
            auto &opt = ast::cast_tree_nonnull<ast::OptionalParam>(expr);
            fn(opt.expr);
            fn(opt.default_);
            break;
        }
        case ast::Tag::BlockParam: {
            fn(ast::cast_tree_nonnull<ast::BlockParam>(expr).expr);
            break;
        }
        case ast::Tag::ShadowArg: {
            fn(ast::cast_tree_nonnull<ast::ShadowArg>(expr).expr);
            break;
        }
        case ast::Tag::EmptyTree:
        case ast::Tag::Retry:
        case ast::Tag::Local:
        case ast::Tag::UnresolvedIdent:
        case ast::Tag::Literal:
        case ast::Tag::ConstantLit:
        case ast::Tag::ZSuperArgs:
        case ast::Tag::RuntimeMethodDefinition:
        case ast::Tag::Self:
            break;
    }
}

void maybeUpdateLoc(const core::LocOffsets &loc, core::LocOffsets merged) {
    if (!loc.exists() || !loc.contains(merged)) {
        const_cast<core::LocOffsets &>(loc) = merged;
    }
}

struct LocSumComputer {
    void postTransformInsSeq(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(tree);
        auto merged = insSeq.expr.loc();
        for (auto &stat : insSeq.stats) {
            merged = merged.join(stat.loc());
        }
        maybeUpdateLoc(insSeq.loc, merged);
    }

    void postTransformIf(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &if_ = ast::cast_tree_nonnull<ast::If>(tree);
        auto merged = if_.cond.loc().join(if_.thenp.loc()).join(if_.elsep.loc());
        maybeUpdateLoc(if_.loc, merged);
    }

    void postTransformWhile(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &while_ = ast::cast_tree_nonnull<ast::While>(tree);
        auto merged = while_.cond.loc().join(while_.body.loc());
        maybeUpdateLoc(while_.loc, merged);
    }

    void postTransformBreak(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &break_ = ast::cast_tree_nonnull<ast::Break>(tree);
        maybeUpdateLoc(break_.loc, break_.expr.loc());
    }

    void postTransformNext(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &next = ast::cast_tree_nonnull<ast::Next>(tree);
        maybeUpdateLoc(next.loc, next.expr.loc());
    }

    void postTransformReturn(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &return_ = ast::cast_tree_nonnull<ast::Return>(tree);
        maybeUpdateLoc(return_.loc, return_.expr.loc());
    }

    void postTransformAssign(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &assign = ast::cast_tree_nonnull<ast::Assign>(tree);
        maybeUpdateLoc(assign.loc, assign.lhs.loc().join(assign.rhs.loc()));
    }

    void postTransformSend(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);
        auto merged = send.recv.loc();
        for (auto &arg : send.nonBlockArgs()) {
            merged = merged.join(arg.loc());
        }
        if (auto *block = send.rawBlock()) {
            merged = merged.join(block->loc());
        }
        maybeUpdateLoc(send.loc, merged);
    }

    void postTransformHash(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &hash = ast::cast_tree_nonnull<ast::Hash>(tree);
        core::LocOffsets merged;
        for (auto &key : hash.keys) {
            merged = merged.join(key.loc());
        }
        for (auto &val : hash.values) {
            merged = merged.join(val.loc());
        }
        maybeUpdateLoc(hash.loc, merged);
    }

    void postTransformArray(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &array = ast::cast_tree_nonnull<ast::Array>(tree);
        core::LocOffsets merged;
        for (auto &elem : array.elems) {
            merged = merged.join(elem.loc());
        }
        maybeUpdateLoc(array.loc, merged);
    }

    void postTransformCast(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &cast = ast::cast_tree_nonnull<ast::Cast>(tree);
        maybeUpdateLoc(cast.loc, cast.arg.loc().join(cast.typeExpr.loc()));
    }

    void postTransformBlock(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &block = ast::cast_tree_nonnull<ast::Block>(tree);
        auto merged = block.body.loc();
        for (auto &param : block.params) {
            merged = merged.join(param.loc());
        }
        maybeUpdateLoc(block.loc, merged);
    }

    void postTransformRescueCase(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &rescueCase = ast::cast_tree_nonnull<ast::RescueCase>(tree);
        auto merged = rescueCase.var.loc().join(rescueCase.body.loc());
        for (auto &exception : rescueCase.exceptions) {
            merged = merged.join(exception.loc());
        }
        maybeUpdateLoc(rescueCase.loc, merged);
    }

    void postTransformRescue(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &rescue = ast::cast_tree_nonnull<ast::Rescue>(tree);
        auto merged = rescue.body.loc().join(rescue.else_.loc()).join(rescue.ensure.loc());
        for (auto &rescueCase : rescue.rescueCases) {
            merged = merged.join(rescueCase.loc());
        }
        maybeUpdateLoc(rescue.loc, merged);
    }

    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        auto merged = methodDef.rhs.loc();
        for (auto &param : methodDef.params) {
            merged = merged.join(param.loc());
        }
        maybeUpdateLoc(methodDef.loc, merged);
    }

    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        auto merged = classDef.name.loc();
        for (auto &rhs : classDef.rhs) {
            merged = merged.join(rhs.loc());
        }
        for (auto &ancestor : classDef.ancestors) {
            merged = merged.join(ancestor.loc());
        }
        for (auto &ancestor : classDef.singletonAncestors) {
            merged = merged.join(ancestor.loc());
        }
        maybeUpdateLoc(classDef.loc, merged);
    }
};

// returns the selection and an approximation of what code will run after the selection
// for example if we select the condition of an if expression we will over approximate and say that the continuation is
// an InsSeq of the then and else branch expressions
optional<pair<vector<ast::ExpressionPtr>, vector<ast::ExpressionPtr>>>
getSelectionAndContinuation(const ast::ExpressionPtr &expr, const core::LocOffsets loc) {
    ENFORCE(expr.loc().contains(loc));

    switch (expr.tag()) {
        case ast::Tag::InsSeq: {
            auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(expr);
            vector<ast::ExpressionPtr> stats;
            for (auto &stat : insSeq.stats) {
                stats.push_back(stat.deepCopy());
            }
            stats.push_back(insSeq.expr.deepCopy());
            auto it =
                absl::c_find_if(stats, [loc](const ast::ExpressionPtr &stat) { return stat.loc().contains(loc); });
            if (it == stats.end()) {
                // TODO(bshu) review this part
                auto it1 = absl::c_find_if(
                    stats, [loc](const ast::ExpressionPtr &stat) { return loc.beginPos() <= stat.loc().beginPos(); });
                auto it2 = stats.end();
                for (auto it = stats.begin(); it < stats.end(); it++) {
                    if (it->loc().endPos() <= loc.endPos()) {
                        it2 = std::next(it);
                    }
                }
                ENFORCE(it1 <= it2);
                if (it1 == it2) {
                    return nullopt;
                }
                if (it1 == stats.end()) {
                    return nullopt;
                }
                vector<ast::ExpressionPtr> selection;
                for (auto it = it1; it < it2; it++) {
                    selection.push_back(it->deepCopy());
                }
                vector<ast::ExpressionPtr> continuation;
                for (auto it = it2; it < stats.end(); it++) {
                    continuation.push_back(it->deepCopy());
                }
                return {{move(selection), move(continuation)}};
            } else {
                auto res = getSelectionAndContinuation(*it, loc);
                if (!res) {
                    return nullopt;
                }
                auto [selection, continuation] = move(res.value());
                for (auto it1 = it + 1; it1 < stats.end(); it1++) {
                    continuation.push_back(it1->deepCopy());
                }
                return {{move(selection), move(continuation)}};
            }
            break;
        }
        case ast::Tag::ClassDef: {
            // should be called in a method body, so no classes
            return nullopt;
            break;
        }
        case ast::Tag::MethodDef: {
            // should be lifted out by this point
            return nullopt;
            break;
        }
        case ast::Tag::If: {
            auto &if_ = ast::cast_tree_nonnull<ast::If>(expr);
            if (if_.cond.loc().contains(loc)) {
                auto res = getSelectionAndContinuation(if_.cond, loc);
                if (!res) {
                    return nullopt;
                }
                auto [selection, continuation] = move(res.value());
                continuation.push_back(if_.thenp.deepCopy());
                continuation.push_back(if_.elsep.deepCopy());
                return {{move(selection), move(continuation)}};
            }
            if (if_.thenp.loc().contains(loc)) {
                return getSelectionAndContinuation(if_.thenp, loc);
            }
            if (if_.elsep.loc().contains(loc)) {
                return getSelectionAndContinuation(if_.elsep, loc);
            }
            vector<ast::ExpressionPtr> selection;
            selection.emplace_back(expr.deepCopy());
            vector<ast::ExpressionPtr> continuation;
            return {{move(selection), move(continuation)}};
        }
        case ast::Tag::While: {
            auto &while_ = ast::cast_tree_nonnull<ast::While>(expr);
            if (while_.cond.loc().contains(loc)) {
                auto res = getSelectionAndContinuation(while_.cond, loc);
                if (!res) {
                    return nullopt;
                }
                auto [selection, continuation] = move(res.value());
                continuation.push_back(while_.body.deepCopy());
                return {{move(selection), move(continuation)}};
            }
            if (while_.body.loc().contains(loc)) {
                return getSelectionAndContinuation(while_.body, loc);
            }
            vector<ast::ExpressionPtr> selection;
            selection.emplace_back(expr.deepCopy());
            vector<ast::ExpressionPtr> continuation;
            return {{move(selection), move(continuation)}};
        }
        case ast::Tag::Block: {
            auto &block = ast::cast_tree_nonnull<ast::Block>(expr);
            if (block.body.loc().contains(loc)) {
                return getSelectionAndContinuation(block.body, loc);
            }
            vector<ast::ExpressionPtr> selection;
            selection.emplace_back(expr.deepCopy());
            vector<ast::ExpressionPtr> continuation;
            return {{move(selection), move(continuation)}};
        }
        case ast::Tag::Rescue: {
            auto &rescue = ast::cast_tree_nonnull<ast::Rescue>(expr);
            if (rescue.body.loc().contains(loc)) {
                auto res = getSelectionAndContinuation(rescue.body, loc);
                if (!res) {
                    return nullopt;
                }
                auto [selection, continuation] = move(res.value());
                continuation.push_back(rescue.else_.deepCopy());
                continuation.push_back(rescue.ensure.deepCopy());
                return {{move(selection), move(continuation)}};
            }
            for (auto &rescueCase : rescue.rescueCases) {
                if (rescueCase.loc().contains(loc)) {
                    auto res = getSelectionAndContinuation(rescueCase, loc);
                    if (!res) {
                        return nullopt;
                    }
                    auto [selection, continuation] = move(res.value());
                    continuation.push_back(rescue.ensure.deepCopy());
                    return {{move(selection), move(continuation)}};
                }
            }
            if (rescue.else_.loc().contains(loc)) {
                auto res = getSelectionAndContinuation(rescue.else_, loc);
                if (!res) {
                    return nullopt;
                }
                auto [selection, continuation] = move(res.value());
                continuation.push_back(rescue.ensure.deepCopy());
                return {{move(selection), move(continuation)}};
            }
            if (rescue.ensure.loc().contains(loc)) {
                return getSelectionAndContinuation(rescue.ensure, loc);
            }
            vector<ast::ExpressionPtr> selection;
            selection.emplace_back(expr.deepCopy());
            vector<ast::ExpressionPtr> continuation;
            return {{move(selection), move(continuation)}};
        }
        case ast::Tag::RescueCase: {
            auto &rescueCase = ast::cast_tree_nonnull<ast::RescueCase>(expr);
            if (rescueCase.body.loc().contains(loc)) {
                return getSelectionAndContinuation(rescueCase.body, loc);
            }
            vector<ast::ExpressionPtr> selection;
            selection.emplace_back(expr.deepCopy());
            vector<ast::ExpressionPtr> continuation;
            return {{move(selection), move(continuation)}};
        }
        // TODO(bshu) handle these cases
        case ast::Tag::Send:
        case ast::Tag::Assign:
        case ast::Tag::Return:
        case ast::Tag::Break:
        case ast::Tag::Next:
        case ast::Tag::Retry:
        case ast::Tag::Cast:
        case ast::Tag::Hash:
        case ast::Tag::Array:
        case ast::Tag::Literal:
        case ast::Tag::Local:
        case ast::Tag::UnresolvedIdent:
        case ast::Tag::UnresolvedConstantLit:
        case ast::Tag::ConstantLit:
        case ast::Tag::ZSuperArgs:
        case ast::Tag::EmptyTree:
        case ast::Tag::RuntimeMethodDefinition:
        case ast::Tag::RestParam:
        case ast::Tag::KeywordArg:
        case ast::Tag::OptionalParam:
        case ast::Tag::BlockParam:
        case ast::Tag::ShadowArg:
        case ast::Tag::Self: {
            vector<ast::ExpressionPtr> selection;
            selection.emplace_back(expr.deepCopy());
            vector<ast::ExpressionPtr> continuation;
            return {{move(selection), move(continuation)}};
        }
    }

    return nullopt;
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
    auto intersectedLoc = core::LocOffsets{newStart, newEnd};
    config.logger->debug("ExtractMethod: intersected loc: {}", intersectedLoc.showRaw(gs, file));
    auto res = getSelectionAndContinuation(walk.enclosingMethod->rhs, intersectedLoc);
    if (!res) {
        config.logger->debug("ExtractMethod: could not determine selection and continuation");
        return {};
    }

    auto &[selection, continuation] = *res;
    config.logger->debug("ExtractMethod: selection size: {}, continuation size: {}", selection.size(),
                         continuation.size());
    for (auto &expr : selection) {
        config.logger->debug("ExtractMethod selection: {}", expr.toStringWithTabs(gs));
    }
    for (auto &expr : continuation) {
        config.logger->debug("ExtractMethod continuation: {}", expr.toStringWithTabs(gs));
    }

    return {};
}

} // namespace extract_method

} // namespace sorbet::realmain::lsp
