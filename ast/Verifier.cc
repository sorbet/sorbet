#include "Verifier.h"
#include "ast.h"

namespace ruby_typer {
namespace ast {

class VerifierWalker {
    void check(Expression *node) {
        // EmptyTree is the only thing allowed to not have a loc, and there
        // isn't any transform for it in TreeMap, so all nodes need a loc
        DEBUG_ONLY(Error::check(!node->loc.is_none()));
    }

public:
    Expression *postTransformClassDef(core::Context ctx, ClassDef *original) {
        check(original);
        return original;
    }
    Expression *postTransformMethodDef(core::Context ctx, MethodDef *original) {
        check(original);
        return original;
    }
    Expression *postTransformIf(core::Context ctx, If *original) {
        check(original);
        return original;
    }
    Expression *postTransformWhile(core::Context ctx, While *original) {
        check(original);
        return original;
    }
    Expression *postTransformBreak(core::Context ctx, Break *original) {
        check(original);
        return original;
    }
    Expression *postTransformNext(core::Context ctx, Next *original) {
        check(original);
        return original;
    }
    Expression *postTransformReturn(core::Context ctx, Return *original) {
        check(original);
        return original;
    }
    Expression *postTransformYield(core::Context ctx, Yield *original) {
        check(original);
        return original;
    }
    Expression *postTransformRescue(core::Context ctx, Rescue *original) {
        check(original);
        return original;
    }
    Expression *postTransformIdent(core::Context ctx, Ident *original) {
        check(original);
        return original;
    }
    Expression *postTransformLocal(core::Context ctx, Local *original) {
        check(original);
        return original;
    }
    Expression *postTransformAssign(core::Context ctx, Assign *original) {
        check(original);
        return original;
    }
    Expression *postTransformSend(core::Context ctx, Send *original) {
        check(original);
        return original;
    }
    Expression *postTransformNamedArg(core::Context ctx, NamedArg *original) {
        check(original);
        return original;
    }
    Expression *postTransformHash(core::Context ctx, Hash *original) {
        check(original);
        return original;
    }
    Expression *postransformArray(core::Context ctx, Array *original) {
        check(original);
        return original;
    }
    Expression *postTransformBoolLit(core::Context ctx, BoolLit *original) {
        check(original);
        return original;
    }
    Expression *postTransformFloatLit(core::Context ctx, FloatLit *original) {
        check(original);
        return original;
    }
    Expression *postTransformIntLit(core::Context ctx, IntLit *original) {
        check(original);
        return original;
    }
    Expression *postTransformStringLit(core::Context ctx, StringLit *original) {
        check(original);
        return original;
    }
    Expression *postTransformConstantLit(core::Context ctx, ConstantLit *original) {
        check(original);
        return original;
    }
    Expression *postTransformArraySplat(core::Context ctx, ArraySplat *original) {
        check(original);
        return original;
    }
    Expression *postTransformHashSplat(core::Context ctx, HashSplat *original) {
        check(original);
        return original;
    }
    Expression *postTransformSelf(core::Context ctx, Self *original) {
        check(original);
        return original;
    }
    Expression *postTransformBlock(core::Context ctx, Block *original) {
        check(original);
        return original;
    }
    Expression *postTransformInsSeq(core::Context ctx, InsSeq *original) {
        check(original);
        return original;
    }
};

std::unique_ptr<Expression> Verifier::run(core::Context ctx, std::unique_ptr<Expression> node) {
    VerifierWalker vw;
    return TreeMap<VerifierWalker>::apply(ctx, vw, move(node));
}

} // namespace ast
} // namespace ruby_typer
