#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {

class DefLocSaver {
public:
    // Handles loc and symbol requests for method definitions.
    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &methodDef);
    // Handles loc and symbol requests for instance variables.
    void postTransformUnresolvedIdent(core::Context ctx, ast::ExpressionPtr &id);

    // Handles loc and symbol requests for constants.
    void postTransformConstantLit(core::Context ctx, ast::ExpressionPtr &lit);

    // Handles loc and symbol requests for ClassDef names.
    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &lit);
};
}; // namespace sorbet::realmain::lsp
