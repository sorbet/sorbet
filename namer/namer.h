#ifndef SRUBY_NAMER_NAMER_H
#define SRUBY_NAMER_NAMER_H
#include "ast/ast.h"
#include <memory>

namespace ruby_typer {
namespace namer {

class Namer final {
public:
    static std::unique_ptr<ast::Expression> run(core::Context &ctx, std::unique_ptr<ast::Expression> tree);

private:
    Namer() = default;
};

class Resolver final {
public:
    static std::unique_ptr<ast::Expression> run(core::Context &ctx, std::unique_ptr<ast::Expression> tree);
    static void finalize(core::GlobalState &gs);
};

} // namespace namer
}; // namespace ruby_typer

#endif
