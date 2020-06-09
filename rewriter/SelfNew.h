#ifndef SORBET_REWRITER_SELF_NEW_H
#define SORBET_REWRITER_SELF_NEW_H

#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars `self.new` into a call to a special intrinsic:
 *
 *    self.new(arg1, ..., argN)
 *
 * =>
 *
 *    Magic.<self-new>(self, arg1, ..., argN)
 *
 * The `Magic.<self-new>` intrinsic will construct a value of type of the
 * enclosing class, and return its type as `C::<AttachedClass>`
 */
class SelfNew final {
public:
    static ast::TreePtr run(core::MutableContext ctx, ast::Send *send);
    SelfNew() = delete;
};

} // namespace sorbet::rewriter

#endif /* SORBET_REWRITER_SELF_NEW_H */
