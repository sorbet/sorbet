#include "main/autogen/constant_hash.h"
#include "common/typecase.h"
#include "core/hashing/hashing.h"
#include "parser/Node.h"

using namespace std;
namespace sorbet::autogen {

// See the comment in `main/autogen/constant_hash.h` for the intention
// and high-level behavior of `constantHashNode`.
//
// A few operational notes about this function:
//
// - the reason it takes a parsed AST (rather than, say, a desugared
// one) is that this means we can do less allocation and run the hash
// as quickly as possible.
//
// - the reason it takes a reference to a `unique_ptr` is so it
// doesn't need to `move` or consume the AST: it just observes it. The
// reason it takes a pointer rather than a reference is so we don't
// need to check for nullability on every recursive call: we can check
// once at the head of `hashNode` instead.
//
// - there's a specific weakness in the recursive implementation of
// this function which I think is worth noting but shouldn't be a
// problem in practice: in particular, if we have a bare constant
// reference directly in the body of a class or module, that'll count
// towards the hash, even though it doesn't change the set of
// constants being defined or the inheritance thereof. That could be
// fixed, but it seemed somewhat more complex, so I left the
// implementation simpler. This should rarely be a problem in
// practice, since it's relatively rare to find a bare constant
// reference directly in that position.
unsigned int hashNode(core::GlobalState &gs, unsigned int hashSoFar, const unique_ptr<parser::Node> &what) {
    if (what.get() == nullptr) {
        return hashSoFar;
    }
    typecase(
        what.get(),
        [&](parser::Const *cnst) {
            // hash the scope then the name
            hashSoFar = hashNode(gs, hashSoFar, cnst->scope);
            hashSoFar = core::mix(hashSoFar, core::_hash("::"));
            hashSoFar = core::mix(hashSoFar, core::_hash(cnst->name.shortName(gs)));
        },
        [&](parser::Send *send) {
            // only need to handle if this is a `require`, `include`,
            // or `extend`
            if (send->method == core::Names::require()) {
                hashSoFar = core::mix(hashSoFar, core::_hash("(r"));
                if (send->args.size() >= 1) {
                    if (auto str = parser::cast_node<parser::String>(send->args[0].get())) {
                        hashSoFar = core::mix(hashSoFar, core::_hash(str->val.shortName(gs)));
                    }
                }
                hashSoFar = core::mix(hashSoFar, core::_hash(")"));
            } else if (send->method == core::Names::include() || send->method == core::Names::extend()) {
                hashSoFar = core::mix(hashSoFar, core::_hash("(i"));
                hashSoFar = core::mix(hashSoFar, core::_hash(send->method.shortName(gs)));
                for (auto &arg : send->args) {
                    hashSoFar = hashNode(gs, hashSoFar, arg);
                }
                hashSoFar = core::mix(hashSoFar, core::_hash(")"));
            }
        },
        [&](parser::Assign *asgn) {
            // we only care about this case if the lhs is a constant
            if (auto lhs = parser::cast_node<parser::ConstLhs>(asgn->lhs.get())) {
                if (parser::isa_node<parser::Const>(asgn->rhs.get())) {
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
                    hashSoFar = core::mix(hashSoFar, core::_hash(lhs->name.shortName(gs)));
                    hashSoFar = core::mix(hashSoFar, core::_hash(" "));
                    hashSoFar = hashNode(gs, hashSoFar, asgn->rhs);
                    hashSoFar = core::mix(hashSoFar, core::_hash(")"));
                } else {
                    // if the RHS is anything else, then it's a constant
                    // definition: we care only about the LHS in that case
                    hashSoFar = core::mix(hashSoFar, core::_hash("(x"));
                    hashSoFar = core::mix(hashSoFar, core::_hash(lhs->name.shortName(gs)));
                    hashSoFar = core::mix(hashSoFar, core::_hash(")"));
                }
            }
        },
        [&](parser::Module *module) {
            hashSoFar = core::mix(hashSoFar, core::_hash("(m"));
            hashSoFar = hashNode(gs, hashSoFar, module->name);
            hashSoFar = core::mix(hashSoFar, core::_hash(" "));
            hashSoFar = hashNode(gs, hashSoFar, module->body);
            hashSoFar = core::mix(hashSoFar, core::_hash(")"));
        },
        [&](parser::Class *claz) {
            hashSoFar = core::mix(hashSoFar, core::_hash("(c"));
            hashSoFar = hashNode(gs, hashSoFar, claz->name);
            hashSoFar = core::mix(hashSoFar, core::_hash("<"));
            hashSoFar = hashNode(gs, hashSoFar, claz->superclass);
            hashSoFar = core::mix(hashSoFar, core::_hash(" "));
            hashSoFar = hashNode(gs, hashSoFar, claz->body);
            hashSoFar = core::mix(hashSoFar, core::_hash(")"));
        },
        [&](parser::Begin *begin) {
            for (auto &stmt : begin->stmts) {
                hashSoFar = hashNode(gs, hashSoFar, stmt);
            }
        },
        // the above are the only cases we should need to handle;
        // otherwise we can just quit
        [&](parser::Node *node) {
            // nothing
        });
    return hashSoFar;
}

unsigned int constantHashNode(core::GlobalState &gs, const unique_ptr<parser::Node> &what) {
    return hashNode(gs, 0, what);
}

} // namespace sorbet::autogen
