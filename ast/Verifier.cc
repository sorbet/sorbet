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
    Expression *postTransformClassDef(Context ctx, ClassDef *original) {
        check(original);
        return original;
    }
    Expression *postTransformMethodDef(Context ctx, MethodDef *original) {
        check(original);
        return original;
    }
    Expression *postTransformIf(Context ctx, If *original) {
        check(original);
        return original;
    }
    Expression *postTransformWhile(Context ctx, While *original) {
        check(original);
        return original;
    }
    Expression *postTransformFor(Context ctx, For *original) {
        check(original);
        return original;
    }
    Expression *postTransformBreak(Context ctx, Break *original) {
        check(original);
        return original;
    }
    Expression *postTransformNext(Context ctx, Next *original) {
        check(original);
        return original;
    }
    Expression *postTransformReturn(Context ctx, Return *original) {
        check(original);
        return original;
    }
    Expression *postTransformYield(Context ctx, Yield *original) {
        check(original);
        return original;
    }
    Expression *postTransformRescue(Context ctx, Rescue *original) {
        check(original);
        return original;
    }
    Expression *postTransformIdent(Context ctx, Ident *original) {
        check(original);
        return original;
    }
    Expression *postTransformAssign(Context ctx, Assign *original) {
        check(original);
        return original;
    }
    Expression *postTransformSend(Context ctx, Send *original) {
        check(original);
        return original;
    }
    Expression *postTransformNamedArg(Context ctx, NamedArg *original) {
        check(original);
        return original;
    }
    Expression *postTransformHash(Context ctx, Hash *original) {
        check(original);
        return original;
    }
    Expression *postransformArray(Context ctx, Array *original) {
        check(original);
        return original;
    }
    Expression *postTransformBoolLit(Context ctx, BoolLit *original) {
        check(original);
        return original;
    }
    Expression *postTransformFloatLit(Context ctx, FloatLit *original) {
        check(original);
        return original;
    }
    Expression *postTransformIntLit(Context ctx, IntLit *original) {
        check(original);
        return original;
    }
    Expression *postTransformStringLit(Context ctx, StringLit *original) {
        check(original);
        return original;
    }
    Expression *postTransformConstantLit(Context ctx, ConstantLit *original) {
        check(original);
        return original;
    }
    Expression *postTransformArraySplat(Context ctx, ArraySplat *original) {
        check(original);
        return original;
    }
    Expression *postTransformHashSplat(Context ctx, HashSplat *original) {
        check(original);
        return original;
    }
    Expression *postTransformSelf(Context ctx, Self *original) {
        check(original);
        return original;
    }
    Expression *postTransformBlock(Context ctx, Block *original) {
        check(original);
        return original;
    }
    Expression *postTransformInsSeq(Context ctx, InsSeq *original) {
        check(original);
        return original;
    }
};

std::unique_ptr<Expression> Verifier::run(Context ctx, std::unique_ptr<Expression> node) {
    VerifierWalker vw;
    return TreeMap<VerifierWalker>::apply(ctx, vw, move(node));
}

} // namespace ast
} // namespace ruby_typer
