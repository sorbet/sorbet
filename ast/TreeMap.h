#ifndef SRUBY_TREEMAP_H
#define SRUBY_TREEMAP_H

#include "ast.h"
#include "memory"
#include <type_traits> // To use 'std::integral_constant'.

using std::unique_ptr;
using std::make_unique;

namespace sruby {
namespace ast {

class FUNC_EXAMPLE {
public:
    // all members are optional, but METHOD NAMES MATTER
    // Not including the member will skip the branch
    // you may return the same pointer that you are given
    // caller is repsonsible to handle it
    Stat *transformClassDef(ClassDef *original);

    Stat *transformMethodDef(MethodDef *original);

    Stat *transformSelfMethodDef(SelfMethodDef *original);

    Stat *transformSelfMethodDef(ConstDef *original);

    Stat *transformIf(If *original);

    Stat *transformWhile(While *original);

    Stat *transformFor(For *original);

    Stat *transformBreak(Break *original);

    Stat *transformNext(Next *original);

    Stat *transformReturn(Return *original);

    Stat *transformRescue(Rescue *original);

    Stat *transformIdent(Ident *original);

    Stat *transformAssign(Assign *original);

    Stat *transformSend(Send *original);

    Stat *transformNew(New *original);

    Stat *transformNamedArg(NamedArg *original);

    Stat *transformHash(Hash *original);

    Stat *transformArray(Array *original);

    Stat *transformFloatLit(FloatLit *original);

    Stat *transformIntLit(IntLit *original);

    Stat *transformStringLit(StringLit *original);

    Stat *transformConstantLit(ConstantLit *original);

    Stat *transformArraySplat(ArraySplat *original);

    Stat *transformHashSplat(HashSplat *original);

    Stat *transformSelf(Self *original);

    Stat *transformClosure(Closure *original);

    Stat *transformInsSeq(InsSeq *original);
};

/**
 * GENERATE_HAS_MEMBER(att)  // Creates 'has_member_att'.
 * `HAS_MEMBER_att<C>::value` can be used to statically test if C has a member att
 */
#define GENERATE_HAS_MEMBER(X)                                                                                         \
    template <typename T> class HAS_MEMBER_##X {                                                                       \
        struct Fallback {                                                                                              \
            int X;                                                                                                     \
        };                                                                                                             \
        struct Derived : T, Fallback {};                                                                               \
                                                                                                                       \
        template <typename U, U> struct Check;                                                                         \
                                                                                                                       \
        typedef char ArrayOfOne[1];                                                                                    \
        typedef char ArrayOfTwo[2];                                                                                    \
                                                                                                                       \
        template <typename U> static ArrayOfOne &func(Check<int Fallback::*, &U::X> *);                                \
        template <typename U> static ArrayOfTwo &func(...);                                                            \
                                                                                                                       \
    public:                                                                                                            \
        typedef HAS_MEMBER_##X type;                                                                                   \
        enum { value = sizeof(func<Derived>(0)) == 2 };                                                                \
    };

GENERATE_HAS_MEMBER(transformClassDef);
GENERATE_HAS_MEMBER(transformMethodDef);
GENERATE_HAS_MEMBER(transformSelfMethodDef);
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
GENERATE_HAS_MEMBER(transformClosure);
GENERATE_HAS_MEMBER(transformInsSeq);

#define GENERATE_POSTPONE_CLASS(X)                                                                                     \
                                                                                                                       \
    template <class FUNC, bool has> class PostPoneCalling_##X {                                                        \
    public:                                                                                                            \
        static Stat *call(X *cd, FUNC &what) {                                                                         \
            Error::raise("should never be called. Incorrect use of TreeMap?");                                         \
            return nullptr;                                                                                            \
        }                                                                                                              \
    };                                                                                                                 \
                                                                                                                       \
    template <class FUNC> class PostPoneCalling_##X<FUNC, true> {                                                      \
    public:                                                                                                            \
        static Stat *call(X *cd, FUNC &func) {                                                                         \
            return func.transform##X(cd);                                                                              \
        }                                                                                                              \
    };                                                                                                                 \
                                                                                                                       \
    template <class FUNC> class PostPoneCalling_##X<FUNC, false> {                                                     \
    public:                                                                                                            \
        static Stat *call(X *cd, FUNC &func) {                                                                         \
            return cd;                                                                                                 \
        }                                                                                                              \
    };

