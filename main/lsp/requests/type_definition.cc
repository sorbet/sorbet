#include "main/lsp/requests/type_definition.h"
#include "common/typecase.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
vector<core::Loc> locsForType(const core::GlobalState &gs, core::TypePtr type) {
    vector<core::Loc> result;
    if (type->isUntyped()) {
        return result;
    }
    typecase(
        type.get(), [&](core::ClassType *t) { result.emplace_back(t->symbol.data(gs)->loc()); },
        [&](core::AppliedType *t) { result.emplace_back(t->klass.data(gs)->loc()); },
        [&](core::OrType *t) {
            for (auto loc : locsForType(gs, t->left)) {
                result.emplace_back(loc);
            }
            for (auto loc : locsForType(gs, t->right)) {
                result.emplace_back(loc);
            }
        },
        [&](core::AndType *t) {
            for (auto loc : locsForType(gs, t->left)) {
                result.emplace_back(loc);
            }
            for (auto loc : locsForType(gs, t->right)) {
                result.emplace_back(loc);
            }
        });
    return result;
}
} // namespace

TypeDefinitionTask::TypeDefinitionTask(const LSPConfiguration &config, MessageId id,
                                       std::unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentTypeDefinition), params(move(params)) {}

unique_ptr<ResponseMessage> TypeDefinitionTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentTypeDefinition);
    prodCategoryCounterInc("lsp.messages.processed", "textDocument.typeDefinition");
    const core::GlobalState &gs = typechecker.state();
    auto result = queryByLoc(typechecker, params->textDocument->uri, *params->position,
                             LSPMethod::TextDocumentTypeDefinition, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        auto &queryResponses = result.responses;
        vector<unique_ptr<Location>> result;
        if (!queryResponses.empty()) {
            const bool fileIsTyped =
                config.uri2FileRef(gs, params->textDocument->uri).data(gs).strictLevel >= core::StrictLevel::True;
            auto resp = move(queryResponses[0]);

            // Only support go-to-type-definition on constants and fields in untyped files.
            if (resp->isConstant() || resp->isField() || (fileIsTyped && (resp->isIdent() || resp->isLiteral()))) {
                for (auto loc : locsForType(gs, resp->getRetType())) {
                    addLocIfExists(gs, result, loc);
                }
            } else if (fileIsTyped && resp->isSend()) {
                auto sendResp = resp->isSend();
                for (auto loc : locsForType(gs, sendResp->dispatchResult->returnType)) {
                    addLocIfExists(gs, result, loc);
                }
            }
        }
        response->result = move(result);
    }
    return response;
}
} // namespace sorbet::realmain::lsp
