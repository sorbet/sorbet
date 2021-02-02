#include "main/lsp/requests/definition.h"
#include "common/FileOps.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include <filesystem>

using namespace std;

namespace sorbet::realmain::lsp {
DefinitionTask::DefinitionTask(const LSPConfiguration &config, MessageId id,
                               unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentDefinition), params(move(params)) {}

core::Loc
DefinitionTask::findRequireRelativeLoc(const core::GlobalState &gs,
                                       const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &responses) {
    /* To detect a require_relative situation we have to match two individual
     * entries from the query response list. The first one is the literal value
     * that's passed in the function and will help us compute the final path
     * we should navigate to. The second one is the `require_relative` call itself.
     */
    if (responses.size() < 2) {
        return core::Loc::none();
    }

    auto parentSendResp = responses[1]->isSend();
    // Match the send response to make sure we are dealing with a top-level `require_relative` call
    const bool isRequireRelative = parentSendResp && parentSendResp->isPrivateOk &&
                                   parentSendResp->callerSideName == core::Names::require_relative();
    auto literal = responses[0]->isLiteral();
    if (isRequireRelative && literal) {
        auto literalValue = core::cast_type_nonnull<core::LiteralType>(literal->retType.type).asName(gs).shortName(gs);
        auto baseFilePath = std::filesystem::path(literal->termLoc.file().data(gs).path());
        auto targetFilePath = baseFilePath.replace_filename(literalValue).replace_extension(".rb").lexically_normal();
        auto targetFileRef = gs.findFileByPath(targetFilePath.string());
        if (targetFileRef.exists()) {
            auto loc = core::Loc(targetFileRef, 0, 0);
            return loc;
        }
    }
    return core::Loc::none();
}

unique_ptr<ResponseMessage> DefinitionTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentDefinition);
    const core::GlobalState &gs = typechecker.state();
    auto result =
        queryByLoc(typechecker, params->textDocument->uri, *params->position, LSPMethod::TextDocumentDefinition, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        auto &queryResponses = result.responses;
        vector<unique_ptr<Location>> result;

        // First, special case detecting require_relative and in that case go to the referenced file.
        auto requireLoc = findRequireRelativeLoc(gs, queryResponses);

        if (requireLoc.exists()) {
            addLocIfExists(gs, result, requireLoc);
        } else if (!queryResponses.empty()) {
            const bool fileIsTyped =
                config.uri2FileRef(gs, params->textDocument->uri).data(gs).strictLevel >= core::StrictLevel::True;
            auto resp = move(queryResponses[0]);

            // Only support go-to-definition on constants and fields in untyped files.
            if (auto c = resp->isConstant()) {
                auto sym = c->symbol;
                for (auto loc : sym.data(gs)->locs()) {
                    addLocIfExists(gs, result, loc);
                }
            } else if (resp->isField() || (fileIsTyped && (resp->isIdent() || resp->isLiteral()))) {
                auto retType = resp->getTypeAndOrigins();
                for (auto &originLoc : retType.origins) {
                    addLocIfExists(gs, result, originLoc);
                }
            } else if (fileIsTyped && resp->isDefinition()) {
                auto sym = resp->isDefinition()->symbol;
                for (auto loc : sym.data(gs)->locs()) {
                    addLocIfExists(gs, result, loc);
                }
            } else if (fileIsTyped && resp->isSend()) {
                auto sendResp = resp->isSend();
                auto start = sendResp->dispatchResult.get();
                while (start != nullptr) {
                    if (start->main.method.exists() && !start->main.receiver.isUntyped()) {
                        addLocIfExists(gs, result, start->main.method.data(gs)->loc());
                    }
                    start = start->secondary.get();
                }
            }
        }
        response->result = move(result);
    }
    return response;
}

} // namespace sorbet::realmain::lsp
