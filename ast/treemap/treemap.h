#ifndef SRUBY_TREEMAP_H
#define SRUBY_TREEMAP_H

#include "ast/Trees.h"
#include "core/Context.h"
#include "core/errors/internal.h"
#include "memory"
#include <type_traits> // To use 'std::integral_constant'.
#include <typeinfo>

using std::make_unique;
using std::unique_ptr;

namespace ruby_typer {
namespace ast {

class FUNC_EXAMPLE {
public:
    // all members are optional, but METHOD NAMES MATTER
    // Not including the member will skip the branch
    // you may return the same pointer that you are given
    // caller is repsonsible to handle it
    ClassDef *preTransformClassDef(core::MutableContext ctx, ClassDef *original);
    Expression *postTransformClassDef(core::MutableContext ctx, ClassDef *original);

    MethodDef *preTransformMethodDef(core::MutableContext ctx, MethodDef *original);
    Expression *postTransformMethodDef(core::MutableContext ctx, MethodDef *original);

    ConstDef *preTransformConstDef(core::MutableContext ctx, ConstDef *original);
    Expression *postTransformConstDef(core::MutableContext ctx, ConstDef *original);

    If *preTransformIf(core::MutableContext ctx, If *original);
    Expression *postTransformIf(core::MutableContext ctx, If *original);

    While *preTransformWhile(core::MutableContext ctx, While *original);
    Expression *postTransformWhile(core::MutableContext ctx, While *original);

    Expression *postTransformBreak(core::MutableContext ctx, Break *original);

    Expression *postTransformRetry(core::MutableContext ctx, Retry *original);

    Expression *postTransformNext(core::MutableContext ctx, Next *original);

    Return *preTransformReturn(core::MutableContext ctx, Return *original);
    Expression *postTransformReturn(core::MutableContext ctx, Return *original);

    Yield *preTransformYield(core::MutableContext ctx, Yield *original);
    Expression *postTransformYield(core::MutableContext ctx, Yield *original);

    RescueCase *preTransformRescueCase(core::MutableContext ctx, RescueCase *original);
    Expression *postTransformRescueCase(core::MutableContext ctx, RescueCase *original);

    Rescue *preTransformRescue(core::MutableContext ctx, Rescue *original);
    Expression *postTransformRescue(core::MutableContext ctx, Rescue *original);

    Expression *postTransformIdent(core::MutableContext ctx, Ident *original);
    Expression *postTransformUnresolvedIdent(core::MutableContext ctx, UnresolvedIdent *original);

    Assign *preTransformAssign(core::MutableContext ctx, Assign *original);
    Expression *postTransformAssign(core::MutableContext ctx, Assign *original);

    Send *preTransformSend(core::MutableContext ctx, Send *original);
    Expression *postTransformSend(core::MutableContext ctx, Send *original);

    Hash *preTransformHash(core::MutableContext ctx, Hash *original);
    Expression *postTransformHash(core::MutableContext ctx, Hash *original);

    Array *preTransformArray(core::MutableContext ctx, Array *original);
    Expression *postransformArray(core::MutableContext ctx, Array *original);

    Expression *postTransformBoolLit(core::MutableContext ctx, BoolLit *original);

    Expression *postTransformFloatLit(core::MutableContext ctx, FloatLit *original);

    Expression *postTransformIntLit(core::MutableContext ctx, IntLit *original);

    Expression *postTransformStringLit(core::MutableContext ctx, StringLit *original);
    Expression *postTransformSymbolLit(core::MutableContext ctx, StringLit *original);

    Expression *postTransformConstantLit(core::MutableContext ctx, ConstantLit *original);

    ArraySplat *preTransformArraySplat(core::MutableContext ctx, ArraySplat *original);
    Expression *postTransformArraySplat(core::MutableContext ctx, ArraySplat *original);

    HashSplat *preTransformHashSplat(core::MutableContext ctx, HashSplat *original);
    Expression *postTransformHashSplat(core::MutableContext ctx, HashSplat *original);

    Expression *postTransformSelf(core::MutableContext ctx, Self *original);

    Block *preTransformBlock(core::MutableContext ctx, Block *original);
    Expression *postTransformBlock(core::MutableContext ctx, Block *original);

