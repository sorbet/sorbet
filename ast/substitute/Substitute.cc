#include "substitute.h"
#include "ast/treemap/treemap.h"

using namespace ruby_typer;
using namespace ruby_typer::ast;

class SubstWalk {
private:
    const ruby_typer::core::GlobalSubstitution &subst;

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
    SubstWalk(const ruby_typer::core::GlobalSubstitution &subst) : subst(subst) {}

    ClassDef *preTransformClassDef(core::MutableContext ctx, ClassDef *original) {
        original->name = substClassName(ctx, move(original->name));
        for (auto &anc : original->ancestors) {
            anc = substClassName(ctx, move(anc));
        }
        return original;
    }
    MethodDef *preTransformMethodDef(core::MutableContext ctx, MethodDef *original) {
        original->name = subst.substitute(original->name);
        for (auto &arg : original->args) {
            arg = substArg(ctx, move(arg));
        }
        return original;
    }

    Block *preTransformBlock(core::MutableContext ctx, Block *original) {
        for (auto &arg : original->args) {
            arg = substArg(ctx, move(arg));
        }
        return original;
    }

    Expression *postTransformUnresolvedIdent(core::MutableContext ctx, UnresolvedIdent *original) {
        original->name = subst.substitute(original->name);
        return original;
    }

    Send *preTransformSend(core::MutableContext ctx, Send *original) {
        original->fun = subst.substitute(original->fun);
        return original;
    }
    Expression *postTransformStringLit(core::MutableContext ctx, StringLit *original) {
        original->value = subst.substitute(original->value);
        return original;
    }
    Expression *postTransformSymbolLit(core::MutableContext ctx, SymbolLit *original) {
        original->name = subst.substitute(original->name);
        return original;
    }
    Expression *postTransformConstantLit(core::MutableContext ctx, ConstantLit *original) {
        original->cnst = subst.substitute(original->cnst);
        original->scope = substClassName(ctx, move(original->scope));
        return original;
    }
};

std::unique_ptr<Expression> ruby_typer::ast::Substitute::run(core::MutableContext ctx,
                                                             const ruby_typer::core::GlobalSubstitution &subst,
                                                             std::unique_ptr<Expression> what) {
    SubstWalk walk(subst);
    what = TreeMap::apply(ctx, walk, move(what));
    return what;
}
