#include "main/autogen/constant_hash.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "common/typecase.h"
#include "core/hashing/hashing.h"

using namespace std;
namespace sorbet::autogen {

// See the comment in `main/autogen/constant_hash.h` for the intention
// and high-level behavior of `constantHashNode`.
//
// A few operational notes about this function:
//
// - the reason it takes a desugared AST (instead of a parsed one) is
// so that we have flexibility in where to apply it. Originally this
// took an AST direct from the parser (since desugaring doesn't
// actually make this any simpler) but it drastically limited where it
// could appear
struct ConstantHashWalk {
    unsigned int hashSoFar;
    ConstantHashWalk() : hashSoFar(0){};

    void hashConstant(core::Context ctx, ast::ExpressionPtr &expr) {
        if (auto cnst = ast::cast_tree<ast::UnresolvedConstantLit>(expr)) {
            hashConstant(ctx, cnst->scope);
            hashSoFar = core::mix(hashSoFar, core::_hash("::"));
            hashSoFar = core::mix(hashSoFar, core::_hash(cnst->cnst.shortName(ctx)));
        } else if (auto cnst = ast::cast_tree<ast::ConstantLit>(expr)) {
            // this shouldn't come up when we plan to run this, but
            // it's not hard to support it in case we happen to need
            // to constant hash later in the pipeline
            hashConstant(ctx, cnst->original);
        }
        // no other cases matter here
    }

    bool isConstant(ast::ExpressionPtr &tree) {
        return ast::isa_tree<ast::UnresolvedConstantLit>(tree) || ast::isa_tree<ast::ConstantLit>(tree);
    }

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        hashSoFar =
            core::mix(hashSoFar, klass.kind == ast::ClassDef::Kind::Class ? core::_hash("(c") : core::_hash("(m"));
        hashConstant(ctx, klass.name);
        hashSoFar = core::mix(hashSoFar, core::_hash("<"));
        if (klass.ancestors.size() > 0) {
            hashConstant(ctx, klass.ancestors.front());
        }
        hashSoFar = core::mix(hashSoFar, core::_hash(" "));
    }

    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        hashSoFar = core::mix(hashSoFar, core::_hash(")"));
    }

    void postTransformSend(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);
        if (send.fun == core::Names::require()) {
            hashSoFar = core::mix(hashSoFar, core::_hash("(r"));
            if (send.hasPosArgs()) {
                if (auto str = ast::cast_tree<ast::Literal>(send.posArgs().front())) {
                    if (str->isString()) {
                        hashSoFar = core::mix(hashSoFar, core::_hash(str->asString().shortName(ctx)));
                    }
                }
            }
            hashSoFar = core::mix(hashSoFar, core::_hash(")"));
        } else if (send.fun == core::Names::include() || send.fun == core::Names::extend()) {
            hashSoFar = core::mix(hashSoFar, core::_hash("(i"));
            hashSoFar = core::mix(hashSoFar, core::_hash(send.fun.shortName(ctx)));
            for (auto &arg : send.posArgs()) {
                hashConstant(ctx, arg);
            }
            hashSoFar = core::mix(hashSoFar, core::_hash(")"));
        }
    }

    void postTransformAssign(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);
        if (isConstant(asgn.lhs)) {
            if (isConstant(asgn.rhs)) {
                // if the RHS is a constant literal, then this is
                // an alias and we care about both sides. (This is
                // because changing the RHS can, in some perverse
                // cases, affect constant resolution: for example,
                // in a snippet line
                //
                //   A = B
                //   class C < A; end
                //
                // which might in turn produce different autogen
                // output, i.e. different subclasses lists.)
                hashSoFar = core::mix(hashSoFar, core::_hash("(a"));
                hashConstant(ctx, asgn.lhs);
                hashSoFar = core::mix(hashSoFar, core::_hash(" "));
                hashConstant(ctx, asgn.rhs);
                hashSoFar = core::mix(hashSoFar, core::_hash(")"));
            } else {
                // if the RHS is anything else, then it's a constant
                // definition: we care only about the LHS in that case
                hashSoFar = core::mix(hashSoFar, core::_hash("(x"));
                hashConstant(ctx, asgn.lhs);
                hashSoFar = core::mix(hashSoFar, core::_hash(")"));
            }
        }
    }
};

HashedParsedFile constantHashTree(const core::GlobalState &gs, ast::ParsedFile pf) {
    core::Context ctx{gs, core::Symbols::root(), pf.file};
    ConstantHashWalk walk;
    ast::ShallowWalk::apply(ctx, walk, pf.tree);
    return {move(pf), walk.hashSoFar};
}

} // namespace sorbet::autogen
