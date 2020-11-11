#ifndef SORBET_TREEMAP_H
#define SORBET_TREEMAP_H

#include "ast/Trees.h"
#include "common/has_member.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/errors/internal.h"
#include <memory>
#include <type_traits> // To use 'std::integral_constant'.
#include <typeinfo>

namespace sorbet::ast {

class FUNC_EXAMPLE {
public:
    // all members are optional, but METHOD NAMES MATTER
    // Not including the member will skip the branch
    // you may return the same pointer that you are given
    // caller is responsible to handle it
    TreePtr preTransformClassDef(core::MutableContext ctx, ClassDef *original);
    TreePtr postTransformClassDef(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformMethodDef(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformMethodDef(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformIf(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformIf(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformWhile(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformWhile(core::MutableContext ctx, TreePtr original);

    TreePtr postTransformBreak(core::MutableContext ctx, TreePtr original);

    TreePtr postTransformRetry(core::MutableContext ctx, TreePtr original);

    TreePtr postTransformNext(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformReturn(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformReturn(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformRescueCase(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformRescueCase(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformRescue(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformRescue(core::MutableContext ctx, TreePtr original);

    TreePtr postTransformUnresolvedIdent(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformAssign(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformAssign(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformSend(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformSend(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformHash(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformHash(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformArray(core::MutableContext ctx, TreePtr original);
    TreePtr postransformArray(core::MutableContext ctx, TreePtr original);

    TreePtr postTransformConstantLit(core::MutableContext ctx, TreePtr original);

    TreePtr postTransformUnresolvedConstantLit(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformBlock(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformBlock(core::MutableContext ctx, TreePtr original);

    TreePtr preTransformInsSeq(core::MutableContext ctx, TreePtr original);
    TreePtr postTransformInsSeq(core::MutableContext ctx, TreePtr original);
};

// NOTE: Implementations must use a context type parameter that `MutableContext` is convertable to.
// That is, either `Context` or `MutableContext`.
#define GENERATE_HAS_MEMBER_VISITOR(X) \
    GENERATE_HAS_MEMBER(X, std::declval<core::MutableContext>(), std::declval<TreePtr>())

// used to check for ABSENCE of method
GENERATE_HAS_MEMBER_VISITOR(preTransformUnresolvedIdent);
GENERATE_HAS_MEMBER_VISITOR(preTransformLocal);
GENERATE_HAS_MEMBER_VISITOR(preTransformUnresolvedConstantLit);
GENERATE_HAS_MEMBER_VISITOR(preTransformConstantLit);
GENERATE_HAS_MEMBER_VISITOR(preTransformLiteral);

#define GENERATE_POSTPONE_PRECLASS(X)                                                                            \
    GENERATE_CALL_MEMBER(preTransform##X, Exception::raise("should never be called. Incorrect use of TreeMap?"); \
                         return nullptr, std::declval<core::MutableContext>(), std::declval<TreePtr>())

#define GENERATE_POSTPONE_POSTCLASS(X)                                                                            \
    GENERATE_CALL_MEMBER(postTransform##X, Exception::raise("should never be called. Incorrect use of TreeMap?"); \
                         return nullptr, std::declval<core::MutableContext>(), std::declval<TreePtr>())

GENERATE_POSTPONE_PRECLASS(Expression);
GENERATE_POSTPONE_PRECLASS(ClassDef);
GENERATE_POSTPONE_PRECLASS(MethodDef);
GENERATE_POSTPONE_PRECLASS(If);
GENERATE_POSTPONE_PRECLASS(While);
GENERATE_POSTPONE_PRECLASS(Break);
GENERATE_POSTPONE_PRECLASS(Retry);
GENERATE_POSTPONE_PRECLASS(Next);
GENERATE_POSTPONE_PRECLASS(Return);
GENERATE_POSTPONE_PRECLASS(RescueCase);
GENERATE_POSTPONE_PRECLASS(Rescue);
GENERATE_POSTPONE_PRECLASS(Assign);
GENERATE_POSTPONE_PRECLASS(Send);
GENERATE_POSTPONE_PRECLASS(Hash);
GENERATE_POSTPONE_PRECLASS(Array);
GENERATE_POSTPONE_PRECLASS(Block);
GENERATE_POSTPONE_PRECLASS(InsSeq);
GENERATE_POSTPONE_PRECLASS(Cast);

GENERATE_POSTPONE_POSTCLASS(ClassDef);
GENERATE_POSTPONE_POSTCLASS(MethodDef);
GENERATE_POSTPONE_POSTCLASS(If);
GENERATE_POSTPONE_POSTCLASS(While);
GENERATE_POSTPONE_POSTCLASS(Break);
GENERATE_POSTPONE_POSTCLASS(Retry);
GENERATE_POSTPONE_POSTCLASS(Next);
GENERATE_POSTPONE_POSTCLASS(Return);
GENERATE_POSTPONE_POSTCLASS(RescueCase);
GENERATE_POSTPONE_POSTCLASS(Rescue);
GENERATE_POSTPONE_POSTCLASS(UnresolvedIdent);
GENERATE_POSTPONE_POSTCLASS(Assign);
GENERATE_POSTPONE_POSTCLASS(Send);
GENERATE_POSTPONE_POSTCLASS(Hash);
GENERATE_POSTPONE_POSTCLASS(Array);
GENERATE_POSTPONE_POSTCLASS(Local);
GENERATE_POSTPONE_POSTCLASS(Literal);
GENERATE_POSTPONE_POSTCLASS(UnresolvedConstantLit);
GENERATE_POSTPONE_POSTCLASS(ConstantLit);
GENERATE_POSTPONE_POSTCLASS(Block);
GENERATE_POSTPONE_POSTCLASS(InsSeq);
GENERATE_POSTPONE_POSTCLASS(Cast);

// Used to indicate that TreeMap has already reported location for this exception
struct ReportedRubyException {
    SorbetException reported;
    core::LocOffsets onLoc;
};

/**
 * Given a tree transformer FUNC transform a tree.
 * Tree is guaranteed to be visited in the definition order.
 * FUNC may maintain internal state.
 * @tparam tree transformer, see FUNC_EXAMPLE
 */
template <class FUNC, class CTX> class TreeMapper {
private:
    friend class TreeMap;

    FUNC &func;

    static_assert(!HAS_MEMBER_preTransformUnresolvedIdent<FUNC>(), "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformLiteral<FUNC>(), "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformUnresolvedConstantLit<FUNC>(), "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformConstantLit<FUNC>(), "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformLocal<FUNC>(), "use post*Transform instead");

    TreeMapper(FUNC &func) : func(func) {}

    TreePtr mapClassDef(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformClassDef<FUNC>()) {
            v = CALL_MEMBER_preTransformClassDef<FUNC>::call(func, ctx, std::move(v));
        }

        // We intentionally do not walk v->ancestors nor v->singletonAncestors.
        // They are guaranteed to be simple trees in the desugarer.
        for (auto &def : cast_tree<ClassDef>(v)->rhs) {
            def = mapIt(std::move(def), ctx.withOwner(cast_tree<ClassDef>(v)->symbol).withFile(ctx.file));
        }

        if constexpr (HAS_MEMBER_postTransformClassDef<FUNC>()) {
            return CALL_MEMBER_postTransformClassDef<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapMethodDef(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformMethodDef<FUNC>()) {
            v = CALL_MEMBER_preTransformMethodDef<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &arg : cast_tree<MethodDef>(v)->args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ = mapIt(std::move(optArg->default_), ctx.withOwner(cast_tree<MethodDef>(v)->symbol));
            }
        }
        cast_tree<MethodDef>(v)->rhs = mapIt(std::move(cast_tree<MethodDef>(v)->rhs),
                                             ctx.withOwner(cast_tree<MethodDef>(v)->symbol).withFile(ctx.file));

        if constexpr (HAS_MEMBER_postTransformMethodDef<FUNC>()) {
            return CALL_MEMBER_postTransformMethodDef<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapIf(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformIf<FUNC>()) {
            v = CALL_MEMBER_preTransformIf<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree<If>(v)->cond = mapIt(std::move(cast_tree<If>(v)->cond), ctx);
        cast_tree<If>(v)->thenp = mapIt(std::move(cast_tree<If>(v)->thenp), ctx);
        cast_tree<If>(v)->elsep = mapIt(std::move(cast_tree<If>(v)->elsep), ctx);

        if constexpr (HAS_MEMBER_postTransformIf<FUNC>()) {
            return CALL_MEMBER_postTransformIf<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapWhile(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformWhile<FUNC>()) {
            v = CALL_MEMBER_preTransformWhile<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree<While>(v)->cond = mapIt(std::move(cast_tree<While>(v)->cond), ctx);
        cast_tree<While>(v)->body = mapIt(std::move(cast_tree<While>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformWhile<FUNC>()) {
            return CALL_MEMBER_postTransformWhile<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapBreak(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBreak<FUNC>()) {
            return CALL_MEMBER_preTransformBreak<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree<Break>(v)->expr = mapIt(std::move(cast_tree<Break>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformBreak<FUNC>()) {
            return CALL_MEMBER_postTransformBreak<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }
    TreePtr mapRetry(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformRetry<FUNC>()) {
            return CALL_MEMBER_postTransformRetry<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapNext(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformNext<FUNC>()) {
            return CALL_MEMBER_preTransformNext<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree<Next>(v)->expr = mapIt(std::move(cast_tree<Next>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformNext<FUNC>()) {
            return CALL_MEMBER_postTransformNext<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapReturn(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformReturn<FUNC>()) {
            v = CALL_MEMBER_preTransformReturn<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree<Return>(v)->expr = mapIt(std::move(cast_tree<Return>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformReturn<FUNC>()) {
            return CALL_MEMBER_postTransformReturn<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapRescueCase(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescueCase<FUNC>()) {
            v = CALL_MEMBER_preTransformRescueCase<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &el : cast_tree<RescueCase>(v)->exceptions) {
            el = mapIt(std::move(el), ctx);
        }

        cast_tree<RescueCase>(v)->var = mapIt(std::move(cast_tree<RescueCase>(v)->var), ctx);

        cast_tree<RescueCase>(v)->body = mapIt(std::move(cast_tree<RescueCase>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformRescueCase<FUNC>()) {
            return CALL_MEMBER_postTransformRescueCase<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }
    TreePtr mapRescue(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescue<FUNC>()) {
            v = CALL_MEMBER_preTransformRescue<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree<Rescue>(v)->body = mapIt(std::move(cast_tree<Rescue>(v)->body), ctx);

        for (auto &el : cast_tree<Rescue>(v)->rescueCases) {
            ENFORCE(isa_tree<RescueCase>(el), "invalid tree where rescue case was expected");
            el = mapRescueCase(std::move(el), ctx);
            ENFORCE(isa_tree<RescueCase>(el), "rescue case was mapped into non-rescue case");
        }

        cast_tree<Rescue>(v)->else_ = mapIt(std::move(cast_tree<Rescue>(v)->else_), ctx);
        cast_tree<Rescue>(v)->ensure = mapIt(std::move(cast_tree<Rescue>(v)->ensure), ctx);

        if constexpr (HAS_MEMBER_postTransformRescue<FUNC>()) {
            return CALL_MEMBER_postTransformRescue<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapUnresolvedIdent(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedIdent<FUNC>()) {
            return CALL_MEMBER_postTransformUnresolvedIdent<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapAssign(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformAssign<FUNC>()) {
            v = CALL_MEMBER_preTransformAssign<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree<Assign>(v)->lhs = mapIt(std::move(cast_tree<Assign>(v)->lhs), ctx);
        cast_tree<Assign>(v)->rhs = mapIt(std::move(cast_tree<Assign>(v)->rhs), ctx);

        if constexpr (HAS_MEMBER_postTransformAssign<FUNC>()) {
            return CALL_MEMBER_postTransformAssign<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapSend(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformSend<FUNC>()) {
            v = CALL_MEMBER_preTransformSend<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree<Send>(v)->recv = mapIt(std::move(cast_tree<Send>(v)->recv), ctx);
        for (auto &arg : cast_tree<Send>(v)->args) {
            arg = mapIt(std::move(arg), ctx);
            ENFORCE(arg != nullptr);
        }

        if (cast_tree<Send>(v)->block) {
            auto nblock = mapBlock(std::move(cast_tree<Send>(v)->block), ctx);
            ENFORCE(isa_tree<Block>(nblock), "block was mapped into not-a block");
            cast_tree<Send>(v)->block = std::move(nblock);
        }

        if constexpr (HAS_MEMBER_postTransformSend<FUNC>()) {
            return CALL_MEMBER_postTransformSend<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapHash(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformHash<FUNC>()) {
            v = CALL_MEMBER_preTransformHash<FUNC>::call(func, ctx, std::move(v));
        }
        for (auto &key : cast_tree<Hash>(v)->keys) {
            key = mapIt(std::move(key), ctx);
        }

        for (auto &value : cast_tree<Hash>(v)->values) {
            value = mapIt(std::move(value), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>()) {
            return CALL_MEMBER_postTransformHash<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapArray(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformArray<FUNC>()) {
            v = CALL_MEMBER_preTransformArray<FUNC>::call(func, ctx, std::move(v));
        }
        for (auto &elem : cast_tree<Array>(v)->elems) {
            elem = mapIt(std::move(elem), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>()) {
            return CALL_MEMBER_postTransformArray<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapLiteral(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLiteral<FUNC>()) {
            return CALL_MEMBER_postTransformLiteral<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapUnresolvedConstantLit(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedConstantLit<FUNC>()) {
            return CALL_MEMBER_postTransformUnresolvedConstantLit<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapConstantLit(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformConstantLit<FUNC>()) {
            return CALL_MEMBER_postTransformConstantLit<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapBlock(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBlock<FUNC>()) {
            v = CALL_MEMBER_preTransformBlock<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &arg : cast_tree<Block>(v)->args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ = mapIt(std::move(optArg->default_), ctx);
            }
        }
        cast_tree<Block>(v)->body = mapIt(std::move(cast_tree<Block>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformBlock<FUNC>()) {
            return CALL_MEMBER_postTransformBlock<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapInsSeq(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformInsSeq<FUNC>()) {
            v = CALL_MEMBER_preTransformInsSeq<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &stat : cast_tree<InsSeq>(v)->stats) {
            stat = mapIt(std::move(stat), ctx);
        }

        cast_tree<InsSeq>(v)->expr = mapIt(std::move(cast_tree<InsSeq>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformInsSeq<FUNC>()) {
            return CALL_MEMBER_postTransformInsSeq<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapLocal(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLocal<FUNC>()) {
            return CALL_MEMBER_postTransformLocal<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapCast(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformCast<FUNC>()) {
            v = CALL_MEMBER_preTransformCast<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree<Cast>(v)->arg = mapIt(std::move(cast_tree<Cast>(v)->arg), ctx);

        if constexpr (HAS_MEMBER_postTransformCast<FUNC>()) {
            return CALL_MEMBER_postTransformCast<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapIt(TreePtr what, CTX ctx) {
        if (what == nullptr) {
            return what;
        }
        auto loc = what.loc();

        try {
            // TODO: reorder by frequency
            if constexpr (HAS_MEMBER_preTransformExpression<FUNC>()) {
                what = CALL_MEMBER_preTransformExpression<FUNC>::call(func, ctx, std::move(what));
            }

            switch (what.tag()) {
                case Tag::EmptyTree:
                    return what;

                case Tag::Send:
                    return mapSend(std::move(what), ctx);

                case Tag::ClassDef:
                    return mapClassDef(std::move(what), ctx);

                case Tag::MethodDef:
                    return mapMethodDef(std::move(what), ctx);

                case Tag::If:
                    return mapIf(std::move(what), ctx);

                case Tag::While:
                    return mapWhile(std::move(what), ctx);

                case Tag::Break:
                    return mapBreak(std::move(what), ctx);

                case Tag::Retry:
                    return mapRetry(std::move(what), ctx);

                case Tag::Next:
                    return mapNext(std::move(what), ctx);

                case Tag::Return:
                    return mapReturn(std::move(what), ctx);

                case Tag::RescueCase:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::Rescue:
                    return mapRescue(std::move(what), ctx);

                case Tag::Local:
                    return mapLocal(std::move(what), ctx);

                case Tag::UnresolvedIdent:
                    return mapUnresolvedIdent(std::move(what), ctx);

                case Tag::RestArg:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::KeywordArg:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::OptionalArg:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::BlockArg:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::ShadowArg:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::Assign:
                    return mapAssign(std::move(what), ctx);

                case Tag::Cast:
                    return mapCast(std::move(what), ctx);

                case Tag::Hash:
                    return mapHash(std::move(what), ctx);

                case Tag::Array:
                    return mapArray(std::move(what), ctx);

                case Tag::Literal:
                    return mapLiteral(std::move(what), ctx);

                case Tag::UnresolvedConstantLit:
                    return mapUnresolvedConstantLit(std::move(what), ctx);

                case Tag::ConstantLit:
                    return mapConstantLit(std::move(what), ctx);

                case Tag::ZSuperArgs:
                    return what;

                case Tag::Block:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::InsSeq:
                    return mapInsSeq(std::move(what), ctx);
            }
        } catch (SorbetException &e) {
            Exception::failInFuzzer();

            throw ReportedRubyException{e, loc};
        }
    }
}; // namespace sorbet::ast

class TreeMap {
public:
    template <typename CTX, typename FUNC> static TreePtr apply(CTX ctx, FUNC &func, TreePtr to) {
        TreeMapper<FUNC, CTX> walker(func);
        try {
            return walker.mapIt(std::move(to), ctx);
        } catch (ReportedRubyException &exception) {
            Exception::failInFuzzer();
            if (auto e = ctx.beginError(exception.onLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Failed to process tree (backtrace is above)");
            }
            throw exception.reported;
        }
    }
};

/**
 * Given a tree transformer FUNC transform a tree.
 * Tree is guaranteed to be visited in the definition order.
 * FUNC may maintain internal state.
 * @tparam tree transformer, see FUNC_EXAMPLE
 */
template <class FUNC, class CTX> class ShallowMapper {
private:
    friend class ShallowMap;

    FUNC &func;

    static_assert(!HAS_MEMBER_preTransformUnresolvedIdent<FUNC>(), "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformLiteral<FUNC>(), "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformUnresolvedConstantLit<FUNC>(), "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformConstantLit<FUNC>(), "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformLocal<FUNC>(), "use post*Transform instead");

    ShallowMapper(FUNC &func) : func(func) {}

    TreePtr mapClassDef(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformClassDef<FUNC>()) {
            v = CALL_MEMBER_preTransformClassDef<FUNC>::call(func, ctx, std::move(v));
        }

        // We intentionally do not walk v->ancestors nor v->singletonAncestors.
        // They are guaranteed to be simple trees in the desugarer.
        for (auto &def : cast_tree<ClassDef>(v)->rhs) {
            def = mapIt(std::move(def), ctx.withOwner(cast_tree<ClassDef>(v)->symbol));
        }

        if constexpr (HAS_MEMBER_postTransformClassDef<FUNC>()) {
            return CALL_MEMBER_postTransformClassDef<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapMethodDef(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformMethodDef<FUNC>()) {
            v = CALL_MEMBER_preTransformMethodDef<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &arg : cast_tree<MethodDef>(v)->args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ = mapIt(std::move(optArg->default_), ctx.withOwner(cast_tree<MethodDef>(v)->symbol));
            }
        }
        // because this is a ShallowMap, we do not map over the body of the method

        if constexpr (HAS_MEMBER_postTransformMethodDef<FUNC>()) {
            return CALL_MEMBER_postTransformMethodDef<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapIf(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformIf<FUNC>()) {
            v = CALL_MEMBER_preTransformIf<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree<If>(v)->cond = mapIt(std::move(cast_tree<If>(v)->cond), ctx);
        cast_tree<If>(v)->thenp = mapIt(std::move(cast_tree<If>(v)->thenp), ctx);
        cast_tree<If>(v)->elsep = mapIt(std::move(cast_tree<If>(v)->elsep), ctx);

        if constexpr (HAS_MEMBER_postTransformIf<FUNC>()) {
            return CALL_MEMBER_postTransformIf<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapWhile(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformWhile<FUNC>()) {
            v = CALL_MEMBER_preTransformWhile<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree<While>(v)->cond = mapIt(std::move(cast_tree<While>(v)->cond), ctx);
        cast_tree<While>(v)->body = mapIt(std::move(cast_tree<While>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformWhile<FUNC>()) {
            return CALL_MEMBER_postTransformWhile<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapBreak(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBreak<FUNC>()) {
            return CALL_MEMBER_preTransformBreak<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree<Break>(v)->expr = mapIt(std::move(cast_tree<Break>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformBreak<FUNC>()) {
            return CALL_MEMBER_postTransformBreak<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }
    TreePtr mapRetry(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformRetry<FUNC>()) {
            return CALL_MEMBER_postTransformRetry<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapNext(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformNext<FUNC>()) {
            return CALL_MEMBER_preTransformNext<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree<Next>(v)->expr = mapIt(std::move(cast_tree<Next>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformNext<FUNC>()) {
            return CALL_MEMBER_postTransformNext<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapReturn(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformReturn<FUNC>()) {
            v = CALL_MEMBER_preTransformReturn<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree<Return>(v)->expr = mapIt(std::move(cast_tree<Return>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformReturn<FUNC>()) {
            return CALL_MEMBER_postTransformReturn<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapRescueCase(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescueCase<FUNC>()) {
            v = CALL_MEMBER_preTransformRescueCase<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &el : cast_tree<RescueCase>(v)->exceptions) {
            el = mapIt(std::move(el), ctx);
        }

        cast_tree<RescueCase>(v)->var = mapIt(std::move(cast_tree<RescueCase>(v)->var), ctx);

        cast_tree<RescueCase>(v)->body = mapIt(std::move(cast_tree<RescueCase>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformRescueCase<FUNC>()) {
            return CALL_MEMBER_postTransformRescueCase<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }
    TreePtr mapRescue(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescue<FUNC>()) {
            v = CALL_MEMBER_preTransformRescue<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree<Rescue>(v)->body = mapIt(std::move(cast_tree<Rescue>(v)->body), ctx);

        for (auto &el : cast_tree<Rescue>(v)->rescueCases) {
            ENFORCE(isa_tree<RescueCase>(el), "invalid tree where rescue case was expected");
            el = mapRescueCase(std::move(el), ctx);
            ENFORCE(isa_tree<RescueCase>(el), "rescue case was mapped into non-rescue case");
        }

        cast_tree<Rescue>(v)->else_ = mapIt(std::move(cast_tree<Rescue>(v)->else_), ctx);
        cast_tree<Rescue>(v)->ensure = mapIt(std::move(cast_tree<Rescue>(v)->ensure), ctx);

        if constexpr (HAS_MEMBER_postTransformRescue<FUNC>()) {
            return CALL_MEMBER_postTransformRescue<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapUnresolvedIdent(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedIdent<FUNC>()) {
            return CALL_MEMBER_postTransformUnresolvedIdent<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapAssign(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformAssign<FUNC>()) {
            v = CALL_MEMBER_preTransformAssign<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree<Assign>(v)->lhs = mapIt(std::move(cast_tree<Assign>(v)->lhs), ctx);
        cast_tree<Assign>(v)->rhs = mapIt(std::move(cast_tree<Assign>(v)->rhs), ctx);

        if constexpr (HAS_MEMBER_postTransformAssign<FUNC>()) {
            return CALL_MEMBER_postTransformAssign<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapSend(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformSend<FUNC>()) {
            v = CALL_MEMBER_preTransformSend<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree<Send>(v)->recv = mapIt(std::move(cast_tree<Send>(v)->recv), ctx);
        for (auto &arg : cast_tree<Send>(v)->args) {
            arg = mapIt(std::move(arg), ctx);
            ENFORCE(arg != nullptr);
        }

        if (cast_tree<Send>(v)->block) {
            auto nblock = mapBlock(std::move(cast_tree<Send>(v)->block), ctx);
            ENFORCE(isa_tree<Block>(nblock), "block was mapped into not-a block");
            cast_tree<Send>(v)->block = std::move(nblock);
        }

        if constexpr (HAS_MEMBER_postTransformSend<FUNC>()) {
            return CALL_MEMBER_postTransformSend<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapHash(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformHash<FUNC>()) {
            v = CALL_MEMBER_preTransformHash<FUNC>::call(func, ctx, std::move(v));
        }
        for (auto &key : cast_tree<Hash>(v)->keys) {
            key = mapIt(std::move(key), ctx);
        }

        for (auto &value : cast_tree<Hash>(v)->values) {
            value = mapIt(std::move(value), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>()) {
            return CALL_MEMBER_postTransformHash<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapArray(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformArray<FUNC>()) {
            v = CALL_MEMBER_preTransformArray<FUNC>::call(func, ctx, std::move(v));
        }
        for (auto &elem : cast_tree<Array>(v)->elems) {
            elem = mapIt(std::move(elem), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>()) {
            return CALL_MEMBER_postTransformArray<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapLiteral(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLiteral<FUNC>()) {
            return CALL_MEMBER_postTransformLiteral<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapUnresolvedConstantLit(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedConstantLit<FUNC>()) {
            return CALL_MEMBER_postTransformUnresolvedConstantLit<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapConstantLit(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformConstantLit<FUNC>()) {
            return CALL_MEMBER_postTransformConstantLit<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapBlock(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBlock<FUNC>()) {
            v = CALL_MEMBER_preTransformBlock<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &arg : cast_tree<Block>(v)->args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ = mapIt(std::move(optArg->default_), ctx);
            }
        }
        cast_tree<Block>(v)->body = mapIt(std::move(cast_tree<Block>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformBlock<FUNC>()) {
            return CALL_MEMBER_postTransformBlock<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapInsSeq(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformInsSeq<FUNC>()) {
            v = CALL_MEMBER_preTransformInsSeq<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &stat : cast_tree<InsSeq>(v)->stats) {
            stat = mapIt(std::move(stat), ctx);
        }

        cast_tree<InsSeq>(v)->expr = mapIt(std::move(cast_tree<InsSeq>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformInsSeq<FUNC>()) {
            return CALL_MEMBER_postTransformInsSeq<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapLocal(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLocal<FUNC>()) {
            return CALL_MEMBER_postTransformLocal<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    TreePtr mapCast(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformCast<FUNC>()) {
            v = CALL_MEMBER_preTransformCast<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree<Cast>(v)->arg = mapIt(std::move(cast_tree<Cast>(v)->arg), ctx);

        if constexpr (HAS_MEMBER_postTransformCast<FUNC>()) {
            return CALL_MEMBER_postTransformCast<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    TreePtr mapIt(TreePtr what, CTX ctx) {
        if (what == nullptr) {
            return what;
        }
        auto loc = what.loc();

        try {
            // TODO: reorder by frequency
            if constexpr (HAS_MEMBER_preTransformExpression<FUNC>()) {
                what = CALL_MEMBER_preTransformExpression<FUNC>::call(func, ctx, std::move(what));
            }

            switch (what.tag()) {
                case Tag::EmptyTree:
                    return what;

                case Tag::Send:
                    return mapSend(std::move(what), ctx);

                case Tag::ClassDef:
                    return mapClassDef(std::move(what), ctx);

                case Tag::MethodDef:
                    return mapMethodDef(std::move(what), ctx);

                case Tag::If:
                    return mapIf(std::move(what), ctx);

                case Tag::While:
                    return mapWhile(std::move(what), ctx);

                case Tag::Break:
                    return mapBreak(std::move(what), ctx);

                case Tag::Retry:
                    return mapRetry(std::move(what), ctx);

                case Tag::Next:
                    return mapNext(std::move(what), ctx);

                case Tag::Return:
                    return mapReturn(std::move(what), ctx);

                case Tag::RescueCase:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::Rescue:
                    return mapRescue(std::move(what), ctx);

                case Tag::Local:
                    return mapLocal(std::move(what), ctx);

                case Tag::UnresolvedIdent:
                    return mapUnresolvedIdent(std::move(what), ctx);

                case Tag::RestArg:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::KeywordArg:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::OptionalArg:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::BlockArg:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::ShadowArg:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::Assign:
                    return mapAssign(std::move(what), ctx);

                case Tag::Cast:
                    return mapCast(std::move(what), ctx);

                case Tag::Hash:
                    return mapHash(std::move(what), ctx);

                case Tag::Array:
                    return mapArray(std::move(what), ctx);

                case Tag::Literal:
                    return mapLiteral(std::move(what), ctx);

                case Tag::UnresolvedConstantLit:
                    return mapUnresolvedConstantLit(std::move(what), ctx);

                case Tag::ConstantLit:
                    return mapConstantLit(std::move(what), ctx);

                case Tag::ZSuperArgs:
                    return what;

                case Tag::Block:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::InsSeq:
                    return mapInsSeq(std::move(what), ctx);
            }
        } catch (SorbetException &e) {
            Exception::failInFuzzer();

            throw ReportedRubyException{e, loc};
        }
    }
};

class ShallowMap {
public:
    template <typename CTX, typename FUNC> static TreePtr apply(CTX ctx, FUNC &func, TreePtr to) {
        ShallowMapper<FUNC, CTX> walker(func);
        try {
            return walker.mapIt(std::move(to), ctx);
        } catch (ReportedRubyException &exception) {
            Exception::failInFuzzer();
            if (auto e = ctx.beginError(exception.onLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Failed to process tree (backtrace is above)");
            }
            throw exception.reported;
        }
    }
};

} // namespace sorbet::ast
#endif // SORBET_TREEMAP_H
