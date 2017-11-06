#include "ast/ast.h"

namespace ruby_typer {
namespace ast {

class Verifier {
public:
    static std::unique_ptr<Statement> run(Context ctx, std::unique_ptr<Statement> node);
};

} // namespace ast
} // namespace ruby_typer
