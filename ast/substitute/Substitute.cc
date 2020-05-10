#include "ast/substitute/substitute.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "common/typecase.h"
#include "core/GlobalSubstitution.h"
namespace sorbet::ast {

namespace {
class SubstWalk {
private:
    const core::GlobalSubstitution &subst;

    TreePtr substClassName(core::MutableContext ctx, TreePtr node) {
        auto *constLit = cast_tree<UnresolvedConstantLit>(node);
        if (constLit == nullptr) { // uncommon case. something is strange
            if (isa_tree<EmptyTree>(node)) {
                return node;
            }
            return TreeMap::apply(ctx, *this, std::move(node));
        }

        auto scope = substClassName(ctx, std::move(constLit->scope));
        auto cnst = subst.substitute(constLit->cnst);

        return make_tree<UnresolvedConstantLit>(constLit->loc, std::move(scope), cnst);
    }

    TreePtr substArg(core::MutableContext ctx, TreePtr argp) {
        Expression *arg = argp.get();
        while (arg != nullptr) {
            typecase(
                arg, [&](RestArg *rest) { arg = rest->expr.get(); }, [&](KeywordArg *kw) { arg = kw->expr.get(); },
                [&](OptionalArg *opt) { arg = opt->expr.get(); }, [&](BlockArg *opt) { arg = opt->expr.get(); },
                [&](ShadowArg *opt) { arg = opt->expr.get(); },
                [&](Local *local) {
                    local->localVariable._name = subst.substitute(local->localVariable._name);
                    arg = nullptr;
                },
                [&](UnresolvedIdent *nm) { Exception::raise("UnresolvedIdent remaining after local_vars"); });
        }
        return argp;
    }

public:
    SubstWalk(const core::GlobalSubstitution &subst) : subst(subst) {}

    TreePtr preTransformClassDef(core::MutableContext ctx, TreePtr tree) {
        auto *original = cast_tree<ClassDef>(tree);
        original->name = substClassName(ctx, std::move(original->name));
        for (auto &anc : original->ancestors) {
            anc = substClassName(ctx, std::move(anc));
        }
        return tree;
    }

    TreePtr preTransformMethodDef(core::MutableContext ctx, TreePtr tree) {
        auto *original = cast_tree<MethodDef>(tree);
        original->name = subst.substitute(original->name);
        for (auto &arg : original->args) {
            arg = substArg(ctx, std::move(arg));
        }
        return tree;
    }

    TreePtr preTransformBlock(core::MutableContext ctx, TreePtr tree) {
        auto *original = cast_tree<Block>(tree);
        for (auto &arg : original->args) {
            arg = substArg(ctx, std::move(arg));
        }
        return tree;
    }

    TreePtr postTransformUnresolvedIdent(core::MutableContext ctx, TreePtr original) {
        cast_tree<UnresolvedIdent>(original)->name = subst.substitute(cast_tree<UnresolvedIdent>(original)->name);
        return original;
    }

    TreePtr postTransformLocal(core::MutableContext ctx, TreePtr local) {
        cast_tree<Local>(local)->localVariable._name = subst.substitute(cast_tree<Local>(local)->localVariable._name);
        return local;
    }

    TreePtr preTransformSend(core::MutableContext ctx, TreePtr original) {
        cast_tree<Send>(original)->fun = subst.substitute(cast_tree<Send>(original)->fun);
        return original;
    }

    TreePtr postTransformLiteral(core::MutableContext ctx, TreePtr tree) {
        auto *original = cast_tree<Literal>(tree);
        if (original->isString(ctx)) {
            auto nameRef = original->asString(ctx);
            // The 'from' and 'to' GlobalState in this substitution will always be the same,
            // because the newly created nameRef reuses our current GlobalState id
            bool allowSameFromTo = true;
            auto newName = subst.substitute(nameRef, allowSameFromTo);
            if (newName == nameRef) {
                return tree;
            }
            return MK::String(original->loc, newName);
        }
        if (original->isSymbol(ctx)) {
            auto nameRef = original->asSymbol(ctx);
            // The 'from' and 'to' GlobalState in this substitution will always be the same,
            // because the newly created nameRef reuses our current GlobalState id
            bool allowSameFromTo = true;
            auto newName = subst.substitute(nameRef, allowSameFromTo);
            if (newName == nameRef) {
                return tree;
            }
            return MK::Symbol(original->loc, newName);
        }
        return tree;
    }

    TreePtr postTransformUnresolvedConstantLit(core::MutableContext ctx, TreePtr tree) {
        auto *original = cast_tree<UnresolvedConstantLit>(tree);
        original->cnst = subst.substitute(original->cnst);
        original->scope = substClassName(ctx, std::move(original->scope));
        return tree;
    }
};
} // namespace

TreePtr Substitute::run(core::MutableContext ctx, const core::GlobalSubstitution &subst, TreePtr what) {
    if (subst.useFastPath()) {
        return what;
    }
    SubstWalk walk(subst);
    what = TreeMap::apply(ctx, walk, std::move(what));
    return what;
}

} // namespace sorbet::ast
