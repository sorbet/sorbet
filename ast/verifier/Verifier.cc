#include "verifier.h"
#include "ast/treemap/treemap.h"

using namespace std;
namespace sorbet::ast {

class VerifierWalker {
    u4 methodDepth = 0;

public:
    TreePtr preTransformExpression(core::Context ctx, TreePtr original) {
        if (!isa_tree<EmptyTree>(original)) {
            ENFORCE(original->loc.exists(), "location is unset");
        }

        original._sanityCheck();

        return original;
    }

    TreePtr preTransformMethodDef(core::Context ctx, TreePtr original) {
        methodDepth++;
        return original;
    }

    TreePtr postTransformMethodDef(core::Context ctx, TreePtr original) {
        methodDepth--;
        return original;
    }

    TreePtr postTransformAssign(core::Context ctx, TreePtr original) {
        auto *assign = cast_tree<Assign>(original);
        if (ast::isa_tree<ast::UnresolvedConstantLit>(assign->lhs)) {
            ENFORCE(methodDepth == 0, "Found constant definition inside method definition");
        }
        return original;
    }

    TreePtr preTransformBlock(core::Context ctx, TreePtr original) {
        original._sanityCheck();
        return original;
    }
};

TreePtr Verifier::run(core::Context ctx, TreePtr node) {
    if (!debug_mode) {
        return node;
    }
    VerifierWalker vw;
    return TreeMap::apply(ctx, vw, move(node));
}

} // namespace sorbet::ast
