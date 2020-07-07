#ifndef SORBET_TREEMAP_H
#define SORBET_TREEMAP_H

#include "ast/Trees.h"
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

/**
 * GENERATE_HAS_MEMBER(att)  // Creates 'has_member_att'.
 * `HAS_MEMBER_att<C>::value` can be used to statically test if C has a member att
 */
#define GENERATE_HAS_MEMBER(X)                                                          \
    template <typename T> class HAS_MEMBER_##X {                                        \
        struct Fallback {                                                               \
            int X;                                                                      \
        };                                                                              \
        struct Derived : T, Fallback {};                                                \
                                                                                        \
        template <typename U, U> struct Check;                                          \
                                                                                        \
        using ArrayOfOne = char[1];                                                     \
        using ArrayOfTwo = char[2];                                                     \
                                                                                        \
        template <typename U> static ArrayOfOne &func(Check<int Fallback::*, &U::X> *); \
        template <typename U> static ArrayOfTwo &func(...);                             \
                                                                                        \
    public:                                                                             \
        using type = HAS_MEMBER_##X;                                                    \
        static constexpr std::size_t value = sizeof(func<Derived>(0)) == 2;             \
    };

GENERATE_HAS_MEMBER(preTransformExpression);
GENERATE_HAS_MEMBER(preTransformClassDef);
GENERATE_HAS_MEMBER(preTransformMethodDef);
GENERATE_HAS_MEMBER(preTransformIf);
GENERATE_HAS_MEMBER(preTransformWhile);
GENERATE_HAS_MEMBER(preTransformBreak);
GENERATE_HAS_MEMBER(preTransformRetry);
GENERATE_HAS_MEMBER(preTransformNext);
GENERATE_HAS_MEMBER(preTransformReturn);
GENERATE_HAS_MEMBER(preTransformRescueCase);
GENERATE_HAS_MEMBER(preTransformRescue);
GENERATE_HAS_MEMBER(preTransformAssign);
GENERATE_HAS_MEMBER(preTransformSend);
GENERATE_HAS_MEMBER(preTransformHash);
GENERATE_HAS_MEMBER(preTransformArray);
GENERATE_HAS_MEMBER(preTransformBlock);
GENERATE_HAS_MEMBER(preTransformInsSeq);

// used to check for ABSENCE of method
GENERATE_HAS_MEMBER(preTransformUnresolvedIdent);
GENERATE_HAS_MEMBER(preTransformLocal);
GENERATE_HAS_MEMBER(preTransformUnresolvedConstantLit);
GENERATE_HAS_MEMBER(preTransformConstantLit);
GENERATE_HAS_MEMBER(preTransformLiteral);
GENERATE_HAS_MEMBER(preTransformCast);

GENERATE_HAS_MEMBER(postTransformClassDef);
GENERATE_HAS_MEMBER(postTransformMethodDef);
GENERATE_HAS_MEMBER(postTransformIf);
GENERATE_HAS_MEMBER(postTransformWhile);
GENERATE_HAS_MEMBER(postTransformBreak);
GENERATE_HAS_MEMBER(postTransformRetry);
GENERATE_HAS_MEMBER(postTransformNext);
GENERATE_HAS_MEMBER(postTransformReturn);
GENERATE_HAS_MEMBER(postTransformRescueCase);
GENERATE_HAS_MEMBER(postTransformRescue);
GENERATE_HAS_MEMBER(postTransformUnresolvedIdent);
GENERATE_HAS_MEMBER(postTransformAssign);
GENERATE_HAS_MEMBER(postTransformSend);
GENERATE_HAS_MEMBER(postTransformHash);
GENERATE_HAS_MEMBER(postTransformLocal);
GENERATE_HAS_MEMBER(postTransformArray);
GENERATE_HAS_MEMBER(postTransformLiteral);
GENERATE_HAS_MEMBER(postTransformUnresolvedConstantLit);
GENERATE_HAS_MEMBER(postTransformConstantLit);
GENERATE_HAS_MEMBER(postTransformArraySplat);
GENERATE_HAS_MEMBER(postTransformHashSplat);
GENERATE_HAS_MEMBER(postTransformBlock);
GENERATE_HAS_MEMBER(postTransformInsSeq);
GENERATE_HAS_MEMBER(postTransformCast);

