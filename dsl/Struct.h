#ifndef SRUBY_DSL_STRUCT_H
#define SRUBY_DSL_STRUCT_H
#include "ast/ast.h"

namespace ruby_typer {
namespace dsl {

/**
 * This class desugars things of the form
 *
 *   A = Struct.new(:foo, :bar)
 *
 * into
 *
 *   class A < Struct
 *       declare_variables(
 *           :@foo => BasicObject,
 *           :@bar => BasicObject,
 *       )
 *       attr_accessor :foo
 *       attr_accessor :bar
 *       sig(foo: BasicObject, bar: BasicObject).returns(A)
 *       def self.new(foo=nil, bar=nil)
 *           T.cast(nil, A)
 *       end
 *   end
 */
class Struct final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::Context ctx, ast::Assign *asgn);

    Struct() = delete;
};

} // namespace dsl
} // namespace ruby_typer

#endif
