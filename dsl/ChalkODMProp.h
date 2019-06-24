#ifndef SORBET_DSL_CHALK_ODM_PROP_H
#define SORBET_DSL_CHALK_ODM_PROP_H
#include "ast/ast.h"

namespace sorbet::dsl {

/**
 * This class desugars things of the form
 *
 *   prop :foo, Type
 *
 * into
 *
 *   sig {returns(Type)}
 *   def foo; ...; end
 *   sig {params(arg0: Type).returns(Type)}
 *   def foo=(arg0); ...; end
 *   class Mutator < Chalk::ODM::Mutator
 *     sig {returns(Type)}
 *     def foo(arg0); end
 *     sig {params(arg0: Type).returns(Type)}
 *     def foo=(arg0); end
 *   end
 *
 * We try to implement a simple approximation of the functionality that Chalk::ODM::Document.prop has. Any deviation
 * from the expected shape stops the desugaring.
 *
 * Most other `replaceDSL`s return just nodes, but we also want to keep track of the prop information so that at the end
 * of the DSL pass on the classDef, we can construct an `initialize` method with good static types.
 *
 */
class ChalkODMProp final {
public:
    struct Prop {};

    struct NodesAndProp {
        // invariant: !nodes.empty().
        std::vector<std::unique_ptr<ast::Expression>> nodes;
        Prop prop;
    };

    static std::optional<NodesAndProp> replaceDSL(core::MutableContext ctx, ast::Send *send);

    ChalkODMProp() = delete;
};

} // namespace sorbet::dsl

#endif
