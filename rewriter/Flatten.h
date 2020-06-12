#ifndef SORBET_REWRITER_FLATTEN_H
#define SORBET_REWRITER_FLATTEN_H
#include "ast/ast.h"

namespace sorbet::rewriter {

class Flatten final {
public:
    static ast::TreePtr run(core::Context ctx, ast::TreePtr tree);

    Flatten() = delete;
};

} // namespace sorbet::rewriter

#endif
