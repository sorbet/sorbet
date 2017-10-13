#ifndef SRUBY_TREEMAP_H
#define SRUBY_TREEMAP_H

#include "Trees.h"
#include "memory"
#include <type_traits> // To use 'std::integral_constant'.

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
    ClassDef *preTransformClassDef(ClassDef *original);
    Statement *postTransformClassDef(ClassDef *original);

    MethodDef *preTransformMethodDef(MethodDef *original);
    Statement *postTransformMethodDef(MethodDef *original);

    If *preTransformIf(If *original);
    Statement *postTransformIf(If *original);

    While *preTransformWhile(While *original);
    Statement *postTransformWhile(While *original);

    For *preTransformFor(For *original);
    Statement *postTransformFor(For *original);

    Statement *postTransformBreak(Break *original);

    Statement *postTransformNext(Next *original);

    Return *preTransformReturn(Return *original);
    Statement *postTransformReturn(Return *original);

    Rescue *preTransformRescue(Rescue *original);
    Statement *postTransformRescue(Rescue *original);

    Statement *postTransformIdent(Ident *original);

    Assign *preTransformAssign(Assign *original);
    Statement *postTransformAssign(Assign *original);

    Send *preTransformSend(Send *original);
    Statement *postTransformSend(Send *original);

    New *preTransformNew(New *original);
    Statement *postTransformNew(New *original);

    NamedArg *preTransformNamedArg(NamedArg *original);
    Statement *postTransformNamedArg(NamedArg *original);

    Hash *preTransformHash(Hash *original);
    Statement *postTransformHash(Hash *original);

    Array *preTransformArray(Array *original);
    Statement *postransformArray(Array *original);

    Statement *postTransformFloatLit(FloatLit *original);

    Statement *postTransformIntLit(IntLit *original);

    Statement *postTransformStringLit(StringLit *original);

    Statement *postTransformConstantLit(ConstantLit *original);

    ArraySplat *preTransformArraySplat(ArraySplat *original);
    Statement *postTransformArraySplat(ArraySplat *original);

    HashSplat *preTransformHashSplat(HashSplat *original);
    Statement *postTransformHashSplat(HashSplat *original);

    Statement *postTransformSelf(Self *original);

    Block *preTransformBlock(Block *original);
    Statement *postTransformBlock(Block *original);

    InsSeq *preTransformInsSeq(InsSeq *original);
    Statement *postTransformInsSeq(InsSeq *original);
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

GENERATE_HAS_MEMBER(preTransformClassDef);
GENERATE_HAS_MEMBER(preTransformMethodDef);
GENERATE_HAS_MEMBER(preTransformConstDef);
GENERATE_HAS_MEMBER(preTransformIf);
GENERATE_HAS_MEMBER(preTransformWhile);
GENERATE_HAS_MEMBER(preTransformFor);
GENERATE_HAS_MEMBER(preTransformReturn);
GENERATE_HAS_MEMBER(preTransformRescue);
GENERATE_HAS_MEMBER(preTransformAssign);
GENERATE_HAS_MEMBER(preTransformSend);
GENERATE_HAS_MEMBER(preTransformNew);
GENERATE_HAS_MEMBER(preTransformNamedArg);
GENERATE_HAS_MEMBER(preTransformHash);
GENERATE_HAS_MEMBER(preTransformArray);
GENERATE_HAS_MEMBER(preTransformArraySplat);
GENERATE_HAS_MEMBER(preTransformHashSplat);
GENERATE_HAS_MEMBER(preTransformBlock);
GENERATE_HAS_MEMBER(preTransformInsSeq);