GENERATE_POSTPONE_CLASS(ClassDef);
GENERATE_POSTPONE_CLASS(MethodDef);
GENERATE_POSTPONE_CLASS(SelfMethodDef);
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
GENERATE_POSTPONE_CLASS(Closure);
GENERATE_POSTPONE_CLASS(InsSeq);

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

    Stat *mapIt(Stat *what) {
        // TODO: reorder by frequency
        if (what == nullptr || dynamic_cast<EmptyTree *>(what) != nullptr)
            return what;
        if (ClassDef *v = dynamic_cast<ClassDef *>(what)) {
            auto orhs = v->rhs.get();
            auto nrhs = mapIt(orhs);
            if (nrhs != orhs) {
                v->rhs.reset(nrhs);
            }
            if (HAS_MEMBER_transformClassDef<FUNC>::value) {
                return PostPoneCalling_ClassDef<FUNC, HAS_MEMBER_transformClassDef<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (MethodDef *v = dynamic_cast<MethodDef *>(what)) {
            auto orhs = v->rhs.get();
            auto nrhs = mapIt(orhs);
            if (nrhs != orhs) {
                Error::check(dynamic_cast<Expr *>(nrhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expr *>(nrhs));
            }
            if (HAS_MEMBER_transformMethodDef<FUNC>::value) {
                return PostPoneCalling_MethodDef<FUNC, HAS_MEMBER_transformMethodDef<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (SelfMethodDef *v = dynamic_cast<SelfMethodDef *>(what)) {
            auto orhs = v->rhs.get();
            auto nrhs = mapIt(orhs);
            if (nrhs != orhs) {
                Error::check(dynamic_cast<Expr *>(nrhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expr *>(nrhs));
            }
            if (HAS_MEMBER_transformSelfMethodDef<FUNC>::value) {
                return PostPoneCalling_SelfMethodDef<FUNC, HAS_MEMBER_transformSelfMethodDef<FUNC>::value>::call(v,
                                                                                                                 func);
            }
            return v;
        } else if (ConstDef *v = dynamic_cast<ConstDef *>(what)) {
            auto orhs = v->rhs.get();
            auto nrhs = mapIt(orhs);
            if (nrhs != orhs) {
                Error::check(dynamic_cast<Expr *>(nrhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expr *>(nrhs));
            }
            if (HAS_MEMBER_transformConstDef<FUNC>::value) {
                return PostPoneCalling_ConstDef<FUNC, HAS_MEMBER_transformConstDef<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (If *v = dynamic_cast<If *>(what)) {
            auto cond = v->cond.get();
            auto thenp = v->thenp.get();
            auto elsep = v->elsep.get();
            auto ncond = mapIt(cond);
            auto nthenp = mapIt(thenp);
            auto nelsep = mapIt(elsep);
            if (ncond != cond) {
                Error::check(dynamic_cast<Expr *>(ncond) != nullptr);
                v->cond.reset(dynamic_cast<Expr *>(ncond));
            }
            if (thenp != nthenp) {
                Error::check(dynamic_cast<Expr *>(nthenp) != nullptr);
                v->thenp.reset(dynamic_cast<Expr *>(nthenp));
            }
            if (elsep != nelsep) {
                Error::check(dynamic_cast<Expr *>(nelsep) != nullptr);
                v->elsep.reset(dynamic_cast<Expr *>(nelsep));
            }
            if (HAS_MEMBER_transformIf<FUNC>::value) {
                return PostPoneCalling_If<FUNC, HAS_MEMBER_transformIf<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (While *v = dynamic_cast<While *>(what)) {
            auto cond = v->cond.get();
            auto body = v->body.get();
            auto ncond = mapIt(cond);
            auto nbody = mapIt(body);
            if (ncond != cond) {
                Error::check(dynamic_cast<Expr *>(ncond) != nullptr);
                v->cond.reset(dynamic_cast<Expr *>(ncond));
            }
            if (nbody != body) {
                v->body.reset(nbody);
            }
            if (HAS_MEMBER_transformWhile<FUNC>::value) {
                return PostPoneCalling_While<FUNC, HAS_MEMBER_transformWhile<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (For *v = dynamic_cast<For *>(what)) {
            Error::notImplemented();
        } else if (Break *v = dynamic_cast<Break *>(what)) {
            if (HAS_MEMBER_transformBreak<FUNC>::value) {
                return PostPoneCalling_Break<FUNC, HAS_MEMBER_transformBreak<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (Next *v = dynamic_cast<Next *>(what)) {
            if (HAS_MEMBER_transformNext<FUNC>::value) {
                return PostPoneCalling_Next<FUNC, HAS_MEMBER_transformNext<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (Return *v = dynamic_cast<Return *>(what)) {
            auto oexpr = v->expr.get();
            auto nexpr = mapIt(oexpr);
            if (oexpr != nexpr) {
                Error::check(dynamic_cast<Expr *>(nexpr) != nullptr);
                v->expr.reset(dynamic_cast<Expr *>(nexpr));
            }
            if (HAS_MEMBER_transformReturn<FUNC>::value) {
                return PostPoneCalling_Return<FUNC, HAS_MEMBER_transformReturn<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (Ident *v = dynamic_cast<Ident *>(what)) {
            if (HAS_MEMBER_transformIdent<FUNC>::value) {
                return PostPoneCalling_Ident<FUNC, HAS_MEMBER_transformIdent<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (Assign *v = dynamic_cast<Assign *>(what)) {
            auto olhs = v->lhs.get();
            auto orhs = v->rhs.get();
            auto nlhs = mapIt(olhs);
            auto nrhs = mapIt(orhs);
            if (nlhs != olhs) {
                Error::check(dynamic_cast<Expr *>(nlhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expr *>(nlhs));
            }
            if (nrhs != orhs) {
                Error::check(dynamic_cast<Expr *>(nrhs) != nullptr);
                v->rhs.reset(dynamic_cast<Expr *>(nrhs));
            }
            if (HAS_MEMBER_transformAssign<FUNC>::value) {
                return PostPoneCalling_Assign<FUNC, HAS_MEMBER_transformAssign<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (Send *v = dynamic_cast<Send *>(what)) {
            auto orecv = v->recv.get();
            auto nrecv = mapIt(orecv);
            auto &vec = v->args;
            if (nrecv != orecv) {
                Error::check(dynamic_cast<Expr *>(nrecv) != nullptr);
                v->recv.reset(dynamic_cast<Expr *>(nrecv));
            }
            auto i = 0;
            while (i < vec.size()) {
                auto &el = vec[i];
                auto oarg = el.get();
                auto narg = mapIt(oarg);
                if (oarg != narg) {
                    Error::check(dynamic_cast<Expr *>(narg) != nullptr);
                    el.reset(dynamic_cast<Expr *>(narg));
                }
                i++;
            }

            if (HAS_MEMBER_transformSend<FUNC>::value) {
                return PostPoneCalling_Send<FUNC, HAS_MEMBER_transformSend<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (New *v = dynamic_cast<New *>(what)) {
            auto &args = v->args;
            auto i = 0;
            while (i < args.size()) {
                auto &el = args[i];
                auto oarg = el.get();
                auto narg = mapIt(oarg);
                if (oarg != narg) {
                    Error::check(dynamic_cast<Expr *>(narg) != nullptr);
                    el.reset(dynamic_cast<Expr *>(narg));
                }
                i++;
            }
            if (HAS_MEMBER_transformNew<FUNC>::value) {
                return PostPoneCalling_New<FUNC, HAS_MEMBER_transformNew<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (NamedArg *v = dynamic_cast<NamedArg *>(what)) {
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expr *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expr *>(narg));
            }
            if (HAS_MEMBER_transformNamedArg<FUNC>::value) {
                return PostPoneCalling_NamedArg<FUNC, HAS_MEMBER_transformNamedArg<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (Hash *v = dynamic_cast<Hash *>(what)) {
            Error::notImplemented();
        } else if (Array *v = dynamic_cast<Array *>(what)) {
            Error::notImplemented();
        } else if (FloatLit *v = dynamic_cast<FloatLit *>(what)) {
            if (HAS_MEMBER_transformFloatLit<FUNC>::value) {
                return PostPoneCalling_FloatLit<FUNC, HAS_MEMBER_transformFloatLit<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (IntLit *v = dynamic_cast<IntLit *>(what)) {
            if (HAS_MEMBER_transformIntLit<FUNC>::value) {
                return PostPoneCalling_IntLit<FUNC, HAS_MEMBER_transformIntLit<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (StringLit *v = dynamic_cast<StringLit *>(what)) {
            if (HAS_MEMBER_transformStringLit<FUNC>::value) {
                return PostPoneCalling_StringLit<FUNC, HAS_MEMBER_transformStringLit<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (ConstantLit *v = dynamic_cast<ConstantLit *>(what)) {
            if (HAS_MEMBER_transformConstantLit<FUNC>::value) {
                return PostPoneCalling_ConstantLit<FUNC, HAS_MEMBER_transformConstantLit<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (ArraySplat *v = dynamic_cast<ArraySplat *>(what)) {
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expr *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expr *>(narg));
            }
            if (HAS_MEMBER_transformArraySplat<FUNC>::value) {
                return PostPoneCalling_ArraySplat<FUNC, HAS_MEMBER_transformArraySplat<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (HashSplat *v = dynamic_cast<HashSplat *>(what)) {
            auto oarg = v->arg.get();
            auto narg = mapIt(oarg);
            if (oarg != narg) {
                Error::check(dynamic_cast<Expr *>(narg) != nullptr);
                v->arg.reset(dynamic_cast<Expr *>(narg));
            }
            if (HAS_MEMBER_transformHashSplat<FUNC>::value) {
                return PostPoneCalling_HashSplat<FUNC, HAS_MEMBER_transformHashSplat<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (Self *v = dynamic_cast<Self *>(what)) {
            if (HAS_MEMBER_transformSelf<FUNC>::value) {
                return PostPoneCalling_Self<FUNC, HAS_MEMBER_transformSelf<FUNC>::value>::call(v, func);
            }
            return v;
        } else if (Closure *v = dynamic_cast<Closure *>(what)) {
            Error::notImplemented();
        } else if (InsSeq *v = dynamic_cast<InsSeq *>(what)) {
            auto &stats = v->stats;
            auto oexpr = v->expr.get();
            auto i = 0;
            while (i < stats.size()) {
                auto &el = stats[i];
                auto oexp = el.get();
                auto nexp = mapIt(oexp);
                if (oexp != nexp) {
                    el.reset(nexp);
                }
                i++;
            }
            auto nexpr = mapIt(oexpr);
            if (nexpr != oexpr) {
                Error::check(dynamic_cast<Expr *>(nexpr) != nullptr);
                v->expr.reset(dynamic_cast<Expr *>(nexpr));
            }

            if (HAS_MEMBER_transformInsSeq<FUNC>::value) {
                return PostPoneCalling_InsSeq<FUNC, HAS_MEMBER_transformInsSeq<FUNC>::value>::call(v, func);
            }
            return v;
        } else {
            Error::raise("should never happen. Forgot to add new tree kind?");
        }
    }

public:
    static unique_ptr<Stat> apply(FUNC &func, unique_ptr<Stat> to) {
        Stat *underlying = to.get();
        TreeMap walker(func);
        Stat *res = walker.mapIt(underlying);

        if (res == underlying) {
            return to;
        } else {
            return std::unique_ptr<Stat>(res);
        }
    }
};

} // namespace ast
} // namespace sruby

#endif // SRUBY_TREEMAP_H
