#ifndef SORBET_DSL_ATTR_READER_H
#define SORBET_DSL_ATTR_READER_H
#include "ast/ast.h"

namespace sorbet {
namespace dsl {

/**
 * This class desugars things of the form
 *
 *   attr_reader :foo
 *
 * into
 *
 *   def foo; @foo; end
 *
 * and
 *
 *   attr_writer :foo
 *
 * into
 *
 *   def foo=(arg0); @foo = arg0; end
 *
 * and `attr_accessor :foo` into both `attr_reader :foo` and `attr_writer :foo`.
 *
 */
class AttrReader final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::MutableContext ctx, ast::Send *send);

    AttrReader() = delete;
};

} // namespace dsl
} // namespace sorbet

#endif