GENERATE_HAS_MEMBER(postTransformClassDef);
GENERATE_HAS_MEMBER(postTransformMethodDef);
GENERATE_HAS_MEMBER(postTransformConstDef);
GENERATE_HAS_MEMBER(postTransformIf);
GENERATE_HAS_MEMBER(postTransformWhile);
GENERATE_HAS_MEMBER(postTransformFor);
GENERATE_HAS_MEMBER(postTransformBreak);
GENERATE_HAS_MEMBER(postTransformNext);
GENERATE_HAS_MEMBER(postTransformReturn);
GENERATE_HAS_MEMBER(postTransformRescue);
GENERATE_HAS_MEMBER(postTransformIdent);
GENERATE_HAS_MEMBER(postTransformAssign);
GENERATE_HAS_MEMBER(postTransformSend);
GENERATE_HAS_MEMBER(postTransformNew);
GENERATE_HAS_MEMBER(postTransformNamedArg);
GENERATE_HAS_MEMBER(postTransformHash);
GENERATE_HAS_MEMBER(postTransformArray);
GENERATE_HAS_MEMBER(postTransformFloatLit);
GENERATE_HAS_MEMBER(postTransformIntLit);
GENERATE_HAS_MEMBER(postTransformStringLit);
GENERATE_HAS_MEMBER(postTransformConstantLit);
GENERATE_HAS_MEMBER(postTransformArraySplat);
GENERATE_HAS_MEMBER(postTransformHashSplat);
GENERATE_HAS_MEMBER(postTransformSelf);
GENERATE_HAS_MEMBER(postTransformBlock);
GENERATE_HAS_MEMBER(postTransformInsSeq);

#define GENERATE_POSTPONE_PRECLASS(X)                                          \
                                                                               \
    template <class FUNC, bool has> class PostPonePreTransform_##X {           \
    public:                                                                    \
        static X *call(Context ctx, X *cd, FUNC &what) {                       \
            Error::raise("should never be called. Incorrect use of TreeMap?"); \
            return nullptr;                                                    \
        }                                                                      \
    };                                                                         \
                                                                               \
    template <class FUNC> class PostPonePreTransform_##X<FUNC, true> {         \
    public:                                                                    \
        static X *call(Context ctx, X *cd, FUNC &func) {                       \
            return func.preTransform##X(ctx, cd);                              \
        }                                                                      \
    };                                                                         \
                                                                               \
    template <class FUNC> class PostPonePreTransform_##X<FUNC, false> {        \
    public:                                                                    \
        static X *call(Context ctx, X *cd, FUNC &func) {                       \
            return cd;                                                         \
        }                                                                      \
    };

#define GENERATE_POSTPONE_POSTCLASS(X)                                         \
                                                                               \
    template <class FUNC, bool has> class PostPonePostTransform_##X {          \
    public:                                                                    \
        static Statement *call(Context ctx, X *cd, FUNC &what) {               \
            Error::raise("should never be called. Incorrect use of TreeMap?"); \
            return nullptr;                                                    \
        }                                                                      \
    };                                                                         \
                                                                               \
    template <class FUNC> class PostPonePostTransform_##X<FUNC, true> {        \
    public:                                                                    \
        static Statement *call(Context ctx, X *cd, FUNC &func) {               \
            return func.postTransform##X(ctx, cd);                             \
        }                                                                      \
    };                                                                         \
                                                                               \
    template <class FUNC> class PostPonePostTransform_##X<FUNC, false> {       \
    public:                                                                    \
        static Statement *call(Context ctx, X *cd, FUNC &func) {               \
            return cd;                                                         \
        }                                                                      \
    };

GENERATE_POSTPONE_PRECLASS(ClassDef);
GENERATE_POSTPONE_PRECLASS(MethodDef);
GENERATE_POSTPONE_PRECLASS(ConstDef);
GENERATE_POSTPONE_PRECLASS(If);
GENERATE_POSTPONE_PRECLASS(While);
GENERATE_POSTPONE_PRECLASS(For);
GENERATE_POSTPONE_PRECLASS(Return);
GENERATE_POSTPONE_PRECLASS(Assign);
GENERATE_POSTPONE_PRECLASS(Send);
GENERATE_POSTPONE_PRECLASS(New);
GENERATE_POSTPONE_PRECLASS(NamedArg);
GENERATE_POSTPONE_PRECLASS(Hash);
GENERATE_POSTPONE_PRECLASS(Array);
GENERATE_POSTPONE_PRECLASS(ArraySplat);
GENERATE_POSTPONE_PRECLASS(HashSplat);
GENERATE_POSTPONE_PRECLASS(Block);
GENERATE_POSTPONE_PRECLASS(InsSeq);

