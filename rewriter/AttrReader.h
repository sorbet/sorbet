#ifndef SORBET_REWRITER_ATTR_READER_H
#define SORBET_REWRITER_ATTR_READER_H
#include "ast/ast.h"

namespace sorbet::rewriter {

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
    static std::vector<ast::TreePtr> run(core::MutableContext ctx, ast::Send *send, ast::TreePtr *prevStat);

    AttrReader() = delete;
};

} // namespace sorbet::rewriter

#endif
