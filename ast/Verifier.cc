#include "Verifier.h"
#include "ast.h"

namespace ruby_typer {
namespace ast {

class VerifierWalker {
    void check(Statement *node) {
        // EmptyTree is the only thing allowed to not have a loc, and there
        // isn't any transform for it in TreeMap, so all nodes need a loc
        DEBUG_ONLY(Error::check(!node->loc.is_none()));
    }

public:
    Statement *postTransformClassDef(Context ctx, ClassDef *original) {
        check(original);
        return original;
    }
    Statement *postTransformMethodDef(Context ctx, MethodDef *original) {
        check(original);
        return original;
    }
    Statement *postTransformIf(Context ctx, If *original) {
        check(original);
        return original;
    }
    Statement *postTransformWhile(Context ctx, While *original) {
        check(original);
        return original;
    }
    Statement *postTransformFor(Context ctx, For *original) {
        check(original);
        return original;
    }
    Statement *postTransformBreak(Context ctx, Break *original) {
        check(original);
        return original;
    }
    Statement *postTransformNext(Context ctx, Next *original) {
        check(original);
        return original;
    }
    Statement *postTransformReturn(Context ctx, Return *original) {
        check(original);
        return original;
    }
    Statement *postTransformYield(Context ctx, Yield *original) {
        check(original);
        return original;
    }
    Statement *postTransformRescue(Context ctx, Rescue *original) {
        check(original);
        return original;
    }
    Statement *postTransformIdent(Context ctx, Ident *original) {
        check(original);
        return original;
    }
    Statement *postTransformAssign(Context ctx, Assign *original) {
        check(original);
        return original;
    }
    Statement *postTransformSend(Context ctx, Send *original) {
        check(original);
        return original;
    }
    Statement *postTransformNamedArg(Context ctx, NamedArg *original) {
        check(original);
        return original;
    }
    Statement *postTransformHash(Context ctx, Hash *original) {
        check(original);
        return original;
    }
    Statement *postransformArray(Context ctx, Array *original) {
        check(original);
        return original;
    }
    Statement *postTransformBoolLit(Context ctx, BoolLit *original) {
        check(original);
        return original;
    }
    Statement *postTransformFloatLit(Context ctx, FloatLit *original) {
        check(original);
        return original;
    }
    Statement *postTransformIntLit(Context ctx, IntLit *original) {
        check(original);
        return original;
    }
    Statement *postTransformStringLit(Context ctx, StringLit *original) {
        check(original);
        return original;
    }
    Statement *postTransformConstantLit(Context ctx, ConstantLit *original) {
        check(original);
        return original;
    }
    Statement *postTransformArraySplat(Context ctx, ArraySplat *original) {
        check(original);
        return original;
    }
    Statement *postTransformHashSplat(Context ctx, HashSplat *original) {
        check(original);
        return original;
    }
    Statement *postTransformSelf(Context ctx, Self *original) {
        check(original);
        return original;
    }
    Statement *postTransformBlock(Context ctx, Block *original) {
        check(original);
        return original;
    }
    Statement *postTransformInsSeq(Context ctx, InsSeq *original) {
        check(original);
        return original;
    }
};

std::unique_ptr<Statement> Verifier::run(Context ctx, std::unique_ptr<Statement> node) {
    VerifierWalker vw;
    return TreeMap<VerifierWalker>::apply(ctx, vw, move(node));
}

} // namespace ast
} // namespace ruby_typer
