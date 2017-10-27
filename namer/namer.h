#include "ast/ast.h"
#include "parser/parser.h"

namespace ruby_typer {
namespace namer {

class Namer {
public:
    static unique_ptr<ast::Statement> run(ast::Context &ctx, unique_ptr<ast::Statement> tree);

private:
    static unique_ptr<ast::Statement> resolve(ast::Context &ctx, unique_ptr<ast::Statement> tree);

    Namer() = default;
};

} // namespace namer
}; // namespace ruby_typer