#define GENERATE_POSTPONE_PRECLASS(X)                                                   \
                                                                                        \
    template <class FUNC, class CTX, bool has> class PostPonePreTransform_##X {         \
    public:                                                                             \
        static TreePtr call(CTX ctx, TreePtr cd, FUNC &what) {                          \
            Exception::raise("should never be called. Incorrect use of TreeMap?");      \
            return nullptr;                                                             \
        }                                                                               \
    };                                                                                  \
                                                                                        \
    template <class FUNC, class CTX> class PostPonePreTransform_##X<FUNC, CTX, true> {  \
    public:                                                                             \
        static TreePtr call(CTX ctx, TreePtr cd, FUNC &func) {                          \
            return func.preTransform##X(ctx, std::move(cd));                            \
        }                                                                               \
    };                                                                                  \
                                                                                        \
    template <class FUNC, class CTX> class PostPonePreTransform_##X<FUNC, CTX, false> { \
    public:                                                                             \
        static TreePtr call(CTX ctx, TreePtr cd, FUNC &func) {                          \
            return cd;                                                                  \
        }                                                                               \
    };

#define GENERATE_POSTPONE_POSTCLASS(X)                                                   \
                                                                                         \
    template <class FUNC, class CTX, bool has> class PostPonePostTransform_##X {         \
    public:                                                                              \
        static TreePtr call(CTX ctx, TreePtr cd, FUNC &what) {                           \
            Exception::raise("should never be called. Incorrect use of TreeMap?");       \
            return nullptr;                                                              \
        }                                                                                \
    };                                                                                   \
                                                                                         \
    template <class FUNC, class CTX> class PostPonePostTransform_##X<FUNC, CTX, true> {  \
    public:                                                                              \
        static TreePtr call(CTX ctx, TreePtr cd, FUNC &func) {                           \
            return func.postTransform##X(ctx, std::move(cd));                            \
        }                                                                                \
    };                                                                                   \
                                                                                         \
    template <class FUNC, class CTX> class PostPonePostTransform_##X<FUNC, CTX, false> { \
    public:                                                                              \
        static TreePtr call(CTX ctx, TreePtr cd, FUNC &func) {                           \
            return cd;                                                                   \
        }                                                                                \
    };

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

    static_assert(!HAS_MEMBER_preTransformUnresolvedIdent<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformLiteral<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformUnresolvedConstantLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformConstantLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformLocal<FUNC>::value, "use post*Transform instead");

    TreeMapper(FUNC &func) : func(func) {}

    TreePtr mapClassDef(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformClassDef<FUNC>::value) {
            v = PostPonePreTransform_ClassDef<FUNC, CTX, HAS_MEMBER_preTransformClassDef<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        // We intentionally do not walk v->ancestors nor v->singletonAncestors.
        // They are guaranteed to be simple trees in the desugarer.
        for (auto &def : cast_tree<ClassDef>(v)->rhs) {
            def = mapIt(std::move(def),
                        ctx.withOwner(cast_tree<ClassDef>(v)->symbol).withFile(cast_tree<ClassDef>(v)->declLoc.file()));
        }

        if constexpr (HAS_MEMBER_postTransformClassDef<FUNC>::value) {
            return PostPonePostTransform_ClassDef<FUNC, CTX, HAS_MEMBER_postTransformClassDef<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapMethodDef(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformMethodDef<FUNC>::value) {
            v = PostPonePreTransform_MethodDef<FUNC, CTX, HAS_MEMBER_preTransformMethodDef<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        for (auto &arg : cast_tree<MethodDef>(v)->args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ = mapIt(std::move(optArg->default_), ctx.withOwner(cast_tree<MethodDef>(v)->symbol));
            }
        }
        cast_tree<MethodDef>(v)->rhs =
            mapIt(std::move(cast_tree<MethodDef>(v)->rhs),
                  ctx.withOwner(cast_tree<MethodDef>(v)->symbol).withFile(cast_tree<MethodDef>(v)->declLoc.file()));

        if constexpr (HAS_MEMBER_postTransformMethodDef<FUNC>::value) {
            return PostPonePostTransform_MethodDef<FUNC, CTX, HAS_MEMBER_postTransformMethodDef<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapIf(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformIf<FUNC>::value) {
            v = PostPonePreTransform_If<FUNC, CTX, HAS_MEMBER_preTransformIf<FUNC>::value>::call(ctx, std::move(v),
                                                                                                 func);
        }
        cast_tree<If>(v)->cond = mapIt(std::move(cast_tree<If>(v)->cond), ctx);
        cast_tree<If>(v)->thenp = mapIt(std::move(cast_tree<If>(v)->thenp), ctx);
        cast_tree<If>(v)->elsep = mapIt(std::move(cast_tree<If>(v)->elsep), ctx);

        if constexpr (HAS_MEMBER_postTransformIf<FUNC>::value) {
            return PostPonePostTransform_If<FUNC, CTX, HAS_MEMBER_postTransformIf<FUNC>::value>::call(ctx, std::move(v),
                                                                                                      func);
        }
        return v;
    }

    TreePtr mapWhile(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformWhile<FUNC>::value) {
            v = PostPonePreTransform_While<FUNC, CTX, HAS_MEMBER_preTransformWhile<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        cast_tree<While>(v)->cond = mapIt(std::move(cast_tree<While>(v)->cond), ctx);
        cast_tree<While>(v)->body = mapIt(std::move(cast_tree<While>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformWhile<FUNC>::value) {
            return PostPonePostTransform_While<FUNC, CTX, HAS_MEMBER_postTransformWhile<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapBreak(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBreak<FUNC>::value) {
            return PostPonePreTransform_Break<FUNC, CTX, HAS_MEMBER_preTransformBreak<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        cast_tree<Break>(v)->expr = mapIt(std::move(cast_tree<Break>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformBreak<FUNC>::value) {
            return PostPonePostTransform_Break<FUNC, CTX, HAS_MEMBER_postTransformBreak<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }
    TreePtr mapRetry(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformRetry<FUNC>::value) {
            return PostPonePostTransform_Retry<FUNC, CTX, HAS_MEMBER_postTransformRetry<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapNext(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformNext<FUNC>::value) {
            return PostPonePreTransform_Next<FUNC, CTX, HAS_MEMBER_preTransformNext<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        cast_tree<Next>(v)->expr = mapIt(std::move(cast_tree<Next>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformNext<FUNC>::value) {
            return PostPonePostTransform_Next<FUNC, CTX, HAS_MEMBER_postTransformNext<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapReturn(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformReturn<FUNC>::value) {
            v = PostPonePreTransform_Return<FUNC, CTX, HAS_MEMBER_preTransformReturn<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        cast_tree<Return>(v)->expr = mapIt(std::move(cast_tree<Return>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformReturn<FUNC>::value) {
            return PostPonePostTransform_Return<FUNC, CTX, HAS_MEMBER_postTransformReturn<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapRescueCase(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescueCase<FUNC>::value) {
            v = PostPonePreTransform_RescueCase<FUNC, CTX, HAS_MEMBER_preTransformRescueCase<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        for (auto &el : cast_tree<RescueCase>(v)->exceptions) {
            el = mapIt(std::move(el), ctx);
        }

        cast_tree<RescueCase>(v)->var = mapIt(std::move(cast_tree<RescueCase>(v)->var), ctx);

        cast_tree<RescueCase>(v)->body = mapIt(std::move(cast_tree<RescueCase>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformRescueCase<FUNC>::value) {
            return PostPonePostTransform_RescueCase<FUNC, CTX, HAS_MEMBER_postTransformRescueCase<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }
    TreePtr mapRescue(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescue<FUNC>::value) {
            v = PostPonePreTransform_Rescue<FUNC, CTX, HAS_MEMBER_preTransformRescue<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        cast_tree<Rescue>(v)->body = mapIt(std::move(cast_tree<Rescue>(v)->body), ctx);

        for (auto &el : cast_tree<Rescue>(v)->rescueCases) {
            ENFORCE(isa_tree<RescueCase>(el), "invalid tree where rescue case was expected");
            el = mapRescueCase(std::move(el), ctx);
            ENFORCE(isa_tree<RescueCase>(el), "rescue case was mapped into non-rescue case");
        }

        cast_tree<Rescue>(v)->else_ = mapIt(std::move(cast_tree<Rescue>(v)->else_), ctx);
        cast_tree<Rescue>(v)->ensure = mapIt(std::move(cast_tree<Rescue>(v)->ensure), ctx);

        if constexpr (HAS_MEMBER_postTransformRescue<FUNC>::value) {
            return PostPonePostTransform_Rescue<FUNC, CTX, HAS_MEMBER_postTransformRescue<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapUnresolvedIdent(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedIdent<FUNC>::value) {
            return PostPonePostTransform_UnresolvedIdent<
                FUNC, CTX, HAS_MEMBER_postTransformUnresolvedIdent<FUNC>::value>::call(ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapAssign(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformAssign<FUNC>::value) {
            v = PostPonePreTransform_Assign<FUNC, CTX, HAS_MEMBER_preTransformAssign<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        cast_tree<Assign>(v)->lhs = mapIt(std::move(cast_tree<Assign>(v)->lhs), ctx);
        cast_tree<Assign>(v)->rhs = mapIt(std::move(cast_tree<Assign>(v)->rhs), ctx);

        if constexpr (HAS_MEMBER_postTransformAssign<FUNC>::value) {
            return PostPonePostTransform_Assign<FUNC, CTX, HAS_MEMBER_postTransformAssign<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapSend(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformSend<FUNC>::value) {
            v = PostPonePreTransform_Send<FUNC, CTX, HAS_MEMBER_preTransformSend<FUNC>::value>::call(ctx, std::move(v),
                                                                                                     func);
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

        if constexpr (HAS_MEMBER_postTransformSend<FUNC>::value) {
            return PostPonePostTransform_Send<FUNC, CTX, HAS_MEMBER_postTransformSend<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapHash(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformHash<FUNC>::value) {
            v = PostPonePreTransform_Hash<FUNC, CTX, HAS_MEMBER_preTransformHash<FUNC>::value>::call(ctx, std::move(v),
                                                                                                     func);
        }
        for (auto &key : cast_tree<Hash>(v)->keys) {
            key = mapIt(std::move(key), ctx);
        }

        for (auto &value : cast_tree<Hash>(v)->values) {
            value = mapIt(std::move(value), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>::value) {
            return PostPonePostTransform_Hash<FUNC, CTX, HAS_MEMBER_postTransformHash<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapArray(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformArray<FUNC>::value) {
            v = PostPonePreTransform_Array<FUNC, CTX, HAS_MEMBER_preTransformArray<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        for (auto &elem : cast_tree<Array>(v)->elems) {
            elem = mapIt(std::move(elem), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>::value) {
            return PostPonePostTransform_Array<FUNC, CTX, HAS_MEMBER_postTransformArray<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapLiteral(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLiteral<FUNC>::value) {
            return PostPonePostTransform_Literal<FUNC, CTX, HAS_MEMBER_postTransformLiteral<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapUnresolvedConstantLit(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedConstantLit<FUNC>::value) {
            return PostPonePostTransform_UnresolvedConstantLit<
                FUNC, CTX, HAS_MEMBER_postTransformUnresolvedConstantLit<FUNC>::value>::call(ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapConstantLit(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformConstantLit<FUNC>::value) {
            return PostPonePostTransform_ConstantLit<FUNC, CTX, HAS_MEMBER_postTransformConstantLit<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapBlock(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBlock<FUNC>::value) {
            v = PostPonePreTransform_Block<FUNC, CTX, HAS_MEMBER_preTransformBlock<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        for (auto &arg : cast_tree<Block>(v)->args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ = mapIt(std::move(optArg->default_), ctx);
            }
        }
        cast_tree<Block>(v)->body = mapIt(std::move(cast_tree<Block>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformBlock<FUNC>::value) {
            return PostPonePostTransform_Block<FUNC, CTX, HAS_MEMBER_postTransformBlock<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapInsSeq(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformInsSeq<FUNC>::value) {
            v = PostPonePreTransform_InsSeq<FUNC, CTX, HAS_MEMBER_preTransformInsSeq<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        for (auto &stat : cast_tree<InsSeq>(v)->stats) {
            stat = mapIt(std::move(stat), ctx);
        }

        cast_tree<InsSeq>(v)->expr = mapIt(std::move(cast_tree<InsSeq>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformInsSeq<FUNC>::value) {
            return PostPonePostTransform_InsSeq<FUNC, CTX, HAS_MEMBER_postTransformInsSeq<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapLocal(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLocal<FUNC>::value) {
            return PostPonePostTransform_Local<FUNC, CTX, HAS_MEMBER_postTransformLocal<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapCast(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformCast<FUNC>::value) {
            v = PostPonePreTransform_Cast<FUNC, CTX, HAS_MEMBER_preTransformCast<FUNC>::value>::call(ctx, std::move(v),
                                                                                                     func);
        }
        cast_tree<Cast>(v)->arg = mapIt(std::move(cast_tree<Cast>(v)->arg), ctx);

        if constexpr (HAS_MEMBER_postTransformCast<FUNC>::value) {
            return PostPonePostTransform_Cast<FUNC, CTX, HAS_MEMBER_postTransformCast<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapIt(TreePtr what, CTX ctx) {
        if (what == nullptr) {
            return what;
        }
        auto loc = what->loc;

        try {
            // TODO: reorder by frequency
            if constexpr (HAS_MEMBER_preTransformExpression<FUNC>::value) {
                what = PostPonePreTransform_Expression<FUNC, CTX, HAS_MEMBER_preTransformExpression<FUNC>::value>::call(
                    ctx, std::move(what), func);
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

    static_assert(!HAS_MEMBER_preTransformUnresolvedIdent<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformLiteral<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformUnresolvedConstantLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformConstantLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformLocal<FUNC>::value, "use post*Transform instead");

    ShallowMapper(FUNC &func) : func(func) {}

    TreePtr mapClassDef(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformClassDef<FUNC>::value) {
            v = PostPonePreTransform_ClassDef<FUNC, CTX, HAS_MEMBER_preTransformClassDef<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        // We intentionally do not walk v->ancestors nor v->singletonAncestors.
        // They are guaranteed to be simple trees in the desugarer.
        for (auto &def : cast_tree<ClassDef>(v)->rhs) {
            def = mapIt(std::move(def), ctx.withOwner(cast_tree<ClassDef>(v)->symbol));
        }

        if constexpr (HAS_MEMBER_postTransformClassDef<FUNC>::value) {
            return PostPonePostTransform_ClassDef<FUNC, CTX, HAS_MEMBER_postTransformClassDef<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapMethodDef(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformMethodDef<FUNC>::value) {
            v = PostPonePreTransform_MethodDef<FUNC, CTX, HAS_MEMBER_preTransformMethodDef<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        for (auto &arg : cast_tree<MethodDef>(v)->args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ = mapIt(std::move(optArg->default_), ctx.withOwner(cast_tree<MethodDef>(v)->symbol));
            }
        }
        // because this is a ShallowMap, we do not map over the body of the method

        if constexpr (HAS_MEMBER_postTransformMethodDef<FUNC>::value) {
            return PostPonePostTransform_MethodDef<FUNC, CTX, HAS_MEMBER_postTransformMethodDef<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapIf(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformIf<FUNC>::value) {
            v = PostPonePreTransform_If<FUNC, CTX, HAS_MEMBER_preTransformIf<FUNC>::value>::call(ctx, std::move(v),
                                                                                                 func);
        }
        cast_tree<If>(v)->cond = mapIt(std::move(cast_tree<If>(v)->cond), ctx);
        cast_tree<If>(v)->thenp = mapIt(std::move(cast_tree<If>(v)->thenp), ctx);
        cast_tree<If>(v)->elsep = mapIt(std::move(cast_tree<If>(v)->elsep), ctx);

        if constexpr (HAS_MEMBER_postTransformIf<FUNC>::value) {
            return PostPonePostTransform_If<FUNC, CTX, HAS_MEMBER_postTransformIf<FUNC>::value>::call(ctx, std::move(v),
                                                                                                      func);
        }
        return v;
    }

    TreePtr mapWhile(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformWhile<FUNC>::value) {
            v = PostPonePreTransform_While<FUNC, CTX, HAS_MEMBER_preTransformWhile<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        cast_tree<While>(v)->cond = mapIt(std::move(cast_tree<While>(v)->cond), ctx);
        cast_tree<While>(v)->body = mapIt(std::move(cast_tree<While>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformWhile<FUNC>::value) {
            return PostPonePostTransform_While<FUNC, CTX, HAS_MEMBER_postTransformWhile<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapBreak(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBreak<FUNC>::value) {
            return PostPonePreTransform_Break<FUNC, CTX, HAS_MEMBER_preTransformBreak<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        cast_tree<Break>(v)->expr = mapIt(std::move(cast_tree<Break>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformBreak<FUNC>::value) {
            return PostPonePostTransform_Break<FUNC, CTX, HAS_MEMBER_postTransformBreak<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }
    TreePtr mapRetry(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformRetry<FUNC>::value) {
            return PostPonePostTransform_Retry<FUNC, CTX, HAS_MEMBER_postTransformRetry<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapNext(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformNext<FUNC>::value) {
            return PostPonePreTransform_Next<FUNC, CTX, HAS_MEMBER_preTransformNext<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        cast_tree<Next>(v)->expr = mapIt(std::move(cast_tree<Next>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformNext<FUNC>::value) {
            return PostPonePostTransform_Next<FUNC, CTX, HAS_MEMBER_postTransformNext<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapReturn(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformReturn<FUNC>::value) {
            v = PostPonePreTransform_Return<FUNC, CTX, HAS_MEMBER_preTransformReturn<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        cast_tree<Return>(v)->expr = mapIt(std::move(cast_tree<Return>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformReturn<FUNC>::value) {
            return PostPonePostTransform_Return<FUNC, CTX, HAS_MEMBER_postTransformReturn<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapRescueCase(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescueCase<FUNC>::value) {
            v = PostPonePreTransform_RescueCase<FUNC, CTX, HAS_MEMBER_preTransformRescueCase<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        for (auto &el : cast_tree<RescueCase>(v)->exceptions) {
            el = mapIt(std::move(el), ctx);
        }

        cast_tree<RescueCase>(v)->var = mapIt(std::move(cast_tree<RescueCase>(v)->var), ctx);

        cast_tree<RescueCase>(v)->body = mapIt(std::move(cast_tree<RescueCase>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformRescueCase<FUNC>::value) {
            return PostPonePostTransform_RescueCase<FUNC, CTX, HAS_MEMBER_postTransformRescueCase<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }
    TreePtr mapRescue(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformRescue<FUNC>::value) {
            v = PostPonePreTransform_Rescue<FUNC, CTX, HAS_MEMBER_preTransformRescue<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        cast_tree<Rescue>(v)->body = mapIt(std::move(cast_tree<Rescue>(v)->body), ctx);

        for (auto &el : cast_tree<Rescue>(v)->rescueCases) {
            ENFORCE(isa_tree<RescueCase>(el), "invalid tree where rescue case was expected");
            el = mapRescueCase(std::move(el), ctx);
            ENFORCE(isa_tree<RescueCase>(el), "rescue case was mapped into non-rescue case");
        }

        cast_tree<Rescue>(v)->else_ = mapIt(std::move(cast_tree<Rescue>(v)->else_), ctx);
        cast_tree<Rescue>(v)->ensure = mapIt(std::move(cast_tree<Rescue>(v)->ensure), ctx);

        if constexpr (HAS_MEMBER_postTransformRescue<FUNC>::value) {
            return PostPonePostTransform_Rescue<FUNC, CTX, HAS_MEMBER_postTransformRescue<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapUnresolvedIdent(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedIdent<FUNC>::value) {
            return PostPonePostTransform_UnresolvedIdent<
                FUNC, CTX, HAS_MEMBER_postTransformUnresolvedIdent<FUNC>::value>::call(ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapAssign(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformAssign<FUNC>::value) {
            v = PostPonePreTransform_Assign<FUNC, CTX, HAS_MEMBER_preTransformAssign<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        cast_tree<Assign>(v)->lhs = mapIt(std::move(cast_tree<Assign>(v)->lhs), ctx);
        cast_tree<Assign>(v)->rhs = mapIt(std::move(cast_tree<Assign>(v)->rhs), ctx);

        if constexpr (HAS_MEMBER_postTransformAssign<FUNC>::value) {
            return PostPonePostTransform_Assign<FUNC, CTX, HAS_MEMBER_postTransformAssign<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapSend(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformSend<FUNC>::value) {
            v = PostPonePreTransform_Send<FUNC, CTX, HAS_MEMBER_preTransformSend<FUNC>::value>::call(ctx, std::move(v),
                                                                                                     func);
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

        if constexpr (HAS_MEMBER_postTransformSend<FUNC>::value) {
            return PostPonePostTransform_Send<FUNC, CTX, HAS_MEMBER_postTransformSend<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapHash(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformHash<FUNC>::value) {
            v = PostPonePreTransform_Hash<FUNC, CTX, HAS_MEMBER_preTransformHash<FUNC>::value>::call(ctx, std::move(v),
                                                                                                     func);
        }
        for (auto &key : cast_tree<Hash>(v)->keys) {
            key = mapIt(std::move(key), ctx);
        }

        for (auto &value : cast_tree<Hash>(v)->values) {
            value = mapIt(std::move(value), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>::value) {
            return PostPonePostTransform_Hash<FUNC, CTX, HAS_MEMBER_postTransformHash<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapArray(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformArray<FUNC>::value) {
            v = PostPonePreTransform_Array<FUNC, CTX, HAS_MEMBER_preTransformArray<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        for (auto &elem : cast_tree<Array>(v)->elems) {
            elem = mapIt(std::move(elem), ctx);
        }

        if constexpr (HAS_MEMBER_postTransformArray<FUNC>::value) {
            return PostPonePostTransform_Array<FUNC, CTX, HAS_MEMBER_postTransformArray<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapLiteral(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLiteral<FUNC>::value) {
            return PostPonePostTransform_Literal<FUNC, CTX, HAS_MEMBER_postTransformLiteral<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapUnresolvedConstantLit(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformUnresolvedConstantLit<FUNC>::value) {
            return PostPonePostTransform_UnresolvedConstantLit<
                FUNC, CTX, HAS_MEMBER_postTransformUnresolvedConstantLit<FUNC>::value>::call(ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapConstantLit(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformConstantLit<FUNC>::value) {
            return PostPonePostTransform_ConstantLit<FUNC, CTX, HAS_MEMBER_postTransformConstantLit<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapBlock(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformBlock<FUNC>::value) {
            v = PostPonePreTransform_Block<FUNC, CTX, HAS_MEMBER_preTransformBlock<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        for (auto &arg : cast_tree<Block>(v)->args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                optArg->default_ = mapIt(std::move(optArg->default_), ctx);
            }
        }
        cast_tree<Block>(v)->body = mapIt(std::move(cast_tree<Block>(v)->body), ctx);

        if constexpr (HAS_MEMBER_postTransformBlock<FUNC>::value) {
            return PostPonePostTransform_Block<FUNC, CTX, HAS_MEMBER_postTransformBlock<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapInsSeq(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformInsSeq<FUNC>::value) {
            v = PostPonePreTransform_InsSeq<FUNC, CTX, HAS_MEMBER_preTransformInsSeq<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        for (auto &stat : cast_tree<InsSeq>(v)->stats) {
            stat = mapIt(std::move(stat), ctx);
        }

        cast_tree<InsSeq>(v)->expr = mapIt(std::move(cast_tree<InsSeq>(v)->expr), ctx);

        if constexpr (HAS_MEMBER_postTransformInsSeq<FUNC>::value) {
            return PostPonePostTransform_InsSeq<FUNC, CTX, HAS_MEMBER_postTransformInsSeq<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapLocal(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_postTransformLocal<FUNC>::value) {
            return PostPonePostTransform_Local<FUNC, CTX, HAS_MEMBER_postTransformLocal<FUNC>::value>::call(
                ctx, std::move(v), func);
        }
        return v;
    }

    TreePtr mapCast(TreePtr v, CTX ctx) {
        if constexpr (HAS_MEMBER_preTransformCast<FUNC>::value) {
            v = PostPonePreTransform_Cast<FUNC, CTX, HAS_MEMBER_preTransformCast<FUNC>::value>::call(ctx, std::move(v),
                                                                                                     func);
        }
        cast_tree<Cast>(v)->arg = mapIt(std::move(cast_tree<Cast>(v)->arg), ctx);

        if constexpr (HAS_MEMBER_postTransformCast<FUNC>::value) {
            return PostPonePostTransform_Cast<FUNC, CTX, HAS_MEMBER_postTransformCast<FUNC>::value>::call(
                ctx, std::move(v), func);
        }

        return v;
    }

    TreePtr mapIt(TreePtr what, CTX ctx) {
        if (what == nullptr) {
            return what;
        }
        auto loc = what->loc;

        try {
            // TODO: reorder by frequency
            if constexpr (HAS_MEMBER_preTransformExpression<FUNC>::value) {
                what = PostPonePreTransform_Expression<FUNC, CTX, HAS_MEMBER_preTransformExpression<FUNC>::value>::call(
                    ctx, std::move(what), func);
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
