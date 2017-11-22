#ifndef SRUBY_NAMER_NAMER_H
#define SRUBY_NAMER_NAMER_H
#include "ast/ast.h"
#include "parser/parser.h"

namespace ruby_typer {
namespace namer {

class Namer final {
public:
    static unique_ptr<ast::Expression> run(core::Context &ctx, unique_ptr<ast::Expression> tree);

private:
    Namer() = default;
};

class Resolver final {
public:
    static unique_ptr<ast::Expression> run(core::Context &ctx, unique_ptr<ast::Expression> tree);
    static void finalize(core::GlobalState &gs);
};

} // namespace namer
}; // namespace ruby_typer

#endif
