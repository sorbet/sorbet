#ifndef SORBET_NAMER_NAMER_H
#define SORBET_NAMER_NAMER_H
#include "ast/ast.h"
#include <memory>

namespace sorbet::namer {

class Namer final {
public:
    static ast::ParsedFile run(core::MutableContext ctx, ast::ParsedFile tree);

    Namer() = delete;
};

} // namespace sorbet::namer

#endif
