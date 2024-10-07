#ifndef SORBET_REWRITER_TLAMBDA_H
#define SORBET_REWRITER_TLAMBDA_H
#include "ast/ast.h"

namespace sorbet::rewriter {

class TLambda final {
public:
    static bool run(core::MutableContext ctx, ast::Send *send);

    TLambda() = delete;
};

} // namespace sorbet::rewriter

#endif
