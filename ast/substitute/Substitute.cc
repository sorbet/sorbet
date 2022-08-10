#include "ast/substitute/substitute.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "common/typecase.h"
#include "core/NameSubstitution.h"
namespace sorbet::ast {

namespace {
// Is used with NameSubstitution and LazyNameSubstitution, which implement the same interface.
template <typename T> class SubstWalk {
private:
    T &subst;

    void substClassName(core::MutableContext ctx, ExpressionPtr &node) {
        auto *constLit = cast_tree<UnresolvedConstantLit>(node);
        if (constLit == nullptr) { // uncommon case. something is strange
            if (isa_tree<EmptyTree>(node)) {
                return;
            }
            TreeWalk::apply(ctx, *this, node);
            return;
        }

        substClassName(ctx, constLit->scope);
        constLit->cnst = subst.substituteSymbolName(constLit->cnst);
    }

    void substArg(core::MutableContext ctx, ExpressionPtr &argp) {
        ExpressionPtr *arg = &argp;
        while (arg != nullptr) {
            typecase(
                *arg, [&](RestArg &rest) { arg = &rest.expr; }, [&](KeywordArg &kw) { arg = &kw.expr; },
                [&](OptionalArg &opt) { arg = &opt.expr; }, [&](BlockArg &opt) { arg = &opt.expr; },
                [&](ShadowArg &opt) { arg = &opt.expr; },
                [&](Local &local) {
                    local.localVariable._name = subst.substitute(local.localVariable._name);
                    arg = nullptr;
                },
                [&](const UnresolvedIdent &nm) { Exception::raise("UnresolvedIdent remaining after local_vars"); });
        }
    }

    core::NameRef unwrapLiteralToName(ExpressionPtr &arg) {
        auto *literal = cast_tree<Literal>(arg);
        if (literal == nullptr) {
            return core::NameRef::noName();
        }

        if (literal->isString()) {
            return literal->asString();
        } else if (literal->isSymbol()) {
            return literal->asSymbol();
        } else {
            return core::NameRef::noName();
        }
    }

public:
    SubstWalk(T &subst) : subst(subst) {}

    void preTransformClassDef(core::MutableContext ctx, ExpressionPtr &tree) {
        auto &original = cast_tree_nonnull<ClassDef>(tree);
        substClassName(ctx, original.name);
        for (auto &anc : original.ancestors) {
            substClassName(ctx, anc);
        }
    }

    void preTransformMethodDef(core::MutableContext ctx, ExpressionPtr &tree) {
        auto &original = cast_tree_nonnull<MethodDef>(tree);
        original.name = subst.substituteSymbolName(original.name);
        for (auto &arg : original.args) {
            substArg(ctx, arg);
        }
    }

    void preTransformBlock(core::MutableContext ctx, ExpressionPtr &tree) {
        auto &original = cast_tree_nonnull<Block>(tree);
        for (auto &arg : original.args) {
            substArg(ctx, arg);
        }
    }

    void postTransformUnresolvedIdent(core::MutableContext ctx, ExpressionPtr &original) {
        auto &id = cast_tree_nonnull<UnresolvedIdent>(original);
        if (id.kind != ast::UnresolvedIdent::Kind::Local) {
            id.name = subst.substituteSymbolName(cast_tree<UnresolvedIdent>(original)->name);
        } else {
            id.name = subst.substitute(cast_tree<UnresolvedIdent>(original)->name);
        }
    }

    void postTransformLocal(core::MutableContext ctx, ExpressionPtr &local) {
        cast_tree_nonnull<Local>(local).localVariable._name =
            subst.substitute(cast_tree_nonnull<Local>(local).localVariable._name);
    }

    void preTransformSend(core::MutableContext ctx, ExpressionPtr &original) {
        auto &send = cast_tree_nonnull<Send>(original);
        if (send.fun == core::Names::aliasMethod()) {
            // `alias_method :foo :bar` is very similar to `def foo; self.bar(); end`, so in
            // addition to handling the `alias_method` fun, we also want to look at the two
            // possibly-Symbol-literal arguments and call substituteSymbolName on them as well.

            // Discards the new name. We only care to do this for the side effect of recording the entry in
            // acc.symbols in the NameSubstitution. When the tree traversal actually visits the arg
            // nodes, they will get substituted like normal.
            if (send.numPosArgs() > 0) {
                auto name = unwrapLiteralToName(send.getPosArg(0));
                if (name.exists()) {
                    [[maybe_unused]] auto _substituted = subst.substituteSymbolName(name);
                }
            }
            if (send.numPosArgs() > 1) {
                auto name = unwrapLiteralToName(send.getPosArg(1));
                if (name.exists()) {
                    [[maybe_unused]] auto _substituted = subst.substituteSymbolName(name);
                }
            }
        } else if (send.fun == core::Names::callWithSplat() || send.fun == core::Names::callWithBlock() ||
                   send.fun == core::Names::callWithSplatAndBlock() || send.fun == core::Names::checkAndAnd()) {
            ENFORCE(send.numPosArgs() > 2, "These are special desugar methods, should have at least two args");

            // We're lucky because all of these methods have the symbol they dispatch to as the
            // positional arg at index 1.
            auto name = unwrapLiteralToName(send.getPosArg(1));
            ENFORCE(name.exists(), "Name should exist because this arg should be a Literal");
            [[maybe_unused]] auto _substituted = subst.substituteSymbolName(name);
        }

        send.fun = subst.substituteSymbolName(send.fun);

        // Some intrinsic method dispatch to other methods. Defensively assume that any method that
        // shares a name with an intrinsic might be an intrinsic method.
        for (const auto &[source, targets] : core::intrinsicMethodsDispatchMap()) {
            for (const auto target : targets) {
                [[maybe_unused]] auto _substituted = subst.substituteSymbolName(target);
            }
        }
    }

    void postTransformLiteral(core::MutableContext ctx, ExpressionPtr &tree) {
        auto &original = cast_tree_nonnull<Literal>(tree);
        auto nameRef = unwrapLiteralToName(tree);
        if (!nameRef.exists()) {
            return;
        }
        auto newName = subst.substitute(nameRef);

        if (newName == nameRef) {
            return;
        }

        if (original.isString()) {
            original.value = core::make_type<core::NamedLiteralType>(core::Symbols::String(), newName);
        } else if (original.isSymbol()) {
            original.value = core::make_type<core::NamedLiteralType>(core::Symbols::Symbol(), newName);
        } else {
            ENFORCE(false, "Should be guaranteed by unwrapLiteralToName");
        }
    }

    void postTransformUnresolvedConstantLit(core::MutableContext ctx, ExpressionPtr &tree) {
        auto &original = cast_tree_nonnull<UnresolvedConstantLit>(tree);
        original.cnst = subst.substituteSymbolName(original.cnst);
        substClassName(ctx, original.scope);
    }

    void postTransformRuntimeMethodDefinition(core::MutableContext ctx, ExpressionPtr &original) {
        auto &def = cast_tree_nonnull<RuntimeMethodDefinition>(original);
        def.name = subst.substitute(def.name);
    }
};
} // namespace

ParsedFile Substitute::run(core::MutableContext ctx, const core::NameSubstitution &subst, ParsedFile what) {
    SubstWalk walk(subst);
    TreeWalk::apply(ctx, walk, what.tree);
    return what;
}

ParsedFile Substitute::run(core::MutableContext ctx, core::LazyNameSubstitution &subst, ParsedFile what) {
    SubstWalk walk(subst);
    TreeWalk::apply(ctx, walk, what.tree);
    return what;
}

} // namespace sorbet::ast
