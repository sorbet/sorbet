#include "ast/ast.h"

namespace sorbet::ast {

class Verifier {
public:
    static ExpressionPtr run(core::Context ctx, ExpressionPtr node);
};

} // namespace sorbet::ast
