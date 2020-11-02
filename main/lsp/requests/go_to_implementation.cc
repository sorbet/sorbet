#include "main/lsp/requests/go_to_implementation.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

GoToImplementationTask::GoToImplementationTask(const LSPConfiguration &config, MessageId id,
                                               std::unique_ptr<ImplementationParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentImplementation), params(move(params)) {}

unique_ptr<ResponseMessage> GoToImplementationTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentImplementation);

    const core::GlobalState &gs = typechecker.state();
    auto queryResult =
        queryByLoc(typechecker, params->textDocument->uri, *params->position, LSPMethod::TextDocumentImplementation);

    if (queryResult.error) {
        // An error happened while setting up the query.
        response->error = move(queryResult.error);
        return response;
    }

    vector<unique_ptr<Location>> result;
    for (auto const &queryResponse : queryResult.responses) {
        auto def = queryResponse->isDefinition();
        if (def == nullptr) {
            continue;
        }
        auto owningClassSymbolRef = def->symbol.data(gs)->owner;
        auto isAbstract = owningClassSymbolRef.data(gs)->isClassOrModuleAbstract();
        if (!isAbstract) {
            response->error = make_unique<ResponseError>(
                (int)LSPErrorCodes::InvalidParams, "Go to definition can be used only for methods of abstract classes");
        }

        auto used = gs.classAndModulesUsed();
        for (u4 idx = 1; idx < used; idx++) {
            core::SymbolRef ref(gs, core::SymbolRef::Kind::ClassOrModule, idx);
            if (ref.data(gs)->derivesFrom(gs, owningClassSymbolRef)) {
                auto members = ref.data(gs)->members();

                for (auto const &pair : members) {
                    if (pair.first == def->name) {
                        addLocIfExists(gs, result, pair.second.data(gs)->loc());
                    }
                }
            }
        }
    }

    response->result = move(result);
    return response;
}
} // namespace sorbet::realmain::lsp