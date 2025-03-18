#include "rewriter/RBSAssertions.h"

#include "absl/strings/str_split.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rbs/RBSParser.h"
#include "rbs/TypeTranslator.h"
#include "rewriter/util/Util.h"
#include <regex>

using namespace std;
using namespace sorbet;

namespace sorbet::rewriter {

namespace {

/**
 * Check if the given expression is a desugared identifier
 */
bool isDesugarIdent(core::MutableContext ctx, const ast::ExpressionPtr &expr) {
    auto ident = ast::cast_tree<ast::UnresolvedIdent>(expr);
    return ident && ident->kind == ast::UnresolvedIdent::Kind::Local &&
           (ident->name.isUniqueNameOf(ctx, core::Names::andAnd()) ||
            ident->name.isUniqueNameOf(ctx, core::Names::assignTemp()) ||
            ident->name.isUniqueNameOf(ctx, core::Names::destructureArg()) ||
            ident->name.isUniqueNameOf(ctx, core::Names::forTemp()) ||
            ident->name.isUniqueNameOf(ctx, core::Names::hashTemp()) ||
            ident->name.isUniqueNameOf(ctx, core::Names::orOr()) ||
            ident->name.isUniqueNameOf(ctx, core::Names::rescueTemp()) ||
            ident->name.isUniqueNameOf(ctx, core::Names::statTemp()));
}

/**
 * Check if the given expression is a send on a desugared identifier receiver
 */
bool isDesugarSend(core::MutableContext ctx, const ast::ExpressionPtr &expr) {
    auto send = ast::cast_tree<ast::Send>(expr);
    while (send) {
        if (isDesugarIdent(ctx, send->recv)) {
            return true;
        }

        send = ast::cast_tree<ast::Send>(send->recv);
    }

    return false;
}

class RBSAssertionsWalker final {
private:
    const vector<pair<core::LocOffsets, core::NameRef>> &typeParams;

    /**
     * Check if the given range contains a heredoc marker `<<-` or `<<~`
     */
    bool hasHeredocMarker(core::MutableContext ctx, const uint32_t fromPos, const uint32_t toPos) {
        string source(ctx.file.data(ctx).source().substr(fromPos, toPos - fromPos));
        regex heredoc_pattern("(\\s+=\\s<<-|~)");
        smatch matches;
        return regex_search(source, matches, heredoc_pattern);
    }

    /**
     * Check if the given send is a magic string interpolation
     */
    bool isMagicString(const ast::Send &send) {
        auto recv = ast::cast_tree<ast::ConstantLit>(send.recv);
        if (!recv) {
            return false;
        }

        if (recv->symbol() != core::Symbols::Magic()) {
            return false;
        }

        if (send.fun != core::Names::stringInterpolate()) {
            return false;
        }

        if (send.numPosArgs() < 1) {
            return false;
        }

        return true;
    }

    /**
     * Check if the given expression is a heredoc
     */
    bool isHeredoc(core::MutableContext ctx, core::LocOffsets assignLoc, const ast::ExpressionPtr &expr) {
        auto result = false;

        typecase(
            expr,
            [&](const ast::Literal &lit) {
                if (lit.isString()) {
                    // For some reason, heredoc strings are parser differently if they contain a single line or more.
                    //
                    // Single line heredocs do not contain the `<<-` or `<<~` markers inside their location.
                    //
                    // For example, this heredoc:
                    //
                    //     <<~MSG
                    //       foo
                    //     MSG

                    // has the `<<-` or `<<~` markers **outside** its location.
                    //
                    // While this heredoc:
                    //
                    //     <<~MSG
                    //       foo
                    //     MSG
                    //
                    // has the `<<-` or `<<~` markers **inside** its location.

                    auto lineStart = core::Loc::offset2Pos(ctx.file.data(ctx), lit.loc.beginLoc).line;
                    auto lineEnd = core::Loc::offset2Pos(ctx.file.data(ctx), lit.loc.endLoc).line;

                    if (lineEnd - lineStart <= 1) {
                        // Single line heredoc, we look for the heredoc marker outside, ie. between the assign `=` sign
                        // and the begining of the string.
                        if (hasHeredocMarker(ctx, assignLoc.endPos(), lit.loc.beginPos())) {
                            result = true;
                        }
                    } else {
                        // Multi-line heredoc, we look for the heredoc marker inside the string itself.
                        if (hasHeredocMarker(ctx, lit.loc.beginPos(), lit.loc.endPos())) {
                            result = true;
                        }
                    }
                }
            },
            [&](const ast::Send &send) {
                if (isMagicString(send)) {
                    result = hasHeredocMarker(ctx, assignLoc.endPos(), send.getPosArg(0).loc().beginPos());
                } else {
                    result = isHeredoc(ctx, assignLoc, send.recv);
                }
            },
            [&](const ast::ExpressionPtr &expr) { result = false; });

        return result;
    }

    /**
     * Find the RBS comment for the given assign
     */
    optional<rbs::Comment> findRBSComments(core::MutableContext ctx, ast::Assign *assign) {
        auto source = ctx.file.data(ctx).source();

        // We want to find the comment right after the end of the assign
        auto startingLoc = assign->loc.endPos();

        // On heredocs, adding the comment at the end of the assign won't work because this is invalid Ruby syntax:
        // ```
        // <<~MSG
        //   foo
        // MSG #: String
        // ```
        // We add a special case for heredocs to allow adding the comment at the end of the assign:
        // ```
        // <<~MSG #: String
        //   foo
        // MSG
        // ```
        if (isHeredoc(ctx, assign->lhs.loc(), assign->rhs)) {
            startingLoc = assign->loc.beginPos();
        }

        // Get the position of the end of the line from the startingLoc
        auto endOfLine = source.find('\n', startingLoc);
        if (endOfLine == string::npos) {
            return nullopt;
        }

        // Check between the startingLoc and the end of the line for a `#: ...` comment
        auto comment = source.substr(startingLoc, endOfLine - startingLoc);

        // Find the position of the `#:` in the comment
        auto commentStart = comment.find("#:");
        if (commentStart == string::npos) {
            return nullopt;
        }

        // Adjust the location to be the correct position depending on the number of spaces after the `#:`
        auto offset = 0;
        for (auto i = startingLoc + commentStart + 2; i < endOfLine; i++) {
            if (source[i] == ' ') {
                offset++;
            } else {
                break;
            }
        }

        return rbs::Comment{
            core::LocOffsets{startingLoc + (uint32_t)commentStart + offset, static_cast<uint32_t>(endOfLine)},
            absl::StripAsciiWhitespace(comment.substr(commentStart + 2))};
    }

    /**
     * Get the RBS type from the given assign
     */
    ast::ExpressionPtr getRBSAssertionType(core::MutableContext ctx, ast::Assign *assign) {
        auto assertion = findRBSComments(ctx, assign);

        if (!assertion) {
            return nullptr;
        }

        auto result = rbs::RBSParser::parseType(ctx, *assertion);
        if (result.second) {
            if (auto e = ctx.beginError(result.second->loc, core::errors::Rewriter::RBSSyntaxError)) {
                e.setHeader("Failed to parse RBS type ({})", result.second->message);
            }
            return nullptr;
        }

        auto rbsType = move(result.first.value());
        return rbs::TypeTranslator::toExpressionPtr(ctx, typeParams, rbsType.node.get(), assertion->loc);
    }

    /**
     * Transform the given assign into an `Assign` with a `Let` if it has an RBS comment
     */
    ast::ExpressionPtr transformAssign(core::MutableContext ctx, ast::Assign *assign) {
        if (auto type = getRBSAssertionType(ctx, assign)) {
            auto rhs = ast::MK::Let(type.loc(), move(assign->rhs), move(type));
            return ast::MK::Assign(assign->loc, move(assign->lhs), move(rhs));
        }

        return nullptr;
    }

public:
    RBSAssertionsWalker(const vector<pair<core::LocOffsets, core::NameRef>> &typeParams) : typeParams(typeParams) {}

    void postTransformAssign(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto assign = ast::cast_tree<ast::Assign>(tree);

        // Desugar added a bunch of magic variables, we don't want to add assertions from RBS to them
        if (isDesugarIdent(ctx, assign->lhs)) {
            return;
        }

        // It's hard to distinguish between a desugared expression and a normal one,
        // sometimes we also have to check if the rhs is a send on a expression added by the desugarer.
        if (isDesugarSend(ctx, assign->rhs)) {
            return;
        }

        if (auto expr = transformAssign(ctx, assign)) {
            tree = move(expr);
        }
    }

    /*
     * We need to handle expressions such as `@x ||= 42 #: Integer?` into the proper `T.let`.
     *
     * `||=` assigns are already desugared into an else if such as:
     *
     *     if @x
     *       @x
     *     else
     *       @x = 42
     *     end
     *
     * Also in desugar, there is a special case for `@x ||= T.let(42, T.nilable(Integer))` to return a
     * non-nilable value:
     *
     *     if @x
     *       @x
     *     else
     *       @x = T.let(@x, T.nilable(Integer))
     *       <tmp> = 42
     *       @x = <tmp>
     *     end
     *
     * Since with RBS comments we introduce the `T.let` after the desugar we need to manually emulate this
     * behavior and transform the `else` branch accordingly.
     *
     * Will apply the transformation if and only if:
     *
     *  1. The condition is an `UnresolvedIdent` and it's an instance variable
     *  2. The `then` branch returns the same instance variable
     *  3. The `else` branch is an `Assign` to the same instance variable using a `T.let` of a nilable type
     */
    void postTransformIf(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto iff = ast::cast_tree<ast::If>(tree);

        // Is the condition an `UnresolvedIdent` and it's an instance variable?
        auto condIdent = ast::cast_tree<ast::UnresolvedIdent>(iff->cond);
        if (!condIdent || condIdent->kind != ast::UnresolvedIdent::Kind::Instance) {
            return;
        }

        // Is the `then` branch an `UnresolvedIdent` and it's the same instance variable?
        auto name = condIdent->name;
        auto thenIdent = ast::cast_tree<ast::UnresolvedIdent>(iff->thenp);
        if (!thenIdent || thenIdent->name != name) {
            return;
        }

        // Is the `else` branch an `Assign` to the same instance variable using a `T.let` of a nilable type?
        auto elseAssign = ast::cast_tree<ast::Assign>(iff->elsep);
        if (!elseAssign) {
            return;
        }
        auto elseAssignLhs = ast::cast_tree<ast::UnresolvedIdent>(elseAssign->lhs);
        if (!elseAssignLhs || elseAssignLhs->name != name) {
            return;
        }
        auto elseAssignCast = ast::cast_tree<ast::Cast>(elseAssign->rhs);
        if (!elseAssignCast) {
            return;
        }
        if (!ast::MK::isTNilable(elseAssignCast->typeExpr)) {
            return;
        }

        // We have a else branch that looks like this:
        //
        //     @x = T.let(42, T.nilable(Integer))
        //
        // We need to transform it into:
        //
        //     @x = T.let(@x, T.nilable(Integer))
        //     <tmp> = 42
        //     @x = <tmp>

        auto stats = ast::InsSeq::STATS_store();
        stats.reserve(3);

        auto let = ast::MK::Let(elseAssignCast->loc, ast::MK::cpRef(elseAssign->lhs), move(elseAssignCast->typeExpr));
        auto newLet = ast::MK::Assign(elseAssign->loc, ast::MK::cpRef(elseAssign->lhs), move(let));
        stats.push_back(move(newLet));

        auto tempLocal = ast::MK::Local(elseAssignCast->loc, core::Names::statTemp());
        auto newTemp = ast::MK::Assign(elseAssignCast->loc, ast::MK::cpRef(tempLocal), move(elseAssignCast->arg));
        stats.push_back(move(newTemp));

        auto expr = ast::MK::Assign(elseAssign->loc, move(elseAssign->lhs), move(tempLocal));

        iff->elsep = ast::MK::InsSeq(iff->elsep.loc(), move(stats), move(expr));

        tree = ast::MK::If(iff->loc, move(iff->cond), move(iff->thenp), move(iff->elsep));
    }

