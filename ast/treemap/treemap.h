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

    ExpressionPtr postTransformRuntimeMethodDefinition(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformBlock(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformBlock(core::MutableContext ctx, ExpressionPtr original);

    ExpressionPtr preTransformInsSeq(core::MutableContext ctx, ExpressionPtr original);
    ExpressionPtr postTransformInsSeq(core::MutableContext ctx, ExpressionPtr original);
};

// NOTE: Implementations must use a context type parameter that `MutableContext` is convertable to.
// That is, either `Context` or `MutableContext`.
#define GENERATE_HAS_MEMBER_VISITOR(X, arg_types...) GENERATE_HAS_MEMBER(X, arg_types)

// used to check for ABSENCE of method

#define GENERATE_POSTPONE_PRECLASS(X, arg_types...)                                                              \
    GENERATE_CALL_MEMBER(preTransform##X, Exception::raise("should never be called. Incorrect use of TreeMap?"); \
                         return nullptr, arg_types)

#define GENERATE_POSTPONE_POSTCLASS(X, arg_types...)                                                              \
    GENERATE_CALL_MEMBER(postTransform##X, Exception::raise("should never be called. Incorrect use of TreeMap?"); \
                         return nullptr, arg_types)

#define GENERATE_METAPROGRAMMING_FOR(arg_types...)                               \
    GENERATE_HAS_MEMBER_VISITOR(preTransformUnresolvedIdent, arg_types);         \
    GENERATE_HAS_MEMBER_VISITOR(preTransformLocal, arg_types);                   \
    GENERATE_HAS_MEMBER_VISITOR(preTransformUnresolvedConstantLit, arg_types);   \
    GENERATE_HAS_MEMBER_VISITOR(preTransformConstantLit, arg_types);             \
    GENERATE_HAS_MEMBER_VISITOR(preTransformLiteral, arg_types);                 \
    GENERATE_HAS_MEMBER_VISITOR(preTransformRuntimeMethodDefinition, arg_types); \
                                                                                 \
    GENERATE_POSTPONE_PRECLASS(Expression, arg_types);                           \
    GENERATE_POSTPONE_PRECLASS(ClassDef, arg_types);                             \
    GENERATE_POSTPONE_PRECLASS(MethodDef, arg_types);                            \
    GENERATE_POSTPONE_PRECLASS(If, arg_types);                                   \
    GENERATE_POSTPONE_PRECLASS(While, arg_types);                                \
    GENERATE_POSTPONE_PRECLASS(Break, arg_types);                                \
    GENERATE_POSTPONE_PRECLASS(Retry, arg_types);                                \
    GENERATE_POSTPONE_PRECLASS(Next, arg_types);                                 \
    GENERATE_POSTPONE_PRECLASS(Return, arg_types);                               \
    GENERATE_POSTPONE_PRECLASS(RescueCase, arg_types);                           \
    GENERATE_POSTPONE_PRECLASS(Rescue, arg_types);                               \
    GENERATE_POSTPONE_PRECLASS(Assign, arg_types);                               \
    GENERATE_POSTPONE_PRECLASS(Send, arg_types);                                 \
    GENERATE_POSTPONE_PRECLASS(Hash, arg_types);                                 \
    GENERATE_POSTPONE_PRECLASS(Array, arg_types);                                \
    GENERATE_POSTPONE_PRECLASS(Block, arg_types);                                \
    GENERATE_POSTPONE_PRECLASS(InsSeq, arg_types);                               \
    GENERATE_POSTPONE_PRECLASS(Cast, arg_types);                                 \
                                                                                 \
    GENERATE_POSTPONE_POSTCLASS(ClassDef, arg_types);                            \
    GENERATE_POSTPONE_POSTCLASS(MethodDef, arg_types);                           \
    GENERATE_POSTPONE_POSTCLASS(If, arg_types);                                  \
    GENERATE_POSTPONE_POSTCLASS(While, arg_types);                               \
    GENERATE_POSTPONE_POSTCLASS(Break, arg_types);                               \
    GENERATE_POSTPONE_POSTCLASS(Retry, arg_types);                               \
    GENERATE_POSTPONE_POSTCLASS(Next, arg_types);                                \
    GENERATE_POSTPONE_POSTCLASS(Return, arg_types);                              \
    GENERATE_POSTPONE_POSTCLASS(RescueCase, arg_types);                          \
    GENERATE_POSTPONE_POSTCLASS(Rescue, arg_types);                              \
    GENERATE_POSTPONE_POSTCLASS(UnresolvedIdent, arg_types);                     \
    GENERATE_POSTPONE_POSTCLASS(Assign, arg_types);                              \
    GENERATE_POSTPONE_POSTCLASS(Send, arg_types);                                \
    GENERATE_POSTPONE_POSTCLASS(Hash, arg_types);                                \
    GENERATE_POSTPONE_POSTCLASS(Array, arg_types);                               \
    GENERATE_POSTPONE_POSTCLASS(Local, arg_types);                               \
    GENERATE_POSTPONE_POSTCLASS(Literal, arg_types);                             \
    GENERATE_POSTPONE_POSTCLASS(UnresolvedConstantLit, arg_types);               \
    GENERATE_POSTPONE_POSTCLASS(ConstantLit, arg_types);                         \
    GENERATE_POSTPONE_POSTCLASS(Block, arg_types);                               \
    GENERATE_POSTPONE_POSTCLASS(InsSeq, arg_types);                              \
    GENERATE_POSTPONE_POSTCLASS(Cast, arg_types);                                \
    GENERATE_POSTPONE_POSTCLASS(RuntimeMethodDefinition, arg_types);

// Used to indicate that TreeMap has already reported location for this exception
struct ReportedRubyException {
    SorbetException reported;
    core::LocOffsets onLoc;
};

enum class TreeMapKind {
    Map,
    Walk,
};

template <TreeMapKind> struct MapFunctions;

template <> struct MapFunctions<TreeMapKind::Map> {
    using return_type = ExpressionPtr;
    using arg_type = ExpressionPtr;
    static ExpressionPtr &&pass(ExpressionPtr &p) {
        return static_cast<ExpressionPtr &&>(p);
    }
    GENERATE_METAPROGRAMMING_FOR(std::declval<core::MutableContext>(), std::declval<ExpressionPtr>());
};

template <> struct MapFunctions<TreeMapKind::Walk> {
    using return_type = void;
    using arg_type = ExpressionPtr &;
    static ExpressionPtr &pass(ExpressionPtr &p) {
        return p;
    }
    GENERATE_METAPROGRAMMING_FOR(std::declval<core::MutableContext>(), std::declval<ExpressionPtr &>());
};

enum class TreeMapDepthKind {
    Full,
    Shallow,
};

/**
 * Given a tree transformer FUNC transform a tree.
 * Tree is guaranteed to be visited in the definition order.
 * FUNC may maintain internal state.
 * @tparam tree transformer, see FUNC_EXAMPLE
 */
template <class FUNC, class CTX, TreeMapKind Kind, TreeMapDepthKind DepthKind> class TreeMapper {
private:
    friend class TreeMap;
    friend class ShallowMap;
    friend class TreeWalk;
    friend class ShallowWalk;

    using Funcs = MapFunctions<Kind>;
    using return_type = typename Funcs::return_type;
    using arg_type = typename Funcs::arg_type;

    FUNC &func;

    static_assert(!Funcs::template HAS_MEMBER_preTransformUnresolvedIdent<FUNC>(), "use post*Transform instead");
    static_assert(!Funcs::template HAS_MEMBER_preTransformLiteral<FUNC>(), "use post*Transform instead");
    static_assert(!Funcs::template HAS_MEMBER_preTransformUnresolvedConstantLit<FUNC>(), "use post*Transform instead");
    static_assert(!Funcs::template HAS_MEMBER_preTransformConstantLit<FUNC>(), "use post*Transform instead");
    static_assert(!Funcs::template HAS_MEMBER_preTransformLocal<FUNC>(), "use post*Transform instead");
    static_assert(!Funcs::template HAS_MEMBER_preTransformRuntimeMethodDefinition<FUNC>(),
                  "use post*Transform instead");

    TreeMapper(FUNC &func) : func(func) {}

#define CALL_PRE(member)                                                                                 \
    if constexpr (Funcs::template HAS_MEMBER_preTransform##member<FUNC>()) {                             \
        if constexpr (Kind == TreeMapKind::Map) {                                                        \
            v = Funcs::template CALL_MEMBER_preTransform##member<FUNC>::call(func, ctx, Funcs::pass(v)); \
        } else if (Kind == TreeMapKind::Walk) {                                                          \
            Funcs::template CALL_MEMBER_preTransform##member<FUNC>::call(func, ctx, Funcs::pass(v));     \
        }                                                                                                \
    }

#define CALL_POST(member)                                                                                    \
    if constexpr (Kind == TreeMapKind::Map) {                                                                \
        if constexpr (Funcs::template HAS_MEMBER_postTransform##member<FUNC>()) {                            \
            return Funcs::template CALL_MEMBER_postTransform##member<FUNC>::call(func, ctx, Funcs::pass(v)); \
        }                                                                                                    \
        return v;                                                                                            \
    } else if constexpr (Kind == TreeMapKind::Walk) {                                                        \
        if constexpr (Funcs::template HAS_MEMBER_postTransform##member<FUNC>()) {                            \
            Funcs::template CALL_MEMBER_postTransform##member<FUNC>::call(func, ctx, Funcs::pass(v));        \
        }                                                                                                    \
    }

#define CALL_MAP(tree, ctx)                           \
    if constexpr (Kind == TreeMapKind::Map) {         \
        tree = mapIt(Funcs::pass(tree), ctx);         \
    } else if constexpr (Kind == TreeMapKind::Walk) { \
        mapIt(Funcs::pass(tree), ctx);                \
    }

    return_type mapClassDef(arg_type v, CTX ctx) {
        CALL_PRE(ClassDef);

        // We intentionally do not walk v->ancestors nor v->singletonAncestors.
        //
        // These lists used to be guaranteed to be simple trees (only constant literals) by desugar,
        // but that was later relaxed. In places where walking ancestors is required, instead define
        // your `preTransformClassDef` method to contain this:
        //
        //   for (auto &ancestor : klass.ancestors) {
        //       ancestor = ast::TreeMap::apply(ctx, *this, std::move(ancestor))
        //   }
        //
        // and that will have the same effect, without having to retroactively change all TreeMaps.

        for (auto &def : cast_tree_nonnull<ClassDef>(v).rhs) {
            CALL_MAP(def, ctx.withOwner(cast_tree_nonnull<ClassDef>(v).symbol).withFile(ctx.file));
        }

        CALL_POST(ClassDef);
    }

    return_type mapMethodDef(arg_type v, CTX ctx) {
        CALL_PRE(MethodDef);

        for (auto &arg : cast_tree_nonnull<MethodDef>(v).args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                CALL_MAP(optArg->default_, ctx.withOwner(cast_tree_nonnull<MethodDef>(v).symbol));
            }
        }

        if constexpr (DepthKind == TreeMapDepthKind::Full) {
            CALL_MAP(cast_tree_nonnull<MethodDef>(v).rhs,
                     ctx.withOwner(cast_tree_nonnull<MethodDef>(v).symbol).withFile(ctx.file));
        }

        CALL_POST(MethodDef);
    }

    return_type mapIf(arg_type v, CTX ctx) {
        CALL_PRE(If);

        CALL_MAP(cast_tree_nonnull<If>(v).cond, ctx);
        CALL_MAP(cast_tree_nonnull<If>(v).thenp, ctx);
        CALL_MAP(cast_tree_nonnull<If>(v).elsep, ctx);

        CALL_POST(If);
    }

    return_type mapWhile(arg_type v, CTX ctx) {
        CALL_PRE(While);

        CALL_MAP(cast_tree_nonnull<While>(v).cond, ctx);
        CALL_MAP(cast_tree_nonnull<While>(v).body, ctx);

        CALL_POST(While);
    }

    return_type mapBreak(arg_type v, CTX ctx) {
        CALL_PRE(Break);

        CALL_MAP(cast_tree_nonnull<Break>(v).expr, ctx);

        CALL_POST(Break);
    }
    return_type mapRetry(arg_type v, CTX ctx) {
        CALL_POST(Retry);
    }

    return_type mapNext(arg_type v, CTX ctx) {
        CALL_PRE(Next);

        CALL_MAP(cast_tree_nonnull<Next>(v).expr, ctx);

        CALL_POST(Next);
    }

    return_type mapReturn(arg_type v, CTX ctx) {
        CALL_PRE(Return);

        CALL_MAP(cast_tree_nonnull<Return>(v).expr, ctx);

        CALL_POST(Return);
    }

    return_type mapRescueCase(arg_type v, CTX ctx) {
        CALL_PRE(RescueCase);

        for (auto &el : cast_tree_nonnull<RescueCase>(v).exceptions) {
            CALL_MAP(el, ctx);
        }

        CALL_MAP(cast_tree_nonnull<RescueCase>(v).var, ctx);

        CALL_MAP(cast_tree_nonnull<RescueCase>(v).body, ctx);

        CALL_POST(RescueCase);
    }
    return_type mapRescue(arg_type v, CTX ctx) {
        CALL_PRE(Rescue);

        CALL_MAP(cast_tree_nonnull<Rescue>(v).body, ctx);

        for (auto &el : cast_tree_nonnull<Rescue>(v).rescueCases) {
            ENFORCE(isa_tree<RescueCase>(el), "invalid tree where rescue case was expected");
            if constexpr (Kind == TreeMapKind::Map) {
                el = mapRescueCase(Funcs::pass(el), ctx);
            } else if constexpr (Kind == TreeMapKind::Walk) {
                mapRescueCase(Funcs::pass(el), ctx);
            }
            ENFORCE(isa_tree<RescueCase>(el), "rescue case was mapped into non-rescue case");
        }

        CALL_MAP(cast_tree_nonnull<Rescue>(v).else_, ctx);
        CALL_MAP(cast_tree_nonnull<Rescue>(v).ensure, ctx);

        CALL_POST(Rescue);
    }

    return_type mapUnresolvedIdent(arg_type v, CTX ctx) {
        CALL_POST(UnresolvedIdent);
    }

    return_type mapAssign(arg_type v, CTX ctx) {
        CALL_PRE(Assign);

        CALL_MAP(cast_tree_nonnull<Assign>(v).lhs, ctx);
        CALL_MAP(cast_tree_nonnull<Assign>(v).rhs, ctx);

        CALL_POST(Assign);
    }

    return_type mapSend(arg_type v, CTX ctx) {
        CALL_PRE(Send);

        CALL_MAP(cast_tree_nonnull<Send>(v).recv, ctx);

        for (auto &arg : cast_tree_nonnull<Send>(v).nonBlockArgs()) {
            CALL_MAP(arg, ctx);
            ENFORCE(arg != nullptr);
        }

        if (auto *block = cast_tree_nonnull<Send>(v).rawBlock()) {
            CALL_MAP(*block, ctx);
            ENFORCE(cast_tree_nonnull<Send>(v).block() != nullptr, "block was mapped into not-a block");
        }

        CALL_POST(Send);
    }

    return_type mapHash(arg_type v, CTX ctx) {
        CALL_PRE(Hash);

        for (auto &key : cast_tree_nonnull<Hash>(v).keys) {
            CALL_MAP(key, ctx);
        }

        for (auto &value : cast_tree_nonnull<Hash>(v).values) {
            CALL_MAP(value, ctx);
        }

        CALL_POST(Hash);
    }

    return_type mapArray(arg_type v, CTX ctx) {
        CALL_PRE(Array);

        for (auto &elem : cast_tree_nonnull<Array>(v).elems) {
            CALL_MAP(elem, ctx);
        }

        CALL_POST(Array);
    }

    return_type mapLiteral(arg_type v, CTX ctx) {
        CALL_POST(Literal);
    }

    return_type mapUnresolvedConstantLit(arg_type v, CTX ctx) {
        CALL_POST(UnresolvedConstantLit);
    }

    return_type mapConstantLit(arg_type v, CTX ctx) {
        CALL_POST(ConstantLit);
    }

    return_type mapBlock(arg_type v, CTX ctx) {
        CALL_PRE(Block);

        for (auto &arg : cast_tree_nonnull<Block>(v).args) {
            // Only OptionalArgs have subexpressions within them.
            if (auto *optArg = cast_tree<OptionalArg>(arg)) {
                CALL_MAP(optArg->default_, ctx);
            }
        }
        CALL_MAP(cast_tree_nonnull<Block>(v).body, ctx);

        CALL_POST(Block);
    }

    return_type mapInsSeq(arg_type v, CTX ctx) {
        CALL_PRE(InsSeq);

        for (auto &stat : cast_tree_nonnull<InsSeq>(v).stats) {
            CALL_MAP(stat, ctx);
        }

        CALL_MAP(cast_tree_nonnull<InsSeq>(v).expr, ctx);

        CALL_POST(InsSeq);
    }

    return_type mapLocal(arg_type v, CTX ctx) {
        CALL_POST(Local);
    }

    return_type mapCast(arg_type v, CTX ctx) {
        CALL_PRE(Cast);

        CALL_MAP(cast_tree_nonnull<Cast>(v).arg, ctx);
        CALL_MAP(cast_tree_nonnull<Cast>(v).typeExpr, ctx);

        CALL_POST(Cast);
    }

    return_type mapRuntimeMethodDefinition(arg_type v, CTX ctx) {
        CALL_POST(RuntimeMethodDefinition);
    }

    return_type mapIt(arg_type what, CTX ctx) {
        if (what == nullptr) {
            if constexpr (Kind == TreeMapKind::Map) {
                return what;
            } else if constexpr (Kind == TreeMapKind::Walk) {
                return;
            }
        }

        try {
            // TODO: reorder by frequency
            if constexpr (Funcs::template HAS_MEMBER_preTransformExpression<FUNC>()) {
                if constexpr (Kind == TreeMapKind::Map) {
                    what = Funcs::template CALL_MEMBER_preTransformExpression<FUNC>::call(func, ctx, Funcs::pass(what));
                } else if constexpr (Kind == TreeMapKind::Walk) {
                    Funcs::template CALL_MEMBER_preTransformExpression<FUNC>::call(func, ctx, Funcs::pass(what));
                }
            }

            switch (what.tag()) {
                case Tag::EmptyTree:
                    if constexpr (Kind == TreeMapKind::Map) {
                        return what;
                    } else if constexpr (Kind == TreeMapKind::Walk) {
                        return;
                    }

                case Tag::Send:
                    return mapSend(Funcs::pass(what), ctx);

                case Tag::ClassDef:
                    return mapClassDef(Funcs::pass(what), ctx);

                case Tag::MethodDef:
                    return mapMethodDef(Funcs::pass(what), ctx);

                case Tag::If:
                    return mapIf(Funcs::pass(what), ctx);

                case Tag::While:
                    return mapWhile(Funcs::pass(what), ctx);

                case Tag::Break:
                    return mapBreak(Funcs::pass(what), ctx);

                case Tag::Retry:
                    return mapRetry(Funcs::pass(what), ctx);

                case Tag::Next:
                    return mapNext(Funcs::pass(what), ctx);

                case Tag::Return:
                    return mapReturn(Funcs::pass(what), ctx);

                case Tag::RescueCase:
                    Exception::raise("should never happen. Forgot to add new tree kind? {}", what.nodeName());
                    break;

                case Tag::Rescue:
                    return mapRescue(Funcs::pass(what), ctx);

                case Tag::Local:
                    return mapLocal(Funcs::pass(what), ctx);

                case Tag::UnresolvedIdent:
                    return mapUnresolvedIdent(Funcs::pass(what), ctx);

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
                    return mapAssign(Funcs::pass(what), ctx);

                case Tag::Cast:
                    return mapCast(Funcs::pass(what), ctx);

                case Tag::Hash:
                    return mapHash(Funcs::pass(what), ctx);

                case Tag::Array:
                    return mapArray(Funcs::pass(what), ctx);

                case Tag::Literal:
                    return mapLiteral(Funcs::pass(what), ctx);

                case Tag::UnresolvedConstantLit:
                    return mapUnresolvedConstantLit(Funcs::pass(what), ctx);

                case Tag::ConstantLit:
                    return mapConstantLit(Funcs::pass(what), ctx);

                case Tag::ZSuperArgs:
                    if constexpr (Kind == TreeMapKind::Map) {
                        return what;
                    } else if constexpr (Kind == TreeMapKind::Walk) {
                        return;
                    }

                case Tag::Block:
                    return mapBlock(Funcs::pass(what), ctx);

                case Tag::InsSeq:
                    return mapInsSeq(Funcs::pass(what), ctx);

                case Tag::RuntimeMethodDefinition:
                    return mapRuntimeMethodDefinition(Funcs::pass(what), ctx);
            }
        } catch (SorbetException &e) {
            auto loc = what.loc();

            Exception::failInFuzzer();

            throw ReportedRubyException{e, loc};
        }
    }

#undef CALL_PRE
#undef CALL_POST
#undef CALL_MAP
};