GENERATE_POSTPONE_POSTCLASS(ClassDef);
GENERATE_POSTPONE_POSTCLASS(MethodDef);
GENERATE_POSTPONE_POSTCLASS(ConstDef);
GENERATE_POSTPONE_POSTCLASS(If);
GENERATE_POSTPONE_POSTCLASS(While);
GENERATE_POSTPONE_POSTCLASS(For);
GENERATE_POSTPONE_POSTCLASS(Break);
GENERATE_POSTPONE_POSTCLASS(Next);
GENERATE_POSTPONE_POSTCLASS(Return);
GENERATE_POSTPONE_POSTCLASS(Ident);
GENERATE_POSTPONE_POSTCLASS(Assign);
GENERATE_POSTPONE_POSTCLASS(Send);
GENERATE_POSTPONE_POSTCLASS(New);
GENERATE_POSTPONE_POSTCLASS(NamedArg);
GENERATE_POSTPONE_POSTCLASS(Hash);
GENERATE_POSTPONE_POSTCLASS(Array);
GENERATE_POSTPONE_POSTCLASS(FloatLit);
GENERATE_POSTPONE_POSTCLASS(IntLit);
GENERATE_POSTPONE_POSTCLASS(StringLit);
GENERATE_POSTPONE_POSTCLASS(ConstantLit);
GENERATE_POSTPONE_POSTCLASS(ArraySplat);
GENERATE_POSTPONE_POSTCLASS(HashSplat);
GENERATE_POSTPONE_POSTCLASS(Self);
GENERATE_POSTPONE_POSTCLASS(Block);
GENERATE_POSTPONE_POSTCLASS(InsSeq);

/**
 * Given a tree transformer FUNC transform a tree.
 * Tree is guaranteed to be visited in the definition order.
 * FUNC may maintain internal state.
 * @tparam tree transformer, see FUNC_EXAMPLE
 */
