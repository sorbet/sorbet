#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {

class DefLocSaver {
public:
    // Handles loc and symbol requests for method definitions.
    ast::TreePtr postTransformMethodDef(core::Context ctx, ast::TreePtr methodDef);
    // Handles loc and symbol requests for instance variables.
    ast::TreePtr postTransformUnresolvedIdent(core::Context ctx, ast::TreePtr id);

    // Handles loc and symbol requests for constants.
    ast::TreePtr postTransformConstantLit(core::Context ctx, ast::TreePtr lit);
};
}; // namespace sorbet::realmain::lsp
