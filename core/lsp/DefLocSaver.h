#include "ast/ast.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet {
namespace lsp {
class DefLocSaver {
public:
    unique_ptr<ast::MethodDef> postTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> methodDef) {
        // This TypeAndOrigins object is currently unused so we just let the default constructor make tp.type
        // an empty shared_ptr and tp.origins an empty vector
        core::TypeAndOrigins tp;

        bool lspQueryMatch = !ctx.state.lspInfoQueryLoc.is_none() && methodDef->loc.contains(ctx.state.lspInfoQueryLoc);

        if (lspQueryMatch) {
            core::QueryResponse::setQueryResponse(ctx, core::QueryResponse::Kind::DEFINITION, {}, nullptr,
                                                  methodDef->loc, methodDef->name, tp, tp);
        }

        return methodDef;
    }
};
} // namespace lsp
}; // namespace sorbet
