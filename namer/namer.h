#ifndef SORBET_NAMER_NAMER_H
#define SORBET_NAMER_NAMER_H
#include "ast/ast.h"
#include <memory>

namespace sorbet::namer {

class Namer final {
public:
    static std::unique_ptr<ast::Expression> run(core::MutableContext ctx, std::unique_ptr<ast::Expression> tree);

    Namer() = delete;
};

} // namespace sorbet::namer

#endif
