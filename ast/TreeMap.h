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
    ClassDef *transformClassDef(ClassDef *original);

    MethodDef *transformMethodDef(MethodDef *original);

    If *transformIf(If *original);

    While *transformWhile(While *original);

    For *transformFor(For *original);

    Break *transformBreak(Break *original);

    Next *transformNext(Next *original);

    Return *transformReturn(Return *original);

    Rescue *transformRescue(Rescue *original);

    Ident *transformIdent(Ident *original);

    Assign *transformAssign(Assign *original);

    Send *transformSend(Send *original);

    New *transformNew(New *original);

    NamedArg *transformNamedArg(NamedArg *original);

    Hash *transformHash(Hash *original);

    Array *transformArray(Array *original);

    FloatLit *transformFloatLit(FloatLit *original);

    IntLit *transformIntLit(IntLit *original);

    StringLit *transformStringLit(StringLit *original);

    ConstantLit *transformConstantLit(ConstantLit *original);

    ArraySplat *transformArraySplat(ArraySplat *original);

    HashSplat *transformHashSplat(HashSplat *original);

    Self *transformSelf(Self *original);

    Block *transformBlock(Block *original);

    InsSeq *transformInsSeq(InsSeq *original);
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

GENERATE_HAS_MEMBER(transformClassDef);
GENERATE_HAS_MEMBER(transformMethodDef);
GENERATE_HAS_MEMBER(transformConstDef);
GENERATE_HAS_MEMBER(transformIf);
GENERATE_HAS_MEMBER(transformWhile);
GENERATE_HAS_MEMBER(transformFor);
GENERATE_HAS_MEMBER(transformBreak);
GENERATE_HAS_MEMBER(transformNext);
GENERATE_HAS_MEMBER(transformReturn);
GENERATE_HAS_MEMBER(transformRescue);
GENERATE_HAS_MEMBER(transformIdent);
GENERATE_HAS_MEMBER(transformAssign);
GENERATE_HAS_MEMBER(transformSend);
GENERATE_HAS_MEMBER(transformNew);
GENERATE_HAS_MEMBER(transformNamedArg);
GENERATE_HAS_MEMBER(transformHash);
GENERATE_HAS_MEMBER(transformArray);
GENERATE_HAS_MEMBER(transformFloatLit);
GENERATE_HAS_MEMBER(transformIntLit);
GENERATE_HAS_MEMBER(transformStringLit);
GENERATE_HAS_MEMBER(transformConstantLit);
GENERATE_HAS_MEMBER(transformArraySplat);
GENERATE_HAS_MEMBER(transformHashSplat);
GENERATE_HAS_MEMBER(transformSelf);
GENERATE_HAS_MEMBER(transformBlock);
GENERATE_HAS_MEMBER(transformInsSeq);

#define GENERATE_POSTPONE_CLASS(X)                                             \
                                                                               \
    template <class FUNC, bool has> class PostPoneCalling_##X {                \
    public:                                                                    \
        static X *call(Context ctx, X *cd, FUNC &what) {                       \
            Error::raise("should never be called. Incorrect use of TreeMap?"); \
            return nullptr;                                                    \
        }                                                                      \
    };                                                                         \
                                                                               \
    template <class FUNC> class PostPoneCalling_##X<FUNC, true> {              \
    public:                                                                    \
        static X *call(Context ctx, X *cd, FUNC &func) {                       \
            return func.transform##X(ctx, cd);                                 \
        }                                                                      \
    };                                                                         \
                                                                               \
    template <class FUNC> class PostPoneCalling_##X<FUNC, false> {             \
    public:                                                                    \
        static X *call(Context ctx, X *cd, FUNC &func) {                       \
            return cd;                                                         \
        }                                                                      \
    };

GENERATE_POSTPONE_CLASS(ClassDef);
GENERATE_POSTPONE_CLASS(MethodDef);
GENERATE_POSTPONE_CLASS(ConstDef);
GENERATE_POSTPONE_CLASS(If);
GENERATE_POSTPONE_CLASS(While);
GENERATE_POSTPONE_CLASS(For);
GENERATE_POSTPONE_CLASS(Break);
GENERATE_POSTPONE_CLASS(Next);
GENERATE_POSTPONE_CLASS(Return);
GENERATE_POSTPONE_CLASS(Ident);
GENERATE_POSTPONE_CLASS(Assign);
GENERATE_POSTPONE_CLASS(Send);
GENERATE_POSTPONE_CLASS(New);
GENERATE_POSTPONE_CLASS(NamedArg);
GENERATE_POSTPONE_CLASS(Hash);
GENERATE_POSTPONE_CLASS(Array);
GENERATE_POSTPONE_CLASS(FloatLit);
GENERATE_POSTPONE_CLASS(IntLit);
GENERATE_POSTPONE_CLASS(StringLit);
GENERATE_POSTPONE_CLASS(ConstantLit);
GENERATE_POSTPONE_CLASS(ArraySplat);
GENERATE_POSTPONE_CLASS(HashSplat);
GENERATE_POSTPONE_CLASS(Self);
GENERATE_POSTPONE_CLASS(Block);
GENERATE_POSTPONE_CLASS(InsSeq);

