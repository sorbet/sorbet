#include "verifier.h"
#include "ast/treemap/treemap.h"

using namespace std;
namespace sorbet::ast {

class VerifierWalker {
    u4 methodDepth = 0;

public:
    unique_ptr<Expression> preTransformExpression(core::Context ctx, unique_ptr<Expression> original) {
        if (!isa_tree<EmptyTree>(original.get())) {
            ENFORCE(original->loc.exists(), "location is unset");
        }

        original->_sanityCheck();

        return original;
    }

    unique_ptr<MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<MethodDef> original) {
        methodDepth++;
        return original;
    }

    unique_ptr<Expression> postTransformMethodDef(core::Context ctx, unique_ptr<MethodDef> original) {
        methodDepth--;
        return original;
    }

    unique_ptr<Expression> postTransformAssign(core::Context ctx, unique_ptr<Assign> original) {
        if (ast::isa_tree<ast::UnresolvedConstantLit>(original->lhs.get())) {
            ENFORCE(methodDepth == 0, "Found constant definition inside method definition");
        }
        return original;
    }

    unique_ptr<Block> preTransformBlock(core::Context ctx, unique_ptr<Block> original) {
        original->_sanityCheck();
        return original;
    }
};

unique_ptr<Expression> Verifier::run(core::Context ctx, unique_ptr<Expression> node) {
    if (!debug_mode) {
        return node;
    }
    VerifierWalker vw;
    return TreeMap::apply(ctx, vw, move(node));
}

} // namespace sorbet::ast