    InsSeq *preTransformInsSeq(core::MutableContext ctx, InsSeq *original);
    Expression *postTransformInsSeq(core::MutableContext ctx, InsSeq *original);
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
        typedef char ArrayOfOne[1];                                                     \
        typedef char ArrayOfTwo[2];                                                     \
                                                                                        \
        template <typename U> static ArrayOfOne &func(Check<int Fallback::*, &U::X> *); \
        template <typename U> static ArrayOfTwo &func(...);                             \
                                                                                        \
    public:                                                                             \
        typedef HAS_MEMBER_##X type;                                                    \
        enum { value = sizeof(func<Derived>(0)) == 2 };                                 \
    };

GENERATE_HAS_MEMBER(preTransformExpression);
GENERATE_HAS_MEMBER(preTransformClassDef);
GENERATE_HAS_MEMBER(preTransformMethodDef);
GENERATE_HAS_MEMBER(preTransformConstDef);
GENERATE_HAS_MEMBER(preTransformIf);
GENERATE_HAS_MEMBER(preTransformWhile);
GENERATE_HAS_MEMBER(preTransformBreak);
GENERATE_HAS_MEMBER(preTransformRetry);
GENERATE_HAS_MEMBER(preTransformNext);
GENERATE_HAS_MEMBER(preTransformReturn);
GENERATE_HAS_MEMBER(preTransformYield);
GENERATE_HAS_MEMBER(preTransformRescueCase);
GENERATE_HAS_MEMBER(preTransformRescue);
GENERATE_HAS_MEMBER(preTransformAssign);
GENERATE_HAS_MEMBER(preTransformSend);
GENERATE_HAS_MEMBER(preTransformHash);
GENERATE_HAS_MEMBER(preTransformArray);
GENERATE_HAS_MEMBER(preTransformArraySplat);
GENERATE_HAS_MEMBER(preTransformHashSplat);
GENERATE_HAS_MEMBER(preTransformBlock);
GENERATE_HAS_MEMBER(preTransformInsSeq);

// used to check for ABSENCE of method
GENERATE_HAS_MEMBER(preTransformIdent);
GENERATE_HAS_MEMBER(preTransformUnresolvedIdent);
GENERATE_HAS_MEMBER(preTransformBoolLit);
GENERATE_HAS_MEMBER(preTransformStringLit);
GENERATE_HAS_MEMBER(preTransformSymbolLit);
GENERATE_HAS_MEMBER(preTransformFloatLit);
GENERATE_HAS_MEMBER(preTransformLocal);
GENERATE_HAS_MEMBER(preTransformIntLit);
GENERATE_HAS_MEMBER(preTransformConstantLit);
GENERATE_HAS_MEMBER(preTransformSelf);
GENERATE_HAS_MEMBER(preTransformCast);

GENERATE_HAS_MEMBER(postTransformClassDef);
GENERATE_HAS_MEMBER(postTransformMethodDef);
GENERATE_HAS_MEMBER(postTransformConstDef);
GENERATE_HAS_MEMBER(postTransformIf);
GENERATE_HAS_MEMBER(postTransformWhile);
GENERATE_HAS_MEMBER(postTransformBreak);
GENERATE_HAS_MEMBER(postTransformRetry);
GENERATE_HAS_MEMBER(postTransformNext);
GENERATE_HAS_MEMBER(postTransformReturn);
GENERATE_HAS_MEMBER(postTransformYield);
GENERATE_HAS_MEMBER(postTransformRescueCase);
GENERATE_HAS_MEMBER(postTransformRescue);
GENERATE_HAS_MEMBER(postTransformIdent);
GENERATE_HAS_MEMBER(postTransformUnresolvedIdent);
GENERATE_HAS_MEMBER(postTransformAssign);
GENERATE_HAS_MEMBER(postTransformSend);
GENERATE_HAS_MEMBER(postTransformHash);
GENERATE_HAS_MEMBER(postTransformLocal);
GENERATE_HAS_MEMBER(postTransformArray);
GENERATE_HAS_MEMBER(postTransformBoolLit);
GENERATE_HAS_MEMBER(postTransformFloatLit);
GENERATE_HAS_MEMBER(postTransformIntLit);
GENERATE_HAS_MEMBER(postTransformStringLit);
GENERATE_HAS_MEMBER(postTransformSymbolLit);
GENERATE_HAS_MEMBER(postTransformConstantLit);
GENERATE_HAS_MEMBER(postTransformArraySplat);
GENERATE_HAS_MEMBER(postTransformHashSplat);
GENERATE_HAS_MEMBER(postTransformSelf);
GENERATE_HAS_MEMBER(postTransformBlock);
GENERATE_HAS_MEMBER(postTransformInsSeq);
GENERATE_HAS_MEMBER(postTransformCast);

#define GENERATE_POSTPONE_PRECLASS(X)                                                   \
                                                                                        \
    template <class FUNC, class CTX, bool has> class PostPonePreTransform_##X {         \
    public:                                                                             \
        static X *call(CTX ctx, X *cd, FUNC &what) {                                    \
            Error::raise("should never be called. Incorrect use of TreeMap?");          \
            return nullptr;                                                             \
        }                                                                               \
    };                                                                                  \
                                                                                        \
    template <class FUNC, class CTX> class PostPonePreTransform_##X<FUNC, CTX, true> {  \
    public:                                                                             \
        static X *call(CTX ctx, X *cd, FUNC &func) {                                    \
            return func.preTransform##X(ctx, cd);                                       \
        }                                                                               \
    };                                                                                  \
                                                                                        \
    template <class FUNC, class CTX> class PostPonePreTransform_##X<FUNC, CTX, false> { \
    public:                                                                             \
        static X *call(CTX ctx, X *cd, FUNC &func) {                                    \
            return cd;                                                                  \
        }                                                                               \
    };

