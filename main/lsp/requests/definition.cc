#include "main/lsp/requests/definition.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
DefinitionTask::DefinitionTask(const LSPConfiguration &config, MessageId id,
                               unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentDefinition), params(move(params)) {}

unique_ptr<ResponseMessage> DefinitionTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentDefinition);
    const core::GlobalState &gs = typechecker.state();
    auto result = LSPQuery::byLoc(config, typechecker, params->textDocument->uri, *params->position,
                                  LSPMethod::TextDocumentDefinition, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto &queryResponses = result.responses;
    vector<unique_ptr<Location>> locations;
    if (!queryResponses.empty()) {
        const bool fileIsTyped =
            config.uri2FileRef(gs, params->textDocument->uri).data(gs).strictLevel >= core::StrictLevel::True;
        auto resp = move(queryResponses[0]);

        // Only support go-to-definition on constants and fields in untyped files.
        if (auto c = resp->isConstant()) {
            auto sym = c->symbol;
            vector<pair<core::Loc, unique_ptr<Location>>> locMapping;
            for (auto loc : sym.locs(gs)) {
                locMapping.emplace_back(loc, config.loc2Location(gs, loc));
            }
            // Move all non-existent Locations to the front of the vector.
            auto validLocations = absl::c_partition(locMapping, [](const auto &p) { return p.second == nullptr; });
            // If we have multiple locations, eliminate "definitions" in RBI files
            // for classes and modules, since the one(s) in Ruby files are more
            // likely to be what the user is looking for.
            if (sym.isClassOrModule() && std::distance(validLocations, locMapping.end()) > 1) {
                validLocations = std::partition(validLocations, locMapping.end(),
                                                [&gs](const auto &p) { return p.first.file().data(gs).isRBI(); });
            }
            std::transform(validLocations, locMapping.end(), std::back_inserter(locations),
                           [](auto &p) { return std::move(p.second); });
        } else if (resp->isField() || (fileIsTyped && (resp->isIdent() || resp->isLiteral()))) {
            const auto &retType = resp->getTypeAndOrigins();
            for (auto &originLoc : retType.origins) {
                addLocIfExists(gs, locations, originLoc);
            }
        } else if (fileIsTyped && resp->isMethodDef()) {
            auto sym = resp->isMethodDef()->symbol;
            for (auto loc : sym.data(gs)->locs()) {
                addLocIfExists(gs, locations, loc);
            }
        } else if (fileIsTyped && resp->isSend()) {
            auto sendResp = resp->isSend();
            auto start = sendResp->dispatchResult.get();
            while (start != nullptr) {
                if (start->main.method.exists() && !start->main.receiver.isUntyped()) {
                    addLocIfExists(gs, locations, start->main.method.data(gs)->loc());
                }
                start = start->secondary.get();
            }
        }
    }
    response->result = move(locations);
    return response;
}

} // namespace sorbet::realmain::lsp
