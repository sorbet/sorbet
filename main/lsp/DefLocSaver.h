#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {

class DefLocSaver {
public:
    std::unique_ptr<ast::MethodDef> postTransformMethodDef(core::Context ctx,
                                                           std::unique_ptr<ast::MethodDef> methodDef);
};
}; // namespace sorbet::realmain::lsp
