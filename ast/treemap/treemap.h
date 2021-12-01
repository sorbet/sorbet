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
    ExpressionPtr preTransformClassDef(core::MutableContext ctx, ClassDef *original);
    ExpressionPtr postTransformClassDef(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformMethodDef(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformMethodDef(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformIf(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformIf(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformWhile(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformWhile(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr postTransformBreak(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr postTransformRetry(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr postTransformNext(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformReturn(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformReturn(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformRescueCase(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformRescueCase(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformRescue(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformRescue(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr postTransformUnresolvedIdent(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformAssign(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformAssign(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformSend(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformSend(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformHash(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformHash(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformArray(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postransformArray(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr postTransformConstantLit(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr postTransformUnresolvedConstantLit(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformBlock(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformBlock(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformInsSeq(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformInsSeq(core::MutableContext ctx, ExpressionPtr original);
};

// NOTE: Implementations must use a context type parameter that `MutableContext` is convertable to.
// That is, either `Context` or `MutableContext`.
#define GENERATE_HAS_MEMBER_VISITOR(X) \
    GENERATE_HAS_MEMBER(X, std::declval<core::MutableContext>(), std::declval<ExpressionPtr>())

// used to check for ABSENCE of method
GENERATE_HAS_MEMBER_VISITOR(preTransformUnresolvedIdent);
GENERATE_HAS_MEMBER_VISITOR(preTransformLocal);
GENERATE_HAS_MEMBER_VISITOR(preTransformUnresolvedConstantLit);
GENERATE_HAS_MEMBER_VISITOR(preTransformConstantLit);
GENERATE_HAS_MEMBER_VISITOR(preTransformLiteral);

#define GENERATE_POSTPONE_PRECLASS(X)                                                                            \
    GENERATE_CALL_MEMBER(preTransform##X, Exception::raise("should never be called. Incorrect use of TreeMap?"); \
                         return nullptr, std::declval<core::MutableContext>(), std::declval<ExpressionPtr>())

#define GENERATE_POSTPONE_POSTCLASS(X)                                                                            \
    GENERATE_CALL_MEMBER(postTransform##X, Exception::raise("should never be called. Incorrect use of TreeMap?"); \
                         return nullptr, std::declval<core::MutableContext>(), std::declval<ExpressionPtr>())

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

    ExpressionPtr mapClassDef(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformClassDef<FUNC>()) {
            v = CALL_MEMBER_preTransformClassDef<FUNC>::call(func, ctx, std::move(v));
        }

        // We intentionally do not walk v->ancestors nor v->singletonAncestors.
        // They are guaranteed to be simple trees in the desugarer.
        for (auto &def : cast_tree_nonnull<ClassDef>(v).rhs) {
            def = mapIt(std::move(def), ctx.withOwner(cast_tree_nonnull<ClassDef>(v).symbol).withFile(ctx.file));
        }

        if constexpr (HAS_MEMBER_postTransformClassDef<FUNC>()) {
            return CALL_MEMBER_postTransformClassDef<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapMethodDef(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformMethodDef<FUNC>()) {
            v = CALL_MEMBER_preTransformMethodDef<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &arg : cast_tree_nonnull<MethodDef>(v).args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ =
                    mapIt(std::move(optArg->default_), ctx.withOwner(cast_tree_nonnull<MethodDef>(v).symbol));
            }
        }
        cast_tree_nonnull<MethodDef>(v).rhs =
            mapIt(std::move(cast_tree_nonnull<MethodDef>(v).rhs),
                  ctx.withOwner(cast_tree_nonnull<MethodDef>(v).symbol).withFile(ctx.file));

        if constexpr (HAS_MEMBER_postTransformMethodDef<FUNC>()) {
            return CALL_MEMBER_postTransformMethodDef<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapIf(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformIf<FUNC>()) {
            v = CALL_MEMBER_preTransformIf<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree_nonnull<If>(v).cond = mapIt(std::move(cast_tree_nonnull<If>(v).cond), ctx);
        cast_tree_nonnull<If>(v).thenp = mapIt(std::move(cast_tree_nonnull<If>(v).thenp), ctx);
        cast_tree_nonnull<If>(v).elsep = mapIt(std::move(cast_tree_nonnull<If>(v).elsep), ctx);

        if constexpr (HAS_MEMBER_postTransformIf<FUNC>()) {
            return CALL_MEMBER_postTransformIf<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapWhile(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformWhile<FUNC>()) {
            v = CALL_MEMBER_preTransformWhile<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree_nonnull<While>(v).cond = mapIt(std::move(cast_tree_nonnull<While>(v).cond), ctx);
        cast_tree_nonnull<While>(v).body = mapIt(std::move(cast_tree_nonnull<While>(v).body), ctx);

        if constexpr (HAS_MEMBER_postTransformWhile<FUNC>()) {
            return CALL_MEMBER_postTransformWhile<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapBreak(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBreak<FUNC>()) {
            return CALL_MEMBER_preTransformBreak<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree_nonnull<Break>(v).expr = mapIt(std::move(cast_tree_nonnull<Break>(v).expr), ctx);

        if constexpr (HAS_MEMBER_postTransformBreak<FUNC>()) {
            return CALL_MEMBER_postTransformBreak<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }
    ExpressionPtr mapRetry(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformRetry<FUNC>()) {
            return CALL_MEMBER_postTransformRetry<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapNext(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformNext<FUNC>()) {
            return CALL_MEMBER_preTransformNext<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree_nonnull<Next>(v).expr = mapIt(std::move(cast_tree_nonnull<Next>(v).expr), ctx);

        if constexpr (HAS_MEMBER_postTransformNext<FUNC>()) {
            return CALL_MEMBER_postTransformNext<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapReturn(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformReturn<FUNC>()) {
            v = CALL_MEMBER_preTransformReturn<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree_nonnull<Return>(v).expr = mapIt(std::move(cast_tree_nonnull<Return>(v).expr), ctx);

        if constexpr (HAS_MEMBER_postTransformReturn<FUNC>()) {
            return CALL_MEMBER_postTransformReturn<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapRescueCase(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescueCase<FUNC>()) {
            v = CALL_MEMBER_preTransformRescueCase<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &el : cast_tree_nonnull<RescueCase>(v).exceptions) {
            el = mapIt(std::move(el), ctx);
        }

        cast_tree_nonnull<RescueCase>(v).var = mapIt(std::move(cast_tree_nonnull<RescueCase>(v).var), ctx);

        cast_tree_nonnull<RescueCase>(v).body = mapIt(std::move(cast_tree_nonnull<RescueCase>(v).body), ctx);

        if constexpr (HAS_MEMBER_postTransformRescueCase<FUNC>()) {
            return CALL_MEMBER_postTransformRescueCase<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }
    ExpressionPtr mapRescue(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescue<FUNC>()) {
            v = CALL_MEMBER_preTransformRescue<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree_nonnull<Rescue>(v).body = mapIt(std::move(cast_tree_nonnull<Rescue>(v).body), ctx);

        for (auto &el : cast_tree_nonnull<Rescue>(v).rescueCases) {
            ENFORCE(isa_tree<RescueCase>(el), "invalid tree where rescue case was expected");
            el = mapRescueCase(std::move(el), ctx);
            ENFORCE(isa_tree<RescueCase>(el), "rescue case was mapped into non-rescue case");
        }

        cast_tree_nonnull<Rescue>(v).else_ = mapIt(std::move(cast_tree_nonnull<Rescue>(v).else_), ctx);
        cast_tree_nonnull<Rescue>(v).ensure = mapIt(std::move(cast_tree_nonnull<Rescue>(v).ensure), ctx);

        if constexpr (HAS_MEMBER_postTransformRescue<FUNC>()) {
            return CALL_MEMBER_postTransformRescue<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapUnresolvedIdent(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedIdent<FUNC>()) {
            return CALL_MEMBER_postTransformUnresolvedIdent<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapAssign(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformAssign<FUNC>()) {
            v = CALL_MEMBER_preTransformAssign<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree_nonnull<Assign>(v).lhs = mapIt(std::move(cast_tree_nonnull<Assign>(v).lhs), ctx);
        cast_tree_nonnull<Assign>(v).rhs = mapIt(std::move(cast_tree_nonnull<Assign>(v).rhs), ctx);

        if constexpr (HAS_MEMBER_postTransformAssign<FUNC>()) {
            return CALL_MEMBER_postTransformAssign<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapSend(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformSend<FUNC>()) {
            v = CALL_MEMBER_preTransformSend<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree_nonnull<Send>(v).recv = mapIt(std::move(cast_tree_nonnull<Send>(v).recv), ctx);

        const auto numPosArgs = cast_tree_nonnull<Send>(v).numPosArgs();
        for (auto i = 0; i < numPosArgs; ++i) {
            auto &arg = cast_tree_nonnull<Send>(v).getPosArg(i);
            arg = mapIt(std::move(arg), ctx);
            ENFORCE(arg != nullptr);
        }

        const auto numKwArgs = cast_tree_nonnull<Send>(v).numKwArgs();
        for (auto i = 0; i < numKwArgs; ++i) {
            auto &key = cast_tree_nonnull<Send>(v).getKwKey(i);
            key = mapIt(std::move(key), ctx);
            ENFORCE(key != nullptr);

            auto &val = cast_tree_nonnull<Send>(v).getKwValue(i);
            val = mapIt(std::move(val), ctx);
            ENFORCE(val != nullptr);
        }

        if (auto kwSplat = cast_tree_nonnull<Send>(v).kwSplat()) {
            *kwSplat = mapIt(std::move(*kwSplat), ctx);
            ENFORCE(kwSplat != nullptr);
        }

        if (auto block = cast_tree_nonnull<Send>(v).rawBlock()) {
            *block = mapIt(std::move(*block), ctx);
            ENFORCE(cast_tree_nonnull<Send>(v).block() != nullptr, "block was mapped into not-a block");
        }

        if constexpr (HAS_MEMBER_postTransformSend<FUNC>()) {
            return CALL_MEMBER_postTransformSend<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapHash(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformHash<FUNC>()) {
            v = CALL_MEMBER_preTransformHash<FUNC>::call(func, ctx, std::move(v));
        }
        for (auto &key : cast_tree_nonnull<Hash>(v).keys) {
            key = mapIt(std::move(key), ctx);
        }

        for (auto &value : cast_tree_nonnull<Hash>(v).values) {
            value = mapIt(std::move(value), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>()) {
            return CALL_MEMBER_postTransformHash<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapArray(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformArray<FUNC>()) {
            v = CALL_MEMBER_preTransformArray<FUNC>::call(func, ctx, std::move(v));
        }
        for (auto &elem : cast_tree_nonnull<Array>(v).elems) {
            elem = mapIt(std::move(elem), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>()) {
            return CALL_MEMBER_postTransformArray<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapLiteral(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLiteral<FUNC>()) {
            return CALL_MEMBER_postTransformLiteral<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapUnresolvedConstantLit(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedConstantLit<FUNC>()) {
            return CALL_MEMBER_postTransformUnresolvedConstantLit<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapConstantLit(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformConstantLit<FUNC>()) {
            return CALL_MEMBER_postTransformConstantLit<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapBlock(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBlock<FUNC>()) {
            v = CALL_MEMBER_preTransformBlock<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &arg : cast_tree_nonnull<Block>(v).args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ = mapIt(std::move(optArg->default_), ctx);
            }
        }
        cast_tree_nonnull<Block>(v).body = mapIt(std::move(cast_tree_nonnull<Block>(v).body), ctx);

        if constexpr (HAS_MEMBER_postTransformBlock<FUNC>()) {
            return CALL_MEMBER_postTransformBlock<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapInsSeq(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformInsSeq<FUNC>()) {
            v = CALL_MEMBER_preTransformInsSeq<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &stat : cast_tree_nonnull<InsSeq>(v).stats) {
            stat = mapIt(std::move(stat), ctx);
        }

        cast_tree_nonnull<InsSeq>(v).expr = mapIt(std::move(cast_tree_nonnull<InsSeq>(v).expr), ctx);

        if constexpr (HAS_MEMBER_postTransformInsSeq<FUNC>()) {
            return CALL_MEMBER_postTransformInsSeq<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapLocal(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLocal<FUNC>()) {
            return CALL_MEMBER_postTransformLocal<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapCast(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformCast<FUNC>()) {
            v = CALL_MEMBER_preTransformCast<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree_nonnull<Cast>(v).arg = mapIt(std::move(cast_tree_nonnull<Cast>(v).arg), ctx);

        if constexpr (HAS_MEMBER_postTransformCast<FUNC>()) {
            return CALL_MEMBER_postTransformCast<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapIt(ExpressionPtr what, CTX ctx) {
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
                    return mapBlock(std::move(what), ctx);

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
    template <typename CTX, typename FUNC> static ExpressionPtr apply(CTX ctx, FUNC &func, ExpressionPtr to) {
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

    ExpressionPtr mapClassDef(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformClassDef<FUNC>()) {
            v = CALL_MEMBER_preTransformClassDef<FUNC>::call(func, ctx, std::move(v));
        }

        // We intentionally do not walk v->ancestors nor v->singletonAncestors.
        // They are guaranteed to be simple trees in the desugarer.
        for (auto &def : cast_tree_nonnull<ClassDef>(v).rhs) {
            def = mapIt(std::move(def), ctx.withOwner(cast_tree_nonnull<ClassDef>(v).symbol));
        }

        if constexpr (HAS_MEMBER_postTransformClassDef<FUNC>()) {
            return CALL_MEMBER_postTransformClassDef<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapMethodDef(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformMethodDef<FUNC>()) {
            v = CALL_MEMBER_preTransformMethodDef<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &arg : cast_tree_nonnull<MethodDef>(v).args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ =
                    mapIt(std::move(optArg->default_), ctx.withOwner(cast_tree_nonnull<MethodDef>(v).symbol));
            }
        }
        // because this is a ShallowMap, we do not map over the body of the method

        if constexpr (HAS_MEMBER_postTransformMethodDef<FUNC>()) {
            return CALL_MEMBER_postTransformMethodDef<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapIf(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformIf<FUNC>()) {
            v = CALL_MEMBER_preTransformIf<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree_nonnull<If>(v).cond = mapIt(std::move(cast_tree_nonnull<If>(v).cond), ctx);
        cast_tree_nonnull<If>(v).thenp = mapIt(std::move(cast_tree_nonnull<If>(v).thenp), ctx);
        cast_tree_nonnull<If>(v).elsep = mapIt(std::move(cast_tree_nonnull<If>(v).elsep), ctx);

        if constexpr (HAS_MEMBER_postTransformIf<FUNC>()) {
            return CALL_MEMBER_postTransformIf<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapWhile(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformWhile<FUNC>()) {
            v = CALL_MEMBER_preTransformWhile<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree_nonnull<While>(v).cond = mapIt(std::move(cast_tree_nonnull<While>(v).cond), ctx);
        cast_tree_nonnull<While>(v).body = mapIt(std::move(cast_tree_nonnull<While>(v).body), ctx);

        if constexpr (HAS_MEMBER_postTransformWhile<FUNC>()) {
            return CALL_MEMBER_postTransformWhile<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapBreak(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBreak<FUNC>()) {
            return CALL_MEMBER_preTransformBreak<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree_nonnull<Break>(v).expr = mapIt(std::move(cast_tree_nonnull<Break>(v).expr), ctx);

        if constexpr (HAS_MEMBER_postTransformBreak<FUNC>()) {
            return CALL_MEMBER_postTransformBreak<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }
    ExpressionPtr mapRetry(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformRetry<FUNC>()) {
            return CALL_MEMBER_postTransformRetry<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapNext(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformNext<FUNC>()) {
            return CALL_MEMBER_preTransformNext<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree_nonnull<Next>(v).expr = mapIt(std::move(cast_tree_nonnull<Next>(v).expr), ctx);

        if constexpr (HAS_MEMBER_postTransformNext<FUNC>()) {
            return CALL_MEMBER_postTransformNext<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapReturn(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformReturn<FUNC>()) {
            v = CALL_MEMBER_preTransformReturn<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree_nonnull<Return>(v).expr = mapIt(std::move(cast_tree_nonnull<Return>(v).expr), ctx);

        if constexpr (HAS_MEMBER_postTransformReturn<FUNC>()) {
            return CALL_MEMBER_postTransformReturn<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapRescueCase(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescueCase<FUNC>()) {
            v = CALL_MEMBER_preTransformRescueCase<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &el : cast_tree_nonnull<RescueCase>(v).exceptions) {
            el = mapIt(std::move(el), ctx);
        }

        cast_tree_nonnull<RescueCase>(v).var = mapIt(std::move(cast_tree_nonnull<RescueCase>(v).var), ctx);

        cast_tree_nonnull<RescueCase>(v).body = mapIt(std::move(cast_tree_nonnull<RescueCase>(v).body), ctx);

        if constexpr (HAS_MEMBER_postTransformRescueCase<FUNC>()) {
            return CALL_MEMBER_postTransformRescueCase<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }
    ExpressionPtr mapRescue(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescue<FUNC>()) {
            v = CALL_MEMBER_preTransformRescue<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree_nonnull<Rescue>(v).body = mapIt(std::move(cast_tree_nonnull<Rescue>(v).body), ctx);

        for (auto &el : cast_tree_nonnull<Rescue>(v).rescueCases) {
            ENFORCE(isa_tree<RescueCase>(el), "invalid tree where rescue case was expected");
            el = mapRescueCase(std::move(el), ctx);
            ENFORCE(isa_tree<RescueCase>(el), "rescue case was mapped into non-rescue case");
        }

        cast_tree_nonnull<Rescue>(v).else_ = mapIt(std::move(cast_tree_nonnull<Rescue>(v).else_), ctx);
        cast_tree_nonnull<Rescue>(v).ensure = mapIt(std::move(cast_tree_nonnull<Rescue>(v).ensure), ctx);

        if constexpr (HAS_MEMBER_postTransformRescue<FUNC>()) {
            return CALL_MEMBER_postTransformRescue<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapUnresolvedIdent(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedIdent<FUNC>()) {
            return CALL_MEMBER_postTransformUnresolvedIdent<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapAssign(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformAssign<FUNC>()) {
            v = CALL_MEMBER_preTransformAssign<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree_nonnull<Assign>(v).lhs = mapIt(std::move(cast_tree_nonnull<Assign>(v).lhs), ctx);
        cast_tree_nonnull<Assign>(v).rhs = mapIt(std::move(cast_tree_nonnull<Assign>(v).rhs), ctx);

        if constexpr (HAS_MEMBER_postTransformAssign<FUNC>()) {
            return CALL_MEMBER_postTransformAssign<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapSend(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformSend<FUNC>()) {
            v = CALL_MEMBER_preTransformSend<FUNC>::call(func, ctx, std::move(v));
        }

        cast_tree_nonnull<Send>(v).recv = mapIt(std::move(cast_tree_nonnull<Send>(v).recv), ctx);

        const auto numPosArgs = cast_tree_nonnull<Send>(v).numPosArgs();
        for (auto i = 0; i < numPosArgs; ++i) {
            auto &arg = cast_tree_nonnull<Send>(v).getPosArg(i);
            arg = mapIt(std::move(arg), ctx);
            ENFORCE(arg != nullptr);
        }

        const auto numKwArgs = cast_tree_nonnull<Send>(v).numKwArgs();
        for (auto i = 0; i < numKwArgs; ++i) {
            auto &key = cast_tree_nonnull<Send>(v).getKwKey(i);
            key = mapIt(std::move(key), ctx);
            ENFORCE(key != nullptr);

            auto &val = cast_tree_nonnull<Send>(v).getKwValue(i);
            val = mapIt(std::move(val), ctx);
            ENFORCE(val != nullptr);
        }

        if (auto kwSplat = cast_tree_nonnull<Send>(v).kwSplat()) {
            *kwSplat = mapIt(std::move(*kwSplat), ctx);
            ENFORCE(kwSplat != nullptr);
        }

        if (auto block = cast_tree_nonnull<Send>(v).rawBlock()) {
            *block = mapIt(std::move(*block), ctx);
            ENFORCE(cast_tree_nonnull<Send>(v).block() != nullptr, "block was mapped into not-a block");
        }

        if constexpr (HAS_MEMBER_postTransformSend<FUNC>()) {
            return CALL_MEMBER_postTransformSend<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapHash(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformHash<FUNC>()) {
            v = CALL_MEMBER_preTransformHash<FUNC>::call(func, ctx, std::move(v));
        }
        for (auto &key : cast_tree_nonnull<Hash>(v).keys) {
            key = mapIt(std::move(key), ctx);
        }

        for (auto &value : cast_tree_nonnull<Hash>(v).values) {
            value = mapIt(std::move(value), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>()) {
            return CALL_MEMBER_postTransformHash<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapArray(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformArray<FUNC>()) {
            v = CALL_MEMBER_preTransformArray<FUNC>::call(func, ctx, std::move(v));
        }
        for (auto &elem : cast_tree_nonnull<Array>(v).elems) {
            elem = mapIt(std::move(elem), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>()) {
            return CALL_MEMBER_postTransformArray<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapLiteral(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLiteral<FUNC>()) {
            return CALL_MEMBER_postTransformLiteral<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapUnresolvedConstantLit(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedConstantLit<FUNC>()) {
            return CALL_MEMBER_postTransformUnresolvedConstantLit<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapConstantLit(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformConstantLit<FUNC>()) {
            return CALL_MEMBER_postTransformConstantLit<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapBlock(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBlock<FUNC>()) {
            v = CALL_MEMBER_preTransformBlock<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &arg : cast_tree_nonnull<Block>(v).args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ = mapIt(std::move(optArg->default_), ctx);
            }
        }
        cast_tree_nonnull<Block>(v).body = mapIt(std::move(cast_tree_nonnull<Block>(v).body), ctx);

        if constexpr (HAS_MEMBER_postTransformBlock<FUNC>()) {
            return CALL_MEMBER_postTransformBlock<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapInsSeq(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformInsSeq<FUNC>()) {
            v = CALL_MEMBER_preTransformInsSeq<FUNC>::call(func, ctx, std::move(v));
        }

        for (auto &stat : cast_tree_nonnull<InsSeq>(v).stats) {
            stat = mapIt(std::move(stat), ctx);
        }

        cast_tree_nonnull<InsSeq>(v).expr = mapIt(std::move(cast_tree_nonnull<InsSeq>(v).expr), ctx);

        if constexpr (HAS_MEMBER_postTransformInsSeq<FUNC>()) {
            return CALL_MEMBER_postTransformInsSeq<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapLocal(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLocal<FUNC>()) {
            return CALL_MEMBER_postTransformLocal<FUNC>::call(func, ctx, std::move(v));
        }
        return v;
    }

    ExpressionPtr mapCast(ExpressionPtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformCast<FUNC>()) {
            v = CALL_MEMBER_preTransformCast<FUNC>::call(func, ctx, std::move(v));
        }
        cast_tree_nonnull<Cast>(v).arg = mapIt(std::move(cast_tree_nonnull<Cast>(v).arg), ctx);

        if constexpr (HAS_MEMBER_postTransformCast<FUNC>()) {
            return CALL_MEMBER_postTransformCast<FUNC>::call(func, ctx, std::move(v));
        }

        return v;
    }

    ExpressionPtr mapIt(ExpressionPtr what, CTX ctx) {
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
                    return mapBlock(std::move(what), ctx);

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
    template <typename CTX, typename FUNC> static ExpressionPtr apply(CTX ctx, FUNC &func, ExpressionPtr to) {
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
