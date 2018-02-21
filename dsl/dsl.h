#ifndef SRUBY_DSL_DSL_H
#define SRUBY_DSL_DSL_H
#include "ast/ast.h"
#include <memory>

namespace ruby_typer {
namespace dsl {

class DSL final {
public:
    static std::unique_ptr<ast::Expression> run(core::MutableContext ctx, std::unique_ptr<ast::Expression> tree);

    DSL() = delete;
};

} // namespace dsl
} // namespace ruby_typer

#endif
