#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {

class DefLocSaver {
public:
    // Handles loc and symbol requests for method definitions.
    std::unique_ptr<ast::MethodDef> postTransformMethodDef(core::Context ctx,
                                                           std::unique_ptr<ast::MethodDef> methodDef);
    // Handles loc and symbol requests for instance variables.
    std::unique_ptr<ast::UnresolvedIdent> postTransformUnresolvedIdent(core::Context ctx,
                                                                       std::unique_ptr<ast::UnresolvedIdent> id);

    // Handles loc and symbol requests for constants.
    std::unique_ptr<ast::ConstantLit> postTransformConstantLit(core::Context ctx,
                                                               std::unique_ptr<ast::ConstantLit> lit);
};
}; // namespace sorbet::realmain::lsp
