#include "ast/ast.h"
#include "parser/parser.h"

namespace ruby_typer {
namespace namer {

class Namer {
public:
    static unique_ptr<ast::Expression> run(ast::Context &ctx, unique_ptr<ast::Expression> tree);

private:
    static unique_ptr<ast::Expression> resolve(ast::Context &ctx, unique_ptr<ast::Expression> tree);

    Namer() = default;
};

} // namespace namer
}; // namespace ruby_typer
