#include "DefLocSaver.h"
#include "ast/Helpers.h"
#include "core/lsp/Query.h"
#include "core/lsp/QueryResponse.h"

using namespace std;
namespace sorbet::realmain::lsp {

unique_ptr<ast::MethodDef> DefLocSaver::postTransformMethodDef(core::Context ctx,
                                                               unique_ptr<ast::MethodDef> methodDef) {
    const core::lsp::Query &lspQuery = ctx.state.lspQuery;
    bool lspQueryMatch = lspQuery.matchesLoc(methodDef->declLoc);

    if (lspQueryMatch) {
        // Query matches against the method definition as a whole.
        auto &symbolData = methodDef->symbol.data(ctx);
        auto &argTypes = symbolData->arguments();
        core::TypeAndOrigins tp;

        // Check if it matches against a specific argument. If it does, send that instead;
        // it's more specific.
        const int numArgs = methodDef->args.size();

        ENFORCE(numArgs == argTypes.size());
        for (int i = 0; i < numArgs; i++) {
            auto &arg = methodDef->args[i];
            auto &argType = argTypes[i];
            auto *localExp = ast::MK::arg2Local(arg.get());
            // localExp should never be null, but guard against the possibility.
            if (localExp) {
                if (lspQuery.matchesLoc(localExp->loc)) {
                    tp.type = argType.data(ctx)->resultType;
                    tp.origins.emplace_back(localExp->loc);
                    core::lsp::QueryResponse::pushQueryResponse(
                        ctx, core::lsp::IdentResponse(methodDef->symbol, localExp->loc, localExp->localVariable, tp));
                    return methodDef;
                }
            }
        }

        core::DispatchComponent dispatchComponent;
        core::DispatchResult::ComponentVec dispatchComponents;
        dispatchComponent.method = methodDef->symbol;
        dispatchComponents.emplace_back(std::move(dispatchComponent));
        tp.type = symbolData->resultType;
        tp.origins.emplace_back(methodDef->declLoc);
        core::lsp::QueryResponse::pushQueryResponse(
            ctx, core::lsp::DefinitionResponse(std::move(dispatchComponents), methodDef->declLoc, methodDef->name, tp));
    }

    return methodDef;
}
} // namespace sorbet::realmain::lsp
