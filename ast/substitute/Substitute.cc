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
        cast_tree_nonnull<Send>(original).fun = subst.substituteSymbolName(cast_tree_nonnull<Send>(original).fun);
    }

    void postTransformLiteral(core::MutableContext ctx, ExpressionPtr &tree) {
        auto &original = cast_tree_nonnull<Literal>(tree);
        if (original.isString()) {
            auto nameRef = original.asString();
            auto newName = subst.substitute(nameRef);
            if (nameRef != newName) {
                original.value = core::make_type<core::NamedLiteralType>(core::Symbols::String(), newName);
            }
            return;
        }
        if (original.isSymbol()) {
            auto nameRef = original.asSymbol();
            auto newName = subst.substitute(nameRef);
            if (newName != nameRef) {
                original.value = core::make_type<core::NamedLiteralType>(core::Symbols::Symbol(), newName);
            }
            return;
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
