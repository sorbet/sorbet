#ifndef SORBET_REWRITER_DSLBUILDER_H
#define SORBET_REWRITER_DSLBUILDER_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars DSLs created by DSLBuilder. Specifically it desugars
 *
 *   dsl_{optional,required} :prop, SomeClass
 *
 * into (approximately):
 *
 *   sig {params(prop: SomeClass).returns(NilClass)}
 *   def self.prop(prop); end
 *
 *   sig {returns(SomeClass)}
 *   def self.get_prop; end
 *
 *   sig {returns(SomeClass)}
 *   def prop; end
 */
class DSLBuilder final {
public:
    static std::vector<ast::TreePtr> run(core::MutableContext ctx, ast::Send *send);

    DSLBuilder() = delete;
};

} // namespace sorbet::rewriter

#endif
