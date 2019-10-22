#ifndef SORBET_DSL_FLATTEN_H
#define SORBET_DSL_FLATTEN_H
#include "ast/ast.h"

namespace sorbet::dsl {

class Flatten final {
public:
    static std::unique_ptr<ast::Expression> patchFile(core::Context ctx, std::unique_ptr<ast::Expression> tree);

    Flatten() = delete;
};

} // namespace sorbet::dsl

#endif
