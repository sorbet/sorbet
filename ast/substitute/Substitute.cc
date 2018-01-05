#include "substitute.h"
#include "ast/treemap/treemap.h"
using namespace ruby_typer;
using namespace ruby_typer::ast;

class SubstWalk {
    const ruby_typer::core::GlobalSubstitution &subst;

public:
    SubstWalk(const ruby_typer::core::GlobalSubstitution &subst) : subst(subst) {}
    MethodDef *preTransformMethodDef(core::Context ctx, MethodDef *original) {
        original->name = subst.substitute(original->name);
        return original;
    }

    Expression *postTransformUnresolvedIdent(core::Context ctx, UnresolvedIdent *original) {
        original->name = subst.substitute(original->name);
        return original;
    }

    Send *preTransformSend(core::Context ctx, Send *original) {
        original->fun = subst.substitute(original->fun);
        return original;
    }
    NamedArg *preTransformNamedArg(core::Context ctx, NamedArg *original) {
        original->name = subst.substitute(original->name);
        return original;
    }
    Expression *postTransformStringLit(core::Context ctx, StringLit *original) {
        original->value = subst.substitute(original->value);
        return original;
    }
    Expression *postTransformSymbolLit(core::Context ctx, SymbolLit *original) {
        original->name = subst.substitute(original->name);
        return original;
    }
    Expression *postTransformConstantLit(core::Context ctx, ConstantLit *original) {
        original->cnst = subst.substitute(original->cnst);
        return original;
    }
};

std::unique_ptr<Expression> ruby_typer::ast::Substitute::run(core::Context ctx,
                                                             const ruby_typer::core::GlobalSubstitution &subst,
                                                             std::unique_ptr<Expression> what) {
    SubstWalk walk(subst);
    what = TreeMap<SubstWalk>::apply(ctx, walk, move(what));
    return what;
}
