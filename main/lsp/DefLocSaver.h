#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {

class DefLocSaver {
    // Only populated for package files--tracks the enclosing method name.
    //
    // Note: we don't handle SendResponse's in DefLocSaver: we do that in inference, where we
    // actually have the dispatch results. Don't abuse this for attempting to handle SendResponse's.
    std::vector<core::NameRef> sendStack;

public:
    DefLocSaver() {
        // Start with an empty name so that we can always ask for `.back()` of this vector.
        sendStack.emplace_back(core::NameRef::noName());
    }

    void preTransformSend(core::Context ctx, ast::ExpressionPtr &send);
    void postTransformSend(core::Context ctx, ast::ExpressionPtr &send);

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
