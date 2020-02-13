#include "main/lsp/requests/hover.h"
#include "absl/strings/ascii.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

string methodInfoString(const core::GlobalState &gs, const core::TypePtr &retType,
                        const core::DispatchResult &dispatchResult,
                        const unique_ptr<core::TypeConstraint> &constraint) {
    string contents;
    auto start = &dispatchResult;
    ;
    while (start != nullptr) {
        auto &component = start->main;
        if (component.method.exists()) {
            if (!contents.empty()) {
                contents += "\n";
            }
            contents = absl::StrCat(
                contents, prettyTypeForMethod(gs, component.method, component.receiver, retType, constraint.get()));
        }
        start = start->secondary.get();
    }

    return contents;
}

HoverTask::HoverTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentHover), params(move(params)) {}

unique_ptr<ResponseMessage> HoverTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentHover);
    prodCategoryCounterInc("lsp.messages.processed", "textDocument.hover");

    const core::GlobalState &gs = typechecker.state();
    auto result = queryByLoc(typechecker, params->textDocument->uri, *params->position, LSPMethod::TextDocumentHover);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        auto &queryResponses = result.responses;
        if (queryResponses.empty()) {
            // Note: Need to specifically specify the variant type here so the null gets placed into the proper slot.
            response->result = variant<JSONNullObject, unique_ptr<Hover>>(JSONNullObject());
            return response;
        }

        auto resp = move(queryResponses[0]);

        optional<string> documentation = nullopt;
        if (resp->isConstant() || resp->isField() || resp->isDefinition()) {
            auto origins = resp->getTypeAndOrigins().origins;
            if (!origins.empty()) {
                auto loc = origins[0];
                if (loc.exists()) {
                    documentation = findDocumentation(loc.file().data(gs).source(), loc.beginPos());
                }
            }
        }

        auto clientHoverMarkupKind = config.getClientConfig().clientHoverMarkupKind;
        if (auto sendResp = resp->isSend()) {
            auto retType = sendResp->dispatchResult->returnType;
            auto start = sendResp->dispatchResult.get();
            if (start != nullptr && start->main.method.exists() && !start->main.receiver->isUntyped()) {
                auto loc = start->main.method.data(gs)->loc();
                if (loc.exists()) {
                    documentation = findDocumentation(loc.file().data(gs).source(), loc.beginPos());
                }
            }
            auto &constraint = sendResp->dispatchResult->main.constr;
            if (constraint) {
                retType = core::Types::instantiate(core::Context(gs, core::Symbols::root()), retType, *constraint);
            }
            string typeString;
            if (sendResp->dispatchResult->main.method.exists() && sendResp->dispatchResult->main.method.isSynthetic()) {
                // For synthetic methods, just show the return type
                typeString = retType->showWithMoreInfo(gs);
            } else {
                typeString = methodInfoString(gs, retType, *sendResp->dispatchResult, constraint);
            }
            response->result = make_unique<Hover>(formatRubyMarkup(clientHoverMarkupKind, typeString, documentation));
        } else if (auto defResp = resp->isDefinition()) {
            string typeString = prettyTypeForMethod(gs, defResp->symbol, nullptr, defResp->retType.type, nullptr);
            response->result = make_unique<Hover>(formatRubyMarkup(clientHoverMarkupKind, typeString, documentation));
        } else if (auto constResp = resp->isConstant()) {
            auto prettyType = prettyTypeForConstant(gs, constResp->symbol);
            response->result = make_unique<Hover>(formatRubyMarkup(clientHoverMarkupKind, prettyType, documentation));
        } else {
            core::TypePtr retType = resp->getRetType();
            // Some untyped arguments have null types.
            if (!retType) {
                retType = core::Types::untypedUntracked();
            }
            response->result = make_unique<Hover>(
                formatRubyMarkup(clientHoverMarkupKind, retType->showWithMoreInfo(gs), documentation));
        }
    }
    return response;
}
} // namespace sorbet::realmain::lsp