#define GENERATE_POSTPONE_POSTCLASS(X)                                                   \
                                                                                         \
    template <class FUNC, class CTX, bool has> class PostPonePostTransform_##X {         \
    public:                                                                              \
        static Expression *call(CTX ctx, X *cd, FUNC &what) {                            \
            Error::raise("should never be called. Incorrect use of TreeMap?");           \
            return nullptr;                                                              \
        }                                                                                \
    };                                                                                   \
                                                                                         \
    template <class FUNC, class CTX> class PostPonePostTransform_##X<FUNC, CTX, true> {  \
    public:                                                                              \
        static Expression *call(CTX ctx, X *cd, FUNC &func) {                            \
            return func.postTransform##X(ctx, cd);                                       \
        }                                                                                \
    };                                                                                   \
                                                                                         \
    template <class FUNC, class CTX> class PostPonePostTransform_##X<FUNC, CTX, false> { \
    public:                                                                              \
        static Expression *call(CTX ctx, X *cd, FUNC &func) {                            \
            return cd;                                                                   \
        }                                                                                \
    };

GENERATE_POSTPONE_PRECLASS(Expression);
GENERATE_POSTPONE_PRECLASS(ClassDef);
GENERATE_POSTPONE_PRECLASS(MethodDef);
GENERATE_POSTPONE_PRECLASS(ConstDef);
GENERATE_POSTPONE_PRECLASS(If);
GENERATE_POSTPONE_PRECLASS(While);
GENERATE_POSTPONE_PRECLASS(Break);
GENERATE_POSTPONE_PRECLASS(Retry);
GENERATE_POSTPONE_PRECLASS(Next);
GENERATE_POSTPONE_PRECLASS(Return);
GENERATE_POSTPONE_PRECLASS(Yield);
GENERATE_POSTPONE_PRECLASS(RescueCase);
GENERATE_POSTPONE_PRECLASS(Rescue);
GENERATE_POSTPONE_PRECLASS(Assign);
GENERATE_POSTPONE_PRECLASS(Send);
GENERATE_POSTPONE_PRECLASS(Hash);
GENERATE_POSTPONE_PRECLASS(Array);
GENERATE_POSTPONE_PRECLASS(ArraySplat);
GENERATE_POSTPONE_PRECLASS(HashSplat);
GENERATE_POSTPONE_PRECLASS(Block);
GENERATE_POSTPONE_PRECLASS(InsSeq);
GENERATE_POSTPONE_PRECLASS(Cast);

GENERATE_POSTPONE_POSTCLASS(ClassDef);
GENERATE_POSTPONE_POSTCLASS(MethodDef);
GENERATE_POSTPONE_POSTCLASS(ConstDef);
GENERATE_POSTPONE_POSTCLASS(If);
GENERATE_POSTPONE_POSTCLASS(While);
GENERATE_POSTPONE_POSTCLASS(Break);
GENERATE_POSTPONE_POSTCLASS(Retry);
GENERATE_POSTPONE_POSTCLASS(Next);
GENERATE_POSTPONE_POSTCLASS(Return);
GENERATE_POSTPONE_POSTCLASS(Yield);
GENERATE_POSTPONE_POSTCLASS(RescueCase);
GENERATE_POSTPONE_POSTCLASS(Rescue);
GENERATE_POSTPONE_POSTCLASS(Ident);
GENERATE_POSTPONE_POSTCLASS(UnresolvedIdent);
GENERATE_POSTPONE_POSTCLASS(Assign);
GENERATE_POSTPONE_POSTCLASS(Send);
GENERATE_POSTPONE_POSTCLASS(Hash);
GENERATE_POSTPONE_POSTCLASS(Array);
GENERATE_POSTPONE_POSTCLASS(BoolLit);
GENERATE_POSTPONE_POSTCLASS(Local);
GENERATE_POSTPONE_POSTCLASS(FloatLit);
GENERATE_POSTPONE_POSTCLASS(IntLit);
GENERATE_POSTPONE_POSTCLASS(StringLit);
GENERATE_POSTPONE_POSTCLASS(SymbolLit);
GENERATE_POSTPONE_POSTCLASS(ConstantLit);
GENERATE_POSTPONE_POSTCLASS(ArraySplat);
GENERATE_POSTPONE_POSTCLASS(HashSplat);
GENERATE_POSTPONE_POSTCLASS(Self);
GENERATE_POSTPONE_POSTCLASS(Block);
GENERATE_POSTPONE_POSTCLASS(InsSeq);
GENERATE_POSTPONE_POSTCLASS(Cast);

/**
 * Given a tree transformer FUNC transform a tree.
 * Tree is guaranteed to be visited in the definition order.
 * FUNC may maintain internal state.
 * @tparam tree transformer, see FUNC_EXAMPLE
 */
