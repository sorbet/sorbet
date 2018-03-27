#ifndef SRUBY_DSL_DSLBUILDER_H
#define SRUBY_DSL_DSLBUILDER_H
#include "ast/ast.h"

namespace ruby_typer {
namespace dsl {

/**
 * This class desugars DSLs created by DSLBuilder. Specifically it desugars
 *
 *   dsl_{optional,required} :prop, SomeClass
 *
 * into (approximately):
 *
 *   sig(prop: SomeClass).returns(NilClass)
 *   def self.prop(prop); end
 *
 *   sig.returns(SomeClass)
 *   def self.get_prop; end
 *
 *   sig.returns(SomeClass)
 *   def prop; end
 */
class DSLBuilder final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::MutableContext ctx, ast::Send *send);

    DSLBuilder() = delete;
};

} // namespace dsl
} // namespace ruby_typer

#endif
