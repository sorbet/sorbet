#include "verifier.h"
#include "ast/treemap/treemap.h"

using namespace std;
namespace sorbet::ast {

class VerifierWalker {
    uint32_t methodDepth = 0;

public:
    void preTransformExpressionPtr(core::Context ctx, const ExpressionPtr &original) {
        if (!isa_tree<EmptyTree>(original)) {
            ENFORCE(original.loc().exists(), "location is unset");
        }

        original._sanityCheck();
    }

    void preTransformMethodDef(core::Context ctx, const MethodDef &original) {
        methodDepth++;
    }

    void postTransformMethodDef(core::Context ctx, const MethodDef &original) {
        methodDepth--;
    }

    void postTransformAssign(core::Context ctx, const Assign &assign) {
        if (ast::isa_tree<ast::UnresolvedConstantLit>(assign.lhs)) {
            ENFORCE(methodDepth == 0, "Found constant definition inside method definition");
        }
    }

    void preTransformBlock(core::Context ctx, ExpressionPtr &original) {
        original._sanityCheck();
    }

    void postTransformUnresolvedIdent(core::Context ctx, ExpressionPtr &tree) {
        ENFORCE(false, "TODO");
    }

    void postTransformUnresolvedConstantLit(core::Context ctx, ExpressionPtr &tree) {
        ENFORCE(false, "TODO");
    }
};

ExpressionPtr Verifier::run(core::Context ctx, ExpressionPtr node) {
    if (!debug_mode) {
        return node;
    }
    VerifierWalker vw;
    ConstTreeWalk::apply(ctx, vw, node);
    return node;
}

} // namespace sorbet::ast
