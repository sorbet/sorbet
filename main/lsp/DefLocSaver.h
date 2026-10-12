#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {

class DefLocSaver {
public:
    // Handles loc and symbol requests for method definitions.
    void postTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef);
    // Handles loc and symbol requests for instance variables.
    void postTransformUnresolvedIdent(core::Context ctx, const ast::UnresolvedIdent &id);

    // Handles loc and symbol requests for constants.
    void postTransformConstantLit(core::Context ctx, const ast::ConstantLit &lit);

    // Handles loc and symbol requests for ClassDef names.
    void preTransformClassDef(core::Context ctx, const ast::ClassDef &classDef);

    // Handles loc and symbol requests for alias_method calls.
    void postTransformSend(core::Context ctx, const ast::Send &send);
};
}; // namespace sorbet::realmain::lsp
