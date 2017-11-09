#include "ast/ast.h"

namespace ruby_typer {
namespace ast {

class Verifier {
public:
    static std::unique_ptr<Expression> run(Context ctx, std::unique_ptr<Expression> node);
};

} // namespace ast
} // namespace ruby_typer
