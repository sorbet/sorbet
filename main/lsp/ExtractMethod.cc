#include "main/lsp/ExtractMethod.h"
#include "ast/treemap/treemap.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
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