template <class FUNC, class CTX> class TreeMap {
private:
    FUNC &func;
    bool locReported = false;

    static_assert(!HAS_MEMBER_preTransformIdent<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformUnresolvedIdent<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformBoolLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformStringLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformSymbolLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformFloatLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformIntLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformConstantLit<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformSelf<FUNC>::value, "use post*Transform instead");
    static_assert(!HAS_MEMBER_preTransformLocal<FUNC>::value, "use post*Transform instead");

    TreeMap(FUNC &func) : func(func) {}

    Expression *mapIt(Expression *what, CTX ctx) {
        try {
            // TODO: reorder by frequency
            if (HAS_MEMBER_preTransformExpression<FUNC>::value) {
                what = PostPonePreTransform_Expression<FUNC, CTX, HAS_MEMBER_preTransformExpression<FUNC>::value>::call(
                    ctx, what, func);
            }

            if (what == nullptr || isa_tree<EmptyTree>(what) || isa_tree<ZSuperArgs>(what))
                return what;

            if (ClassDef *v = cast_tree<ClassDef>(what)) {
                if (HAS_MEMBER_preTransformClassDef<FUNC>::value) {
                    v = PostPonePreTransform_ClassDef<FUNC, CTX, HAS_MEMBER_preTransformClassDef<FUNC>::value>::call(
                        ctx, v, func);
                }

                for (auto &def : v->rhs) {
                    auto originalDef = def.get();
                    auto newDef = mapIt(originalDef, ctx.withOwner(v->symbol));
                    if (newDef != originalDef) {
                        def.reset(newDef);
                    }
                }

                if (HAS_MEMBER_postTransformClassDef<FUNC>::value) {
                    return PostPonePostTransform_ClassDef<FUNC, CTX,
                                                          HAS_MEMBER_postTransformClassDef<FUNC>::value>::call(ctx, v,
                                                                                                               func);
                }

                return v;
            } else if (MethodDef *v = cast_tree<MethodDef>(what)) {
                if (HAS_MEMBER_preTransformMethodDef<FUNC>::value) {
                    v = PostPonePreTransform_MethodDef<FUNC, CTX, HAS_MEMBER_preTransformMethodDef<FUNC>::value>::call(
                        ctx, v, func);
                }
                auto originalRhs = v->rhs.get();
                auto newRhs = mapIt(originalRhs, ctx.withOwner(v->symbol));
                if (newRhs != originalRhs) {
                    v->rhs.reset(newRhs);
                }

                if (HAS_MEMBER_postTransformMethodDef<FUNC>::value) {
                    return PostPonePostTransform_MethodDef<FUNC, CTX,
                                                           HAS_MEMBER_postTransformMethodDef<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
                }

                return v;
            } else if (ConstDef *v = cast_tree<ConstDef>(what)) {
                if (HAS_MEMBER_preTransformConstDef<FUNC>::value) {
                    v = PostPonePreTransform_ConstDef<FUNC, CTX, HAS_MEMBER_preTransformConstDef<FUNC>::value>::call(
                        ctx, v, func);
                }
                auto originalRhs = v->rhs.get();
                auto newRhs = mapIt(originalRhs, ctx.withOwner(v->symbol));
                if (newRhs != originalRhs) {
                    v->rhs.reset(newRhs);
                }

                if (HAS_MEMBER_postTransformConstDef<FUNC>::value) {
                    return PostPonePostTransform_ConstDef<FUNC, CTX,
                                                          HAS_MEMBER_postTransformConstDef<FUNC>::value>::call(ctx, v,
                                                                                                               func);
                }

                return v;
            } else if (If *v = cast_tree<If>(what)) {
                if (HAS_MEMBER_preTransformIf<FUNC>::value) {
                    v = PostPonePreTransform_If<FUNC, CTX, HAS_MEMBER_preTransformIf<FUNC>::value>::call(ctx, v, func);
                }
                auto originalCond = v->cond.get();
                auto originalThen = v->thenp.get();
                auto originalElse = v->elsep.get();
                auto newCond = mapIt(originalCond, ctx);
                auto newThen = mapIt(originalThen, ctx);
                auto newElse = mapIt(originalElse, ctx);
                if (newCond != originalCond) {
                    v->cond.reset(newCond);
                }
                if (originalThen != newThen) {
                    v->thenp.reset(newThen);
                }
                if (originalElse != newElse) {
                    v->elsep.reset(newElse);
                }
                if (HAS_MEMBER_postTransformIf<FUNC>::value) {
                    return PostPonePostTransform_If<FUNC, CTX, HAS_MEMBER_postTransformIf<FUNC>::value>::call(ctx, v,
                                                                                                              func);
                }
                return v;
            } else if (While *v = cast_tree<While>(what)) {
                if (HAS_MEMBER_preTransformWhile<FUNC>::value) {
                    v = PostPonePreTransform_While<FUNC, CTX, HAS_MEMBER_preTransformWhile<FUNC>::value>::call(ctx, v,
                                                                                                               func);
                }
                auto originalCond = v->cond.get();
                auto originalBody = v->body.get();
                auto newCond = mapIt(originalCond, ctx);
                auto newBody = mapIt(originalBody, ctx);
                if (newCond != originalCond) {
                    v->cond.reset(newCond);
                }
                if (newBody != originalBody) {
                    v->body.reset(newBody);
                }

                if (HAS_MEMBER_postTransformWhile<FUNC>::value) {
                    return PostPonePostTransform_While<FUNC, CTX, HAS_MEMBER_postTransformWhile<FUNC>::value>::call(
                        ctx, v, func);
                }

                return v;
            } else if (Break *v = cast_tree<Break>(what)) {
                if (HAS_MEMBER_preTransformBreak<FUNC>::value) {
                    return PostPonePreTransform_Break<FUNC, CTX, HAS_MEMBER_preTransformBreak<FUNC>::value>::call(
                        ctx, v, func);
                }

                auto oexpr = v->expr.get();
                auto nexpr = mapIt(oexpr, ctx);
                if (oexpr != nexpr) {
                    v->expr.reset(nexpr);
                }

                if (HAS_MEMBER_postTransformBreak<FUNC>::value) {
                    return PostPonePostTransform_Break<FUNC, CTX, HAS_MEMBER_postTransformBreak<FUNC>::value>::call(
                        ctx, v, func);
                }
                return v;
            } else if (Retry *v = cast_tree<Retry>(what)) {
                if (HAS_MEMBER_postTransformRetry<FUNC>::value) {
                    return PostPonePostTransform_Retry<FUNC, CTX, HAS_MEMBER_postTransformRetry<FUNC>::value>::call(
                        ctx, v, func);
                }
                return v;
            } else if (Next *v = cast_tree<Next>(what)) {
                if (HAS_MEMBER_preTransformNext<FUNC>::value) {
                    return PostPonePreTransform_Next<FUNC, CTX, HAS_MEMBER_preTransformNext<FUNC>::value>::call(ctx, v,
                                                                                                                func);
                }

                auto oexpr = v->expr.get();
                auto nexpr = mapIt(oexpr, ctx);
                if (oexpr != nexpr) {
                    v->expr.reset(nexpr);
                }

                if (HAS_MEMBER_postTransformNext<FUNC>::value) {
                    return PostPonePostTransform_Next<FUNC, CTX, HAS_MEMBER_postTransformNext<FUNC>::value>::call(
                        ctx, v, func);
                }
                return v;
            } else if (Return *v = cast_tree<Return>(what)) {
                if (HAS_MEMBER_preTransformReturn<FUNC>::value) {
                    v = PostPonePreTransform_Return<FUNC, CTX, HAS_MEMBER_preTransformReturn<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
                }
                auto oexpr = v->expr.get();
                auto nexpr = mapIt(oexpr, ctx);
                if (oexpr != nexpr) {
                    v->expr.reset(nexpr);
                }

                if (HAS_MEMBER_postTransformReturn<FUNC>::value) {
                    return PostPonePostTransform_Return<FUNC, CTX, HAS_MEMBER_postTransformReturn<FUNC>::value>::call(
                        ctx, v, func);
                }

                return v;
            } else if (Yield *v = cast_tree<Yield>(what)) {
                if (HAS_MEMBER_preTransformYield<FUNC>::value) {
                    v = PostPonePreTransform_Yield<FUNC, CTX, HAS_MEMBER_preTransformYield<FUNC>::value>::call(ctx, v,
                                                                                                               func);
                }
                auto oexpr = v->expr.get();
                auto nexpr = mapIt(oexpr, ctx);
                if (oexpr != nexpr) {
                    v->expr.reset(nexpr);
                }

                if (HAS_MEMBER_postTransformYield<FUNC>::value) {
                    return PostPonePostTransform_Yield<FUNC, CTX, HAS_MEMBER_postTransformYield<FUNC>::value>::call(
                        ctx, v, func);
                }

                return v;
            } else if (RescueCase *v = cast_tree<RescueCase>(what)) {
                if (HAS_MEMBER_preTransformRescueCase<FUNC>::value) {
                    v = PostPonePreTransform_RescueCase<FUNC, CTX,
                                                        HAS_MEMBER_preTransformRescueCase<FUNC>::value>::call(ctx, v,
                                                                                                              func);
                }
                int i = 0;
                while (i < v->exceptions.size()) {
                    auto &el = v->exceptions[i];
                    auto oarg = el.get();
                    auto narg = mapIt(oarg, ctx);
                    if (oarg != narg) {
                        el.reset(narg);
                    }
                    i++;
                }

                auto oexpr = v->var.get();
                auto nexpr = mapIt(oexpr, ctx);
                if (oexpr != nexpr) {
                    v->var.reset(nexpr);
                }

                oexpr = v->body.get();
                nexpr = mapIt(oexpr, ctx);
                if (oexpr != nexpr) {
                    v->body.reset(nexpr);
                }

                if (HAS_MEMBER_postTransformRescueCase<FUNC>::value) {
                    return PostPonePostTransform_RescueCase<
                        FUNC, CTX, HAS_MEMBER_postTransformRescueCase<FUNC>::value>::call(ctx, v, func);
                }

                return v;
            } else if (Rescue *v = cast_tree<Rescue>(what)) {
                if (HAS_MEMBER_preTransformRescue<FUNC>::value) {
                    v = PostPonePreTransform_Rescue<FUNC, CTX, HAS_MEMBER_preTransformRescue<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
                }

                auto oexpr = v->body.get();
                auto nexpr = mapIt(oexpr, ctx);
                if (oexpr != nexpr) {
                    v->body.reset(nexpr);
                }

                int i = 0;
                while (i < v->rescueCases.size()) {
                    auto &el = v->rescueCases[i];
                    auto oarg = el.get();
                    auto narg = mapIt(oarg, ctx);
                    if (oarg != narg) {
                        auto nargCase = cast_tree<RescueCase>(narg);
                        ENFORCE(nargCase != nullptr, "rescue case was mapped into non-a rescue case");
                        el.reset(nargCase);
                    }
                    i++;
                }

                oexpr = v->else_.get();
                nexpr = mapIt(oexpr, ctx);
                if (oexpr != nexpr) {
                    v->else_.reset(nexpr);
                }

                oexpr = v->ensure.get();
                nexpr = mapIt(oexpr, ctx);
                if (oexpr != nexpr) {
                    v->ensure.reset(nexpr);
                }

                if (HAS_MEMBER_postTransformRescue<FUNC>::value) {
                    return PostPonePostTransform_Rescue<FUNC, CTX, HAS_MEMBER_postTransformRescue<FUNC>::value>::call(
                        ctx, v, func);
                }

                return v;
            } else if (Ident *v = cast_tree<Ident>(what)) {
                if (HAS_MEMBER_postTransformIdent<FUNC>::value) {
                    return PostPonePostTransform_Ident<FUNC, CTX, HAS_MEMBER_postTransformIdent<FUNC>::value>::call(
                        ctx, v, func);
                }
                return v;
            } else if (UnresolvedIdent *v = cast_tree<UnresolvedIdent>(what)) {
                if (HAS_MEMBER_postTransformUnresolvedIdent<FUNC>::value) {
                    return PostPonePostTransform_UnresolvedIdent<
                        FUNC, CTX, HAS_MEMBER_postTransformUnresolvedIdent<FUNC>::value>::call(ctx, v, func);
                }
                return v;
            } else if (Assign *v = cast_tree<Assign>(what)) {
                if (HAS_MEMBER_preTransformAssign<FUNC>::value) {
                    v = PostPonePreTransform_Assign<FUNC, CTX, HAS_MEMBER_preTransformAssign<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
                }
                auto olhs = v->lhs.get();
                auto orhs = v->rhs.get();
                auto nlhs = mapIt(olhs, ctx);
                auto nrhs = mapIt(orhs, ctx);
                if (nlhs != olhs) {
                    v->lhs.reset(nlhs);
                }
                if (nrhs != orhs) {
                    v->rhs.reset(nrhs);
                }

                if (HAS_MEMBER_postTransformAssign<FUNC>::value) {
                    return PostPonePostTransform_Assign<FUNC, CTX, HAS_MEMBER_postTransformAssign<FUNC>::value>::call(
                        ctx, v, func);
                }

                return v;
            } else if (Send *v = cast_tree<Send>(what)) {
                if (HAS_MEMBER_preTransformSend<FUNC>::value) {
                    v = PostPonePreTransform_Send<FUNC, CTX, HAS_MEMBER_preTransformSend<FUNC>::value>::call(ctx, v,
                                                                                                             func);
                }
                auto orecv = v->recv.get();
                auto nrecv = mapIt(orecv, ctx);
                auto &vec = v->args;
                if (nrecv != orecv) {
                    v->recv.reset(nrecv);
                }
                auto i = 0;
                while (i < vec.size()) {
                    auto &el = vec[i];
                    auto oarg = el.get();
                    auto narg = mapIt(oarg, ctx);
                    if (oarg != narg) {
                        el.reset(narg);
                    }
                    i++;
                }

                if (v->block != nullptr) {
                    auto oblock = v->block.get();
                    auto nblock = mapIt(oblock, ctx);
                    if (oblock != nblock) {
                        ENFORCE(isa_tree<Block>(nblock), "block was mapped into not-a block");
                        v->block.reset(cast_tree<Block>(nblock));
                    }
                }

                if (HAS_MEMBER_postTransformSend<FUNC>::value) {
                    return PostPonePostTransform_Send<FUNC, CTX, HAS_MEMBER_postTransformSend<FUNC>::value>::call(
                        ctx, v, func);
                }

                return v;
            } else if (Hash *v = cast_tree<Hash>(what)) {
                if (HAS_MEMBER_preTransformHash<FUNC>::value) {
                    v = PostPonePreTransform_Hash<FUNC, CTX, HAS_MEMBER_preTransformHash<FUNC>::value>::call(ctx, v,
                                                                                                             func);
                }
                int i = 0;
                while (i < v->keys.size()) {
                    auto &el = v->keys[i];
                    auto oarg = el.get();
                    auto narg = mapIt(oarg, ctx);
                    if (oarg != narg) {
                        el.reset(narg);
                    }
                    i++;
                }

                i = 0;
                while (i < v->values.size()) {
                    auto &el = v->values[i];
                    auto oarg = el.get();
                    auto narg = mapIt(oarg, ctx);
                    if (oarg != narg) {
                        el.reset(narg);
                    }
                    i++;
                }

                if (HAS_MEMBER_postTransformArray<FUNC>::value) {
                    return PostPonePostTransform_Hash<FUNC, CTX, HAS_MEMBER_postTransformHash<FUNC>::value>::call(
                        ctx, v, func);
                }
                return what;
            } else if (Array *v = cast_tree<Array>(what)) {
                if (HAS_MEMBER_preTransformArray<FUNC>::value) {
                    v = PostPonePreTransform_Array<FUNC, CTX, HAS_MEMBER_preTransformArray<FUNC>::value>::call(ctx, v,
                                                                                                               func);
                }
                int i = 0;
                while (i < v->elems.size()) {
                    auto &el = v->elems[i];
                    auto oarg = el.get();
                    auto narg = mapIt(oarg, ctx);
                    if (oarg != narg) {
                        el.reset(narg);
                    }
                    i++;
                }

                if (HAS_MEMBER_postTransformArray<FUNC>::value) {
                    return PostPonePostTransform_Array<FUNC, CTX, HAS_MEMBER_postTransformArray<FUNC>::value>::call(
                        ctx, v, func);
                }
                return what;
            } else if (FloatLit *v = cast_tree<FloatLit>(what)) {
                if (HAS_MEMBER_postTransformFloatLit<FUNC>::value) {
                    return PostPonePostTransform_FloatLit<FUNC, CTX,
                                                          HAS_MEMBER_postTransformFloatLit<FUNC>::value>::call(ctx, v,
                                                                                                               func);
                }
                return v;
            } else if (BoolLit *v = cast_tree<BoolLit>(what)) {
                if (HAS_MEMBER_postTransformBoolLit<FUNC>::value) {
                    return PostPonePostTransform_BoolLit<FUNC, CTX, HAS_MEMBER_postTransformBoolLit<FUNC>::value>::call(
                        ctx, v, func);
                }
                return v;
            } else if (IntLit *v = cast_tree<IntLit>(what)) {
                if (HAS_MEMBER_postTransformIntLit<FUNC>::value) {
                    return PostPonePostTransform_IntLit<FUNC, CTX, HAS_MEMBER_postTransformIntLit<FUNC>::value>::call(
                        ctx, v, func);
                }
                return v;
            } else if (StringLit *v = cast_tree<StringLit>(what)) {
                if (HAS_MEMBER_postTransformStringLit<FUNC>::value) {
                    return PostPonePostTransform_StringLit<FUNC, CTX,
                                                           HAS_MEMBER_postTransformStringLit<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
                }
                return v;
            } else if (ConstantLit *v = cast_tree<ConstantLit>(what)) {
                if (HAS_MEMBER_postTransformConstantLit<FUNC>::value) {
                    return PostPonePostTransform_ConstantLit<
                        FUNC, CTX, HAS_MEMBER_postTransformConstantLit<FUNC>::value>::call(ctx, v, func);
                }
                return v;
            } else if (BoolLit *v = cast_tree<BoolLit>(what)) {
                // Error::notImplemented();
                return what;
            } else if (ArraySplat *v = cast_tree<ArraySplat>(what)) {
                if (HAS_MEMBER_preTransformArraySplat<FUNC>::value) {
                    v = PostPonePreTransform_ArraySplat<FUNC, CTX,
                                                        HAS_MEMBER_preTransformArraySplat<FUNC>::value>::call(ctx, v,
                                                                                                              func);
                }
                auto oarg = v->arg.get();
                auto narg = mapIt(oarg, ctx);
                if (oarg != narg) {
                    v->arg.reset(narg);
                }

                if (HAS_MEMBER_postTransformArraySplat<FUNC>::value) {
                    return PostPonePostTransform_ArraySplat<
                        FUNC, CTX, HAS_MEMBER_postTransformArraySplat<FUNC>::value>::call(ctx, v, func);
                }

                return v;
            } else if (HashSplat *v = cast_tree<HashSplat>(what)) {
                if (HAS_MEMBER_preTransformHashSplat<FUNC>::value) {
                    v = PostPonePreTransform_HashSplat<FUNC, CTX, HAS_MEMBER_preTransformHashSplat<FUNC>::value>::call(
                        ctx, v, func);
                }
                auto oarg = v->arg.get();
                auto narg = mapIt(oarg, ctx);
                if (oarg != narg) {
                    v->arg.reset(narg);
                }

                if (HAS_MEMBER_postTransformHashSplat<FUNC>::value) {
                    return PostPonePostTransform_HashSplat<FUNC, CTX,
                                                           HAS_MEMBER_postTransformHashSplat<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
                }
                return v;
            } else if (SymbolLit *v = cast_tree<SymbolLit>(what)) {
                if (HAS_MEMBER_postTransformSymbolLit<FUNC>::value) {
                    return PostPonePostTransform_SymbolLit<FUNC, CTX,
                                                           HAS_MEMBER_postTransformSymbolLit<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
                }
                return v;
            } else if (Self *v = cast_tree<Self>(what)) {
                if (HAS_MEMBER_postTransformSelf<FUNC>::value) {
                    return PostPonePostTransform_Self<FUNC, CTX, HAS_MEMBER_postTransformSelf<FUNC>::value>::call(
                        ctx, v, func);
                }
                return v;
            } else if (Block *v = cast_tree<Block>(what)) {
                if (HAS_MEMBER_preTransformBlock<FUNC>::value) {
                    v = PostPonePreTransform_Block<FUNC, CTX, HAS_MEMBER_preTransformBlock<FUNC>::value>::call(ctx, v,
                                                                                                               func);
                }

                auto originalBody = v->body.get();
                auto newBody = mapIt(originalBody, ctx.withOwner(v->symbol));
                if (newBody != originalBody) {
                    v->body.reset(newBody);
                }

                if (HAS_MEMBER_postTransformBlock<FUNC>::value) {
                    return PostPonePostTransform_Block<FUNC, CTX, HAS_MEMBER_postTransformBlock<FUNC>::value>::call(
                        ctx, v, func);
                }
                return v;
            } else if (InsSeq *v = cast_tree<InsSeq>(what)) {
                if (HAS_MEMBER_preTransformInsSeq<FUNC>::value) {
                    v = PostPonePreTransform_InsSeq<FUNC, CTX, HAS_MEMBER_preTransformInsSeq<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
                }
                auto &stats = v->stats;
                auto oexpr = v->expr.get();
                auto i = 0;
                while (i < stats.size()) {
                    auto &el = stats[i];
                    auto oexp = el.get();
                    auto nexp = mapIt(oexp, ctx);
                    if (oexp != nexp) {
                        el.reset(nexp);
                    }
                    i++;
                }
                auto nexpr = mapIt(oexpr, ctx);
                if (nexpr != oexpr) {
                    v->expr.reset(nexpr);
                }

                if (HAS_MEMBER_postTransformInsSeq<FUNC>::value) {
                    return PostPonePostTransform_InsSeq<FUNC, CTX, HAS_MEMBER_postTransformInsSeq<FUNC>::value>::call(
                        ctx, v, func);
                }

                return v;
            } else if (Local *v = cast_tree<Local>(what)) {
                if (HAS_MEMBER_postTransformLocal<FUNC>::value) {
                    return PostPonePostTransform_Local<FUNC, CTX, HAS_MEMBER_postTransformLocal<FUNC>::value>::call(
                        ctx, v, func);
                }
                return v;
            } else if (Cast *v = cast_tree<Cast>(what)) {
                if (HAS_MEMBER_preTransformCast<FUNC>::value) {
                    v = PostPonePreTransform_Cast<FUNC, CTX, HAS_MEMBER_preTransformCast<FUNC>::value>::call(ctx, v,
                                                                                                             func);
                }
                auto oarg = v->arg.get();
                auto narg = mapIt(oarg, ctx);
                if (oarg != narg) {
                    v->arg.reset(narg);
                }

                if (HAS_MEMBER_postTransformCast<FUNC>::value) {
                    return PostPonePostTransform_Cast<FUNC, CTX, HAS_MEMBER_postTransformCast<FUNC>::value>::call(
                        ctx, v, func);
                }

                return v;
            } else {
                Error::raise("should never happen. Forgot to add new tree kind? ", demangle(typeid(*what).name()));
            }
        } catch (...) {
            if (!locReported) {
                locReported = true;
                ctx.state.error(what->loc, core::errors::Internal::InternalError,
                                "Failed to process tree (backtrace is above)");
            }
            throw;
        }
    }

public:
    static unique_ptr<Expression> apply(CTX ctx, FUNC &func, unique_ptr<Expression> to) {
        Expression *underlying = to.get();
        TreeMap walker(func);
        Expression *res = walker.mapIt(underlying, ctx);

        if (res == underlying) {
            return to;
        } else {
            return std::unique_ptr<Expression>(res);
        }
    }
};

} // namespace ast
} // namespace ruby_typer

#endif // SRUBY_TREEMAP_H
