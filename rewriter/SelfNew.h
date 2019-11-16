#ifndef SORBET_REWRITER_SELF_NEW_H
#define SORBET_REWRITER_SELF_NEW_H

#include "ast/ast.h"

namespace sorbet::rewriter {

class SelfNew final {
public:
    static std::unique_ptr<ast::Expression> run(core::MutableContext ctx, ast::Send *send);
    SelfNew() = delete;
};

} // namespace sorbet::rewriter

#endif /* SORBET_REWRITER_SELF_NEW_H */
