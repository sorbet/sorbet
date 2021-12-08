#include "verifier.h"
#include "ast/treemap/treemap.h"

using namespace std;
namespace sorbet::ast {

class VerifierWalker {
    uint32_t methodDepth = 0;

public:
    ExpressionPtr preTransformExpression(core::Context ctx, ExpressionPtr original) {
        if (!isa_tree<EmptyTree>(original)) {
            ENFORCE(original.loc().exists(), "location is unset");
        }

        original._sanityCheck();

        return original;
    }

    ExpressionPtr preTransformMethodDef(core::Context ctx, ExpressionPtr original) {
        methodDepth++;
        return original;
    }

    ExpressionPtr postTransformMethodDef(core::Context ctx, ExpressionPtr original) {
        methodDepth--;
        return original;
    }

    ExpressionPtr postTransformAssign(core::Context ctx, ExpressionPtr original) {
        auto *assign = cast_tree<Assign>(original);
        if (ast::isa_tree<ast::UnresolvedConstantLit>(assign->lhs)) {
            ENFORCE(methodDepth == 0, "Found constant definition inside method definition");
        }
        return original;
    }

    ExpressionPtr preTransformBlock(core::Context ctx, ExpressionPtr original) {
        original._sanityCheck();
        return original;
    }
};

ExpressionPtr Verifier::run(core::Context ctx, ExpressionPtr node) {
    if (!debug_mode) {
        return node;
    }
    VerifierWalker vw;
    return TreeMap::apply(ctx, vw, move(node));
}

} // namespace sorbet::ast
