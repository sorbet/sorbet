#include "ast/ast.h"

namespace sorbet {
namespace ast {

class Verifier {
public:
    static std::unique_ptr<Expression> run(core::MutableContext ctx, std::unique_ptr<Expression> node);
};

} // namespace ast
} // namespace sorbet
