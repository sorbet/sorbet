#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet::realmain::lsp {

class DefLocSaver {
public:
    unique_ptr<ast::MethodDef> postTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> methodDef);
};
}; // namespace sorbet::realmain::lsp