    /*
     * Splat expression such as:
     *
     *     x, y = value #: [Integer, String]
     *
     * are desugared using a Magic.expand-splat call:
     *
     *     <assignTemp>$1 = value
     *     <assignTemp>$2 = ::<Magic>.<expand-splat>(<assignTemp>$1 ...)
     *     x = <assignTemp>$2[0]
     *     y = <assignTemp>$2[1]
     *     <assignTemp>$1
     *
     * We need to transform this into:
     *
     *     <assignTemp>$1 = T.let(value, [Integer, String])
     *     <assignTemp>$2 = ::<Magic>.<expand-splat>(<assignTemp>$1 ...)
     *     x = <assignTemp>$2[0]
     *     y = <assignTemp>$2[1]
     *     <assignTemp>$1
     */
    void postTransformInsSeq(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        // Are we in an ins seq?
        auto insSeq = ast::cast_tree<ast::InsSeq>(tree);
        if (!insSeq) {
            return;
        }

        // Do we have at least 2 stats?
        // So we have the `<assignTemp>$1 = T.let...` and `<assignTemp>$2 = <Magic>.expand-splat(...)`
        if (insSeq->stats.size() < 2) {
            return;
        }

        // Is the first stat an assign to a desugared identifier?
        auto assignTemp1 = ast::cast_tree<ast::Assign>(insSeq->stats[0]);
        if (!assignTemp1 || !isDesugarIdent(ctx, assignTemp1->lhs)) {
            return;
        }

        // Is the second stat also an assign to a desugared identifier?
        auto assignTemp2 = ast::cast_tree<ast::Assign>(insSeq->stats[1]);
        if (!assignTemp2 || !isDesugarIdent(ctx, assignTemp2->lhs)) {
            return;
        }

        // Is the rhs of the second stat a call to `<Magic>.<expand-splat>`?
        auto expandSplat = ast::cast_tree<ast::Send>(assignTemp2->rhs);
        if (!expandSplat || expandSplat->fun != core::Names::expandSplat()) {
            return;
        }

        // Is the receiver of the send the `<Magic>` constant?
        auto recv = ast::cast_tree<ast::ConstantLit>(expandSplat->recv);
        if (!recv || recv->symbol() != core::Symbols::Magic()) {
            return;
        }

        // Transform the first assign into an `Assign` with a `Let` if it has an RBS comment
        if (auto newAssign = transformAssign(ctx, assignTemp1)) {
            insSeq->stats[0] = move(newAssign);
        }

        return;
    }
};

/**
 * Parse the type parameters from the previous statement
 *
 * Given a case like this one:
 *
 *     #: [X] (X) -> void
 *     def foo(x)
 *       y = nil #: X?
 *     end
 *
 * We need to be aware of the type parameter `X` so we can use it to resolve the type of `y`.
 */
vector<pair<core::LocOffsets, core::NameRef>> parseTypeParams(core::MutableContext ctx,
                                                              const ast::ExpressionPtr *prevStat) {
    vector<pair<core::LocOffsets, core::NameRef>> typeParams;

    // Do we have a previous statement?
    if (!prevStat) {
        return typeParams;
    }

    // Is the previous statement a sig call?
    auto *sig = rewriter::ASTUtil::castSig(*prevStat);
    if (sig == nullptr) {
        return typeParams;
    }

    // Make sure the sig has a block
    auto *block = sig->block();
    if (block == nullptr) {
        return typeParams;
    }

    // Does the sig contain a `type_parameters()` invocation?
    auto send = ast::cast_tree<ast::Send>(block->body);
    while (send && send->fun != core::Names::typeParameters()) {
        send = ast::cast_tree<ast::Send>(send->recv);
    }
    if (send == nullptr) {
        return typeParams;
    }

    // Collect the type parameters
    for (auto &arg : send->posArgs()) {
        auto sym = ast::cast_tree<ast::Literal>(arg);
        if (sym == nullptr) {
            continue;
        }

        if (!sym->isSymbol()) {
            continue;
        }

        typeParams.emplace_back(arg.loc(), sym->asName());
    }

    return typeParams;
}

} // namespace

void RBSAssertions::run(core::MutableContext ctx, ast::ClassDef *classDef) {
    ast::ExpressionPtr *prevStat = nullptr;

    for (auto &stat : classDef->rhs) {
        typecase(
            stat,
            [&](ast::ClassDef &cdef) {
                // no-op to avoid visiting the same class defs twice
            },
            [&](ast::MethodDef &mdef) {
                // When visiting a method definition, we need to parse the type parameters from the previous
                // statement
                auto typeParams = parseTypeParams(ctx, prevStat);
                RBSAssertionsWalker walker(typeParams);
                ast::TreeWalk::apply(ctx, walker, stat);
            },
            [&](ast::ExpressionPtr &e) {
                // For bare statements, we don't have any type parameters
                vector<pair<core::LocOffsets, core::NameRef>> typeParams = {};
                RBSAssertionsWalker walker(typeParams);
                ast::TreeWalk::apply(ctx, walker, stat);
            });

        prevStat = &stat;
    }
}

} // namespace sorbet::rewriter

// TODO: fix heredoc location