class TreeMap {
public:
    template <typename CTX, typename FUNC> static ExpressionPtr apply(CTX ctx, FUNC &func, ExpressionPtr to) {
        TreeMapper<FUNC, CTX, TreeMapKind::Map, TreeMapDepthKind::Full> walker(func);
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

class TreeWalk {
public:
    template <typename CTX, typename FUNC> static void apply(CTX ctx, FUNC &func, ExpressionPtr &to) {
        TreeMapper<FUNC, CTX, TreeMapKind::Walk, TreeMapDepthKind::Full> walker(func);
        try {
            walker.mapIt(to, ctx);
        } catch (ReportedRubyException &exception) {
            Exception::failInFuzzer();
            if (auto e = ctx.beginError(exception.onLoc, core::errors::Internal::InternalError)) {
                e.setHeader("Failed to process tree (backtrace is above)");
            }
            throw exception.reported;
        }
    }
};

class ShallowMap {
public:
    template <typename CTX, typename FUNC> static ExpressionPtr apply(CTX ctx, FUNC &func, ExpressionPtr to) {
        TreeMapper<FUNC, CTX, TreeMapKind::Map, TreeMapDepthKind::Shallow> walker(func);
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

class ShallowWalk {
public:
    template <typename CTX, typename FUNC> static void apply(CTX ctx, FUNC &func, ExpressionPtr &to) {
        TreeMapper<FUNC, CTX, TreeMapKind::Walk, TreeMapDepthKind::Shallow> walker(func);
        try {
            walker.mapIt(to, ctx);
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
