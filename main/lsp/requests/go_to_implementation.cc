#include "main/lsp/requests/go_to_implementation.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

vector<core::SymbolRef> findChildren(const core::GlobalState &gs, core::SymbolRef ancestorClass) {
    vector<core::SymbolRef> results;
    auto used = gs.classAndModulesUsed();
    for (u4 idx = 1; idx < used; idx++) {
        core::SymbolRef ref(gs, core::SymbolRef::Kind::ClassOrModule, idx);
        if (ref.data(gs)->derivesFrom(gs, ancestorClass)) {
            results.push_back(ref);
        }
    }
    return results;
}

variant<vector<core::Loc>, unique_ptr<ResponseError>> findMethodImplementations(const core::GlobalState &gs,
                                                                                core::SymbolRef method) {
    vector<core::Loc> locations;
    auto owningClassSymbolRef = method.data(gs)->owner;
    auto isAbstract = owningClassSymbolRef.data(gs)->isClassOrModuleAbstract();
    if (!isAbstract) {
        return make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidParams,
            "Go to implementation can be used only for methods or references of abstract classes");
    }

    auto childClasses = findChildren(gs, owningClassSymbolRef);
    auto methodName = method.data(gs)->name;
    for (const auto &childClass : childClasses) {
        auto methodImplementation = childClass.data(gs)->findMember(gs, methodName);
        locations.push_back(methodImplementation.data(gs)->loc());
    }

    return locations;
}

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

    if (queryResult.responses.empty()) {
        return response;
    }

    vector<unique_ptr<Location>> result;
    auto queryResponse = move(queryResult.responses[0]);
    if (auto def = queryResponse->isDefinition()) {
        // User called "Go to Implementation" from the abstract function definition
        core::SymbolRef method = def->symbol;
        auto locationsOrError = findMethodImplementations(gs, method);

        if (auto error = get_if<unique_ptr<ResponseError>>(&locationsOrError)) {
            response->error = move(*error);
            return response;
        } else if (auto locations = get_if<vector<core::Loc>>(&locationsOrError)) {
            for (const auto &location : *locations) {
                addLocIfExists(gs, result, location);
            }
        }
    } else if (auto constant = queryResponse->isConstant()) {
        // User called "Go to Implementation" from the abstract class reference
        auto classSymbol = constant->symbol;

        if (!classSymbol.data(gs)->isClassOrModuleAbstract()) {
            response->error = make_unique<ResponseError>(
                (int)LSPErrorCodes::InvalidParams,
                "Go to implementation can be used only for methods or references of abstract classes");
            return response;
        }

        auto childClasses = findChildren(gs, classSymbol);
        for (const auto &childClass : childClasses) {
            addLocIfExists(gs, result, childClass.data(gs)->loc());
        }

    } else if (auto send = queryResponse->isSend()) {
        // User called "Go to Implementation" from the abstract function call
        auto calledMethod = send->dispatchResult->main.method;
        auto locationsOrError = findMethodImplementations(gs, calledMethod);

        if (auto error = get_if<unique_ptr<ResponseError>>(&locationsOrError)) {
            response->error = move(*error);
            return response;
        } else if (auto locations = get_if<vector<core::Loc>>(&locationsOrError)) {
            for (const auto &location : *locations) {
                addLocIfExists(gs, result, location);
            }
        }
    }

    response->result = move(result);
    return response;
}
} // namespace sorbet::realmain::lsp