#include "ast/ast.h"
#include "parser/parser.h"

namespace ruby_typer {
namespace namer {

class Namer final {
public:
    static unique_ptr<ast::Expression> run(core::Context &ctx, unique_ptr<ast::Expression> tree);

private:
    static unique_ptr<ast::Expression> resolve(core::Context &ctx, unique_ptr<ast::Expression> tree);

    Namer() = default;
};

} // namespace namer
}; // namespace ruby_typer
