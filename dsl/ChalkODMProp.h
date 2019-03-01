#ifndef SORBET_DSL_CHALK_ODM_PROP_H
#define SORBET_DSL_CHALK_ODM_PROP_H
#include "ast/ast.h"

namespace sorbet::dsl {

/**
 * This class desugars things of the form
 *
 *   prop :foo, String
 *
 * into
 *
 *   sig {returns(T.nilable(String))}
 *   def foo; T.cast(nil, T.nilable(String)); end
 *   sig {params(arg0: String).returns(NilClass)}
 *   def foo=(arg0); end
 *   class Mutator < Chalk::ODM::Mutator
 *     sig {params(arg0: String).returns(NilClass)}
 *     def foo=(arg0); end
 *   end
 *
 * We try to implement a simple approximation of the functionality that
 * Chalk::ODM::Document.prop has. This isn't full fidelity, but we're trying to
 * straddle the line between complexity and usefulness. Specifically:
 *
 * Any `prop` method call in a class body whose shape matches is considered.
 * `const ...` is the same as `prop ..., immutable: true`.
 * The getter will return `T.nilable(TheType)` and the setter will take `TheType`.
 * In the last param if there is a:
 *   type: TheType - overrides the second param.
 *   array: TheType - overrides the second param and makes it an `Array[TheValue]`.
 *   default: - the getter isn't nilable anymore.
 *   factory: - same as default:.
 *
 * Any deviation from this expected shape stops the desugaring.
 *
 */
class ChalkODMProp final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::MutableContext ctx, ast::Send *send);

    ChalkODMProp() = delete;
};

} // namespace sorbet::dsl

#endif
