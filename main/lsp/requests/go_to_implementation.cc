#include "main/lsp/requests/go_to_implementation.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include <memory>

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
    struct MethodImplementationResults {
        vector<core::Loc> locations;
        unique_ptr<ResponseError> error;
    };
} // namespace

unique_ptr<ResponseError> makeInvalidParamsError(std::string error) {
    return make_unique<ResponseError>((int)LSPErrorCodes::InvalidParams, error);
}

const MethodImplementationResults findMethodImplementations(const core::GlobalState &gs,
                                                                                core::SymbolRef method) {
    if (!method.data(gs)->isMethod() || !method.data(gs)->isAbstract()) {
        return {
            .error={makeInvalidParamsError("Go to implementation can be used only for methods or references of abstract classes")}
        };
    }

    vector<core::Loc> locations;
    auto owner = method.data(gs)->owner;
    if (!owner.isClassOrModule())
        return {
            .error={makeInvalidParamsError("Abstract method can only be inside a class or module")}
        };

    auto owningClassSymbolRef = owner.asClassOrModuleRef();
    auto childClasses = owningClassSymbolRef.getSubclasses(gs, false);
    auto methodName = method.data(gs)->name;
    for (const auto &childClass : childClasses) {
        auto methodImplementation = childClass.data(gs)->findMember(gs, methodName);
        locations.push_back(methodImplementation.data(gs)->loc());
    }

    return {.locations={locations}};
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
        if (!method.data(gs)->isMethod())
            response->error = make_unique<ResponseError>(
                (int)LSPErrorCodes::InvalidParams,
                "Go to implementation can be used only for methods or references of abstract classes");

        auto locationsOrError = findMethodImplementations(gs, method);

        if (locationsOrError.error != nullptr) {
            response->error = move(locationsOrError.error);
            return response;
        } else {
            for (const auto &location : locationsOrError.locations) {
                addLocIfExists(gs, result, location);
            }
        }
    } else if (auto constant = queryResponse->isConstant()) {
        // User called "Go to Implementation" from the abstract class reference
        auto classSymbol = constant->symbol;

        if (!classSymbol.data(gs)->isClassOrModule() || !classSymbol.data(gs)->isClassOrModuleAbstract()) {
            response->error = make_unique<ResponseError>(
                (int)LSPErrorCodes::InvalidParams,
                "Go to implementation can be used only for methods or references of abstract classes");
            return response;
        }

        auto classOrModuleRef = classSymbol.asClassOrModuleRef();
        auto childClasses = classOrModuleRef.getSubclasses(gs, false);
        for (const auto &childClass : childClasses) {
            for (auto loc : childClass.data(gs)->allLocs()) {
              addLocIfExists(gs, result, loc);
            }
        }

    } else if (auto send = queryResponse->isSend()) {
        // User called "Go to Implementation" from the abstract function call
        auto calledMethod = send->dispatchResult->main.method;
        auto locationsOrError = findMethodImplementations(gs, calledMethod);

        if (locationsOrError.error != nullptr) {
            response->error = move(locationsOrError.error);
            return response;
        } else {
            for (const auto &location : locationsOrError.locations) {
                addLocIfExists(gs, result, location);
            }
        }
    }

    response->result = move(result);
    return response;
}
} // namespace sorbet::realmain::lsp