template <class FUNC> class TreeMap {
private:
    FUNC &func;

    TreeMap(FUNC &func) : func(func) {}

    Statement *mapIt(Statement *what, Context ctx) {
        // TODO: reorder by frequency
        if (what == nullptr || dynamic_cast<EmptyTree *>(what) != nullptr)
            return what;
        if (ClassDef *v = dynamic_cast<ClassDef *>(what)) {

            if (HAS_MEMBER_preTransformClassDef<FUNC>::value) {
                v = PostPonePreTransform_ClassDef<FUNC, HAS_MEMBER_preTransformClassDef<FUNC>::value>::call(ctx, v,
                                                                                                            func);
            }

            for (auto &def : v->rhs) {
                auto originalDef = def.get();
                auto newDef = mapIt(originalDef, ctx.withOwner(v->symbol));
                if (newDef != originalDef) {
                    def.reset(newDef);
                }
            }

            if (HAS_MEMBER_postTransformClassDef<FUNC>::value) {
                return PostPonePostTransform_ClassDef<FUNC, HAS_MEMBER_postTransformClassDef<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
            }

            return v;
        } else if (MethodDef *v = dynamic_cast<MethodDef *>(what)) {
            if (HAS_MEMBER_preTransformMethodDef<FUNC>::value) {
                v = PostPonePreTransform_MethodDef<FUNC, HAS_MEMBER_preTransformMethodDef<FUNC>::value>::call(ctx, v,
                                                                                                              func);
            }
            auto originalRhs = v->rhs.get();
            auto newRhs = mapIt(originalRhs, ctx.withOwner(v->symbol));
            if (newRhs != originalRhs) {
                Error::check(dynamic_cast<Expression *>(newRhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expression *>(newRhs));
            }

            if (HAS_MEMBER_postTransformMethodDef<FUNC>::value) {
                return PostPonePostTransform_MethodDef<FUNC, HAS_MEMBER_postTransformMethodDef<FUNC>::value>::call(
                    ctx, v, func);
            }

            return v;
        } else if (ConstDef *v = dynamic_cast<ConstDef *>(what)) {
            if (HAS_MEMBER_preTransformConstDef<FUNC>::value) {
                v = PostPonePreTransform_ConstDef<FUNC, HAS_MEMBER_preTransformConstDef<FUNC>::value>::call(ctx, v,
                                                                                                            func);
            }
            auto originalRhs = v->rhs.get();
            auto newRhs = mapIt(originalRhs, ctx.withOwner(v->symbol));
            if (newRhs != originalRhs) {
                Error::check(dynamic_cast<Expression *>(newRhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expression *>(newRhs));
            }

            if (HAS_MEMBER_postTransformConstDef<FUNC>::value) {
                return PostPonePostTransform_ConstDef<FUNC, HAS_MEMBER_postTransformConstDef<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
            }

            return v;
        } else if (If *v = dynamic_cast<If *>(what)) {
            if (HAS_MEMBER_preTransformIf<FUNC>::value) {
                v = PostPonePreTransform_If<FUNC, HAS_MEMBER_preTransformIf<FUNC>::value>::call(ctx, v, func);
            }
            auto originalCond = v->cond.get();
            auto originalThen = v->thenp.get();
            auto originalElse = v->elsep.get();
            auto newCond = mapIt(originalCond, ctx);
            auto newThen = mapIt(originalThen, ctx);
            auto newElse = mapIt(originalElse, ctx);
            if (newCond != originalCond) {
                Error::check(dynamic_cast<Expression *>(newCond) != nullptr);
                v->cond.reset(dynamic_cast<Expression *>(newCond));
            }
            if (originalThen != newThen) {
                Error::check(dynamic_cast<Expression *>(newThen) != nullptr);
                v->thenp.reset(dynamic_cast<Expression *>(newThen));
            }
            if (originalElse != newElse) {
                Error::check(dynamic_cast<Expression *>(newElse) != nullptr);
                v->elsep.reset(dynamic_cast<Expression *>(newElse));
            }
            if (HAS_MEMBER_postTransformIf<FUNC>::value) {
                return PostPonePostTransform_If<FUNC, HAS_MEMBER_postTransformIf<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (While *v = dynamic_cast<While *>(what)) {
            if (HAS_MEMBER_preTransformWhile<FUNC>::value) {
                v = PostPonePreTransform_While<FUNC, HAS_MEMBER_preTransformWhile<FUNC>::value>::call(ctx, v, func);
            }
            auto originalCond = v->cond.get();
            auto originalBody = v->body.get();
            auto newCond = mapIt(originalCond, ctx);
            auto newBody = mapIt(originalBody, ctx);
            if (newCond != originalCond) {
                Error::check(dynamic_cast<Expression *>(newCond) != nullptr);
                v->cond.reset(dynamic_cast<Expression *>(newCond));
            }
            if (newBody != originalBody) {
                v->body.reset(newBody);
            }

            if (HAS_MEMBER_postTransformWhile<FUNC>::value) {
                return PostPonePostTransform_While<FUNC, HAS_MEMBER_postTransformWhile<FUNC>::value>::call(ctx, v,
                                                                                                           func);
            }

            return v;
        } else if (For *v = dynamic_cast<For *>(what)) {
            Error::notImplemented();
        } else if (Break *v = dynamic_cast<Break *>(what)) {
            if (HAS_MEMBER_postTransformBreak<FUNC>::value) {
                return PostPonePostTransform_Break<FUNC, HAS_MEMBER_postTransformBreak<FUNC>::value>::call(ctx, v,
                                                                                                           func);
            }
            return v;
        } else if (Next *v = dynamic_cast<Next *>(what)) {
            if (HAS_MEMBER_postTransformNext<FUNC>::value) {
                return PostPonePostTransform_Next<FUNC, HAS_MEMBER_postTransformNext<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (Return *v = dynamic_cast<Return *>(what)) {
            if (HAS_MEMBER_preTransformReturn<FUNC>::value) {
                v = PostPonePreTransform_Return<FUNC, HAS_MEMBER_preTransformReturn<FUNC>::value>::call(ctx, v, func);
            }
            auto oexpr = v->expr.get();
            auto nexpr = mapIt(oexpr, ctx);
            if (oexpr != nexpr) {
                Error::check(dynamic_cast<Expression *>(nexpr) != nullptr);
                v->expr.reset(dynamic_cast<Expression *>(nexpr));
            }

            if (HAS_MEMBER_postTransformReturn<FUNC>::value) {
                return PostPonePostTransform_Return<FUNC, HAS_MEMBER_postTransformReturn<FUNC>::value>::call(ctx, v,
                                                                                                             func);
            }

            return v;
        } else if (Ident *v = dynamic_cast<Ident *>(what)) {
            if (HAS_MEMBER_postTransformIdent<FUNC>::value) {
                return PostPonePostTransform_Ident<FUNC, HAS_MEMBER_postTransformIdent<FUNC>::value>::call(ctx, v,
                                                                                                           func);
            }
            return v;
        } else if (Assign *v = dynamic_cast<Assign *>(what)) {
            if (HAS_MEMBER_preTransformAssign<FUNC>::value) {
                v = PostPonePreTransform_Assign<FUNC, HAS_MEMBER_preTransformAssign<FUNC>::value>::call(ctx, v, func);
            }
            auto olhs = v->lhs.get();
            auto orhs = v->rhs.get();
            auto nlhs = mapIt(olhs, ctx);
            auto nrhs = mapIt(orhs, ctx);
            if (nlhs != olhs) {
                Error::check(dynamic_cast<Expression *>(nlhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expression *>(nlhs));
            }
            if (nrhs != orhs) {
                Error::check(dynamic_cast<Expression *>(nrhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expression *>(nrhs));
            }

            if (HAS_MEMBER_postTransformAssign<FUNC>::value) {
                return PostPonePostTransform_Assign<FUNC, HAS_MEMBER_postTransformAssign<FUNC>::value>::call(ctx, v,
                                                                                                             func);
            }

            return v;
        } else if (Send *v = dynamic_cast<Send *>(what)) {
            if (HAS_MEMBER_preTransformSend<FUNC>::value) {
                v = PostPonePreTransform_Send<FUNC, HAS_MEMBER_preTransformSend<FUNC>::value>::call(ctx, v, func);
            }
            auto orecv = v->recv.get();
            auto nrecv = mapIt(orecv, ctx);
            auto &vec = v->args;
            if (nrecv != orecv) {
                Error::check(dynamic_cast<Expression *>(nrecv) != nullptr);
                v->recv.reset(dynamic_cast<Expression *>(nrecv));
            }
            auto i = 0;
            while (i < vec.size()) {
                auto &el = vec[i];
                auto oarg = el.get();
                auto narg = mapIt(oarg, ctx);
                if (oarg != narg) {
                    Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                    el.reset(dynamic_cast<Expression *>(narg));
                }
                i++;
            }

            if (HAS_MEMBER_postTransformSend<FUNC>::value) {
                return PostPonePostTransform_Send<FUNC, HAS_MEMBER_postTransformSend<FUNC>::value>::call(ctx, v, func);
            }

            return v;
        } else if (New *v = dynamic_cast<New *>(what)) {
            if (HAS_MEMBER_preTransformNew<FUNC>::value) {
                v = PostPonePreTransform_New<FUNC, HAS_MEMBER_preTransformNew<FUNC>::value>::call(ctx, v, func);
            }
            auto &args = v->args;
            auto i = 0;
            while (i < args.size()) {
                auto &el = args[i];
                auto oarg = el.get();
                auto narg = mapIt(oarg, ctx);
                if (oarg != narg) {
                    Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                    el.reset(dynamic_cast<Expression *>(narg));
                }
                i++;
            }

            if (HAS_MEMBER_postTransformNew<FUNC>::value) {
                return PostPonePostTransform_New<FUNC, HAS_MEMBER_postTransformNew<FUNC>::value>::call(ctx, v, func);
            }

            return v;
        } else if (NamedArg *v = dynamic_cast<NamedArg *>(what)) {
            if (HAS_MEMBER_preTransformNamedArg<FUNC>::value) {
                v = PostPonePreTransform_NamedArg<FUNC, HAS_MEMBER_preTransformNamedArg<FUNC>::value>::call(ctx, v,
                                                                                                            func);
            }
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg, ctx);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expression *>(narg));
            }

            if (HAS_MEMBER_postTransformNamedArg<FUNC>::value) {
                return PostPonePostTransform_NamedArg<FUNC, HAS_MEMBER_postTransformNamedArg<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
            }

            return v;
        } else if (Hash *v = dynamic_cast<Hash *>(what)) {
            Error::notImplemented();
        } else if (Array *v = dynamic_cast<Array *>(what)) {
            Error::notImplemented();
        } else if (FloatLit *v = dynamic_cast<FloatLit *>(what)) {
            if (HAS_MEMBER_postTransformFloatLit<FUNC>::value) {
                return PostPonePostTransform_FloatLit<FUNC, HAS_MEMBER_postTransformFloatLit<FUNC>::value>::call(ctx, v,
                                                                                                                 func);
            }
            return v;
        } else if (IntLit *v = dynamic_cast<IntLit *>(what)) {
            if (HAS_MEMBER_postTransformIntLit<FUNC>::value) {
                return PostPonePostTransform_IntLit<FUNC, HAS_MEMBER_postTransformIntLit<FUNC>::value>::call(ctx, v,
                                                                                                             func);
            }
            return v;
        } else if (StringLit *v = dynamic_cast<StringLit *>(what)) {
            if (HAS_MEMBER_postTransformStringLit<FUNC>::value) {
                return PostPonePostTransform_StringLit<FUNC, HAS_MEMBER_postTransformStringLit<FUNC>::value>::call(
                    ctx, v, func);
            }
            return v;
        } else if (ConstantLit *v = dynamic_cast<ConstantLit *>(what)) {
            if (HAS_MEMBER_postTransformConstantLit<FUNC>::value) {
                return PostPonePostTransform_ConstantLit<FUNC, HAS_MEMBER_postTransformConstantLit<FUNC>::value>::call(
                    ctx, v, func);
            }
            return v;
        } else if (ArraySplat *v = dynamic_cast<ArraySplat *>(what)) {
            if (HAS_MEMBER_preTransformArraySplat<FUNC>::value) {
                v = PostPonePreTransform_ArraySplat<FUNC, HAS_MEMBER_preTransformArraySplat<FUNC>::value>::call(ctx, v,
                                                                                                                func);
            }
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg, ctx);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expression *>(narg));
            }

            if (HAS_MEMBER_postTransformArraySplat<FUNC>::value) {
                return PostPonePostTransform_ArraySplat<FUNC, HAS_MEMBER_postTransformArraySplat<FUNC>::value>::call(
                    ctx, v, func);
            }

            return v;
        } else if (HashSplat *v = dynamic_cast<HashSplat *>(what)) {
            if (HAS_MEMBER_preTransformHashSplat<FUNC>::value) {
                v = PostPonePreTransform_HashSplat<FUNC, HAS_MEMBER_preTransformHashSplat<FUNC>::value>::call(ctx, v,
                                                                                                              func);
            }
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg, ctx);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expression *>(narg));
            }

            if (HAS_MEMBER_postTransformHashSplat<FUNC>::value) {
                return PostPonePostTransform_HashSplat<FUNC, HAS_MEMBER_postTransformHashSplat<FUNC>::value>::call(
                    ctx, v, func);
            }
            return v;
        } else if (Self *v = dynamic_cast<Self *>(what)) {
            if (HAS_MEMBER_postTransformSelf<FUNC>::value) {
                return PostPonePostTransform_Self<FUNC, HAS_MEMBER_postTransformSelf<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (Block *v = dynamic_cast<Block *>(what)) {
            Error::notImplemented();
        } else if (InsSeq *v = dynamic_cast<InsSeq *>(what)) {
            if (HAS_MEMBER_preTransformInsSeq<FUNC>::value) {
                v = PostPonePreTransform_InsSeq<FUNC, HAS_MEMBER_preTransformInsSeq<FUNC>::value>::call(ctx, v, func);
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
                Error::check(dynamic_cast<Expression *>(nexpr) != nullptr);
                v->expr.reset(dynamic_cast<Expression *>(nexpr));
            }

            if (HAS_MEMBER_postTransformInsSeq<FUNC>::value) {
                return PostPonePostTransform_InsSeq<FUNC, HAS_MEMBER_postTransformInsSeq<FUNC>::value>::call(ctx, v,
                                                                                                             func);
            }

            return v;
        } else {
            Error::raise("should never happen. Forgot to add new tree kind?");
        }
    }

public:
    static unique_ptr<Statement> apply(Context ctx, FUNC &func, unique_ptr<Statement> to) {
        Statement *underlying = to.get();
        TreeMap walker(func);
        Statement *res = walker.mapIt(underlying, ctx);

        if (res == underlying) {
            return to;
        } else {
            return std::unique_ptr<Statement>(res);
        }
    }
};

} // namespace ast
} // namespace ruby_typer

#endif // SRUBY_TREEMAP_H
