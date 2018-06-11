#include "substitute.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"

using namespace sorbet;
using namespace sorbet::ast;

class SubstWalk {
private:
    const sorbet::core::GlobalSubstitution &subst;

    unique_ptr<ast::Expression> substClassName(core::MutableContext ctx, unique_ptr<ast::Expression> node) {
        auto constLit = ast::cast_tree<ast::ConstantLit>(node.get());
        if (constLit == nullptr) { // uncommon case. something is strange
            if (ast::isa_tree<ast::EmptyTree>(node.get())) {
                return node;
            }
            return TreeMap::apply(ctx, *this, move(node));
        }

        auto scope = substClassName(ctx, move(constLit->scope));
        auto cnst = subst.substitute(constLit->cnst);

        return make_unique<ast::ConstantLit>(constLit->loc, move(scope), cnst);
    }

    unique_ptr<ast::Expression> substArg(core::MutableContext ctx, unique_ptr<ast::Expression> argp) {
        ast::Expression *arg = argp.get();
        while (arg != nullptr) {
            typecase(arg, [&](ast::RestArg *rest) { arg = rest->expr.get(); },
                     [&](ast::KeywordArg *kw) { arg = kw->expr.get(); },
                     [&](ast::OptionalArg *opt) {
                         opt->default_ = TreeMap::apply(ctx, *this, move(opt->default_));
                         arg = opt->expr.get();
                     },
                     [&](ast::BlockArg *opt) { arg = opt->expr.get(); },
                     [&](ast::ShadowArg *opt) { arg = opt->expr.get(); },
                     [&](ast::UnresolvedIdent *nm) {
                         nm->name = subst.substitute(nm->name);
                         arg = nullptr;
                     });
        }
        return argp;
    }

public:
    SubstWalk(const sorbet::core::GlobalSubstitution &subst) : subst(subst) {}

    unique_ptr<ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ClassDef> original) {
        original->name = substClassName(ctx, move(original->name));
        for (auto &anc : original->ancestors) {
            anc = substClassName(ctx, move(anc));
        }
        return original;
    }
    unique_ptr<MethodDef> preTransformMethodDef(core::MutableContext ctx, unique_ptr<MethodDef> original) {
        original->name = subst.substitute(original->name);
        for (auto &arg : original->args) {
            arg = substArg(ctx, move(arg));
        }
        return original;
    }

    unique_ptr<Block> preTransformBlock(core::MutableContext ctx, unique_ptr<Block> original) {
        for (auto &arg : original->args) {
            arg = substArg(ctx, move(arg));
        }
        return original;
    }

    unique_ptr<Expression> postTransformUnresolvedIdent(core::MutableContext ctx,
                                                        unique_ptr<UnresolvedIdent> original) {
        original->name = subst.substitute(original->name);
        return original;
    }

    unique_ptr<Send> preTransformSend(core::MutableContext ctx, unique_ptr<Send> original) {
        original->fun = subst.substitute(original->fun);
        return original;
    }
    unique_ptr<Expression> postTransformLiteral(core::MutableContext ctx, unique_ptr<Literal> original) {
        if (original->isString(ctx)) {
            auto nameRef = original->asString(ctx);
            auto newName = subst.substitute(nameRef);
            if (newName == nameRef) {
                return original;
            }
            return MK::String(original->loc, newName);
        }
        if (original->isSymbol(ctx)) {
            auto nameRef = original->asSymbol(ctx);
            auto newName = subst.substitute(nameRef);
            if (newName == nameRef) {
                return original;
            }
            return MK::Symbol(original->loc, newName);
        }
        return original;
    }
    unique_ptr<Expression> postTransformConstantLit(core::MutableContext ctx, unique_ptr<ConstantLit> original) {
        original->cnst = subst.substitute(original->cnst);
        original->scope = substClassName(ctx, move(original->scope));
        return original;
    }
};

std::unique_ptr<Expression> sorbet::ast::Substitute::run(core::MutableContext ctx,
                                                         const sorbet::core::GlobalSubstitution &subst,
                                                         std::unique_ptr<Expression> what) {
    if (subst.useFastPath()) {
        return what;
    }
    SubstWalk walk(subst);
    what = TreeMap::apply(ctx, walk, move(what));
    return what;
}