/**
 * Given a tree transformer FUNC transform a tree.
 * Tree is guaranteed to be visited in the definition order.
 * FUNC may maintain internal state.
 * @tparam tree transformer, see FUNC_EXAMPLE
 */
template <class FUNC> class PreOrderTreeMap {
private:
    FUNC &func;

    PreOrderTreeMap(FUNC &func) : func(func) {}

    Statement *mapIt(Statement *what, Context ctx) {
        // TODO: reorder by frequency
        if (what == nullptr || dynamic_cast<EmptyTree *>(what) != nullptr)
            return what;
        if (ClassDef *v = dynamic_cast<ClassDef *>(what)) {

            if (HAS_MEMBER_transformClassDef<FUNC>::value) {
                v = PostPoneCalling_ClassDef<FUNC, HAS_MEMBER_transformClassDef<FUNC>::value>::call(ctx, v, func);
            }

            for (auto &def : v->rhs) {
                auto originalDef = def.get();
                auto newDef = mapIt(originalDef, ctx.withOwner(v->symbol));
                if (newDef != originalDef) {
                    def.reset(newDef);
                }
            }

            return v;
        } else if (MethodDef *v = dynamic_cast<MethodDef *>(what)) {
            if (HAS_MEMBER_transformMethodDef<FUNC>::value) {
                v = PostPoneCalling_MethodDef<FUNC, HAS_MEMBER_transformMethodDef<FUNC>::value>::call(ctx, v, func);
            }
            auto originalRhs = v->rhs.get();
            auto newRhs = mapIt(originalRhs, ctx.withOwner(v->symbol));
            if (newRhs != originalRhs) {
                Error::check(dynamic_cast<Expression *>(newRhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expression *>(newRhs));
            }
            return v;
        } else if (ConstDef *v = dynamic_cast<ConstDef *>(what)) {
            if (HAS_MEMBER_transformConstDef<FUNC>::value) {
                v = PostPoneCalling_ConstDef<FUNC, HAS_MEMBER_transformConstDef<FUNC>::value>::call(ctx, v, func);
            }
            auto originalRhs = v->rhs.get();
            auto newRhs = mapIt(originalRhs, ctx.withOwner(v->symbol));
            if (newRhs != originalRhs) {
                Error::check(dynamic_cast<Expression *>(newRhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expression *>(newRhs));
            }
            return v;
        } else if (If *v = dynamic_cast<If *>(what)) {
            if (HAS_MEMBER_transformIf<FUNC>::value) {
                v = PostPoneCalling_If<FUNC, HAS_MEMBER_transformIf<FUNC>::value>::call(ctx, v, func);
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
            return v;
        } else if (While *v = dynamic_cast<While *>(what)) {
            if (HAS_MEMBER_transformWhile<FUNC>::value) {
                v = PostPoneCalling_While<FUNC, HAS_MEMBER_transformWhile<FUNC>::value>::call(ctx, v, func);
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

            return v;
        } else if (For *v = dynamic_cast<For *>(what)) {
            Error::notImplemented();
        } else if (Break *v = dynamic_cast<Break *>(what)) {
            if (HAS_MEMBER_transformBreak<FUNC>::value) {
                return PostPoneCalling_Break<FUNC, HAS_MEMBER_transformBreak<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (Next *v = dynamic_cast<Next *>(what)) {
            if (HAS_MEMBER_transformNext<FUNC>::value) {
                return PostPoneCalling_Next<FUNC, HAS_MEMBER_transformNext<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (Return *v = dynamic_cast<Return *>(what)) {
            if (HAS_MEMBER_transformReturn<FUNC>::value) {
                v = PostPoneCalling_Return<FUNC, HAS_MEMBER_transformReturn<FUNC>::value>::call(ctx, v, func);
            }
            auto oexpr = v->expr.get();
            auto nexpr = mapIt(oexpr, ctx);
            if (oexpr != nexpr) {
                Error::check(dynamic_cast<Expression *>(nexpr) != nullptr);
                v->expr.reset(dynamic_cast<Expression *>(nexpr));
            }

            return v;
        } else if (Ident *v = dynamic_cast<Ident *>(what)) {
            if (HAS_MEMBER_transformIdent<FUNC>::value) {
                return PostPoneCalling_Ident<FUNC, HAS_MEMBER_transformIdent<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (Assign *v = dynamic_cast<Assign *>(what)) {
            if (HAS_MEMBER_transformAssign<FUNC>::value) {
                v = PostPoneCalling_Assign<FUNC, HAS_MEMBER_transformAssign<FUNC>::value>::call(ctx, v, func);
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

            return v;
        } else if (Send *v = dynamic_cast<Send *>(what)) {
            if (HAS_MEMBER_transformSend<FUNC>::value) {
                v = PostPoneCalling_Send<FUNC, HAS_MEMBER_transformSend<FUNC>::value>::call(ctx, v, func);
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

            return v;
        } else if (New *v = dynamic_cast<New *>(what)) {
            if (HAS_MEMBER_transformNew<FUNC>::value) {
                v = PostPoneCalling_New<FUNC, HAS_MEMBER_transformNew<FUNC>::value>::call(ctx, v, func);
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

            return v;
        } else if (NamedArg *v = dynamic_cast<NamedArg *>(what)) {
            if (HAS_MEMBER_transformNamedArg<FUNC>::value) {
                v = PostPoneCalling_NamedArg<FUNC, HAS_MEMBER_transformNamedArg<FUNC>::value>::call(ctx, v, func);
            }
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg, ctx);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expression *>(narg));
            }

            return v;
        } else if (Hash *v = dynamic_cast<Hash *>(what)) {
            Error::notImplemented();
        } else if (Array *v = dynamic_cast<Array *>(what)) {
            Error::notImplemented();
        } else if (FloatLit *v = dynamic_cast<FloatLit *>(what)) {
            if (HAS_MEMBER_transformFloatLit<FUNC>::value) {
                return PostPoneCalling_FloatLit<FUNC, HAS_MEMBER_transformFloatLit<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (IntLit *v = dynamic_cast<IntLit *>(what)) {
            if (HAS_MEMBER_transformIntLit<FUNC>::value) {
                return PostPoneCalling_IntLit<FUNC, HAS_MEMBER_transformIntLit<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (StringLit *v = dynamic_cast<StringLit *>(what)) {
            if (HAS_MEMBER_transformStringLit<FUNC>::value) {
                return PostPoneCalling_StringLit<FUNC, HAS_MEMBER_transformStringLit<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (ConstantLit *v = dynamic_cast<ConstantLit *>(what)) {
            if (HAS_MEMBER_transformConstantLit<FUNC>::value) {
                return PostPoneCalling_ConstantLit<FUNC, HAS_MEMBER_transformConstantLit<FUNC>::value>::call(ctx, v,
                                                                                                             func);
            }
            return v;
        } else if (ArraySplat *v = dynamic_cast<ArraySplat *>(what)) {
            if (HAS_MEMBER_transformArraySplat<FUNC>::value) {
                v = PostPoneCalling_ArraySplat<FUNC, HAS_MEMBER_transformArraySplat<FUNC>::value>::call(ctx, v, func);
            }
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg, ctx);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expression *>(narg));
            }

            return v;
        } else if (HashSplat *v = dynamic_cast<HashSplat *>(what)) {
            if (HAS_MEMBER_transformHashSplat<FUNC>::value) {
                v = PostPoneCalling_HashSplat<FUNC, HAS_MEMBER_transformHashSplat<FUNC>::value>::call(ctx, v, func);
            }
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg, ctx);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expression *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expression *>(narg));
            }
            return v;
        } else if (Self *v = dynamic_cast<Self *>(what)) {
            if (HAS_MEMBER_transformSelf<FUNC>::value) {
                return PostPoneCalling_Self<FUNC, HAS_MEMBER_transformSelf<FUNC>::value>::call(ctx, v, func);
            }
            return v;
        } else if (Block *v = dynamic_cast<Block *>(what)) {
            Error::notImplemented();
        } else if (InsSeq *v = dynamic_cast<InsSeq *>(what)) {
            if (HAS_MEMBER_transformInsSeq<FUNC>::value) {
                v = PostPoneCalling_InsSeq<FUNC, HAS_MEMBER_transformInsSeq<FUNC>::value>::call(ctx, v, func);
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

            return v;
        } else {
            Error::raise("should never happen. Forgot to add new tree kind?");
        }
    }

public:
    static unique_ptr<Statement> apply(Context ctx, FUNC &func, unique_ptr<Statement> to) {
        Statement *underlying = to.get();
        PreOrderTreeMap walker(func);
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
