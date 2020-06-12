#include "ast/ast.h"

namespace sorbet::ast {

class Verifier {
public:
    static TreePtr run(core::Context ctx, TreePtr node);
};

} // namespace sorbet::ast
