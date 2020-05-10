#ifndef SORBET_REWRITER_REGEXP_H
#define SORBET_REWRITER_REGEXP_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *   A = Regexp.new(...)
 *
 * into
 *
 *   A = T.let(Regexp.new(...), Regexp)
 *
 */
class Regexp final {
public:
    static std::vector<ast::TreePtr> run(core::MutableContext ctx, ast::Assign *asgn);

    Regexp() = delete;
};

} // namespace sorbet::rewriter

#endif
