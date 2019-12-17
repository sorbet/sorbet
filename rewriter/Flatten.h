#ifndef SORBET_REWRITER_FLATTEN_H
#define SORBET_REWRITER_FLATTEN_H
#include "ast/ast.h"

namespace sorbet::rewriter {

class Flatten final {
public:
    static std::unique_ptr<ast::Expression> run(core::Context ctx, std::unique_ptr<ast::Expression> tree);

    Flatten() = delete;
};

} // namespace sorbet::rewriter

#endif
