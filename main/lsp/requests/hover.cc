#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"
#include "main/options/options.h"

using namespace std;

namespace sorbet::realmain::lsp {
LSPResult LSPLoop::handleTextDocumentHover(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const TextDocumentPositionParams &params) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentHover);
    if (!state.opts.lspHoverEnabled) {
        response->error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidRequest, "The `Hover` LSP feature is experimental and disabled by default.");
        return LSPResult::make(move(gs), move(response));
    }

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.hover");

    auto result =
        setupLSPQueryByLoc(move(gs), params.textDocument->uri, *params.position, LSPMethod::TextDocumentHover, false);
    if (auto run = get_if<TypecheckRun>(&result)) {
        gs = move(run->gs);
        auto &queryResponses = run->responses;
        if (queryResponses.empty()) {
            // Note: Need to specifically specify the variant type here so the null gets placed into the proper slot.
            response->result = variant<JSONNullObject, unique_ptr<Hover>>(JSONNullObject());
            return LSPResult::make(move(gs), move(response));
        }

        auto resp = move(queryResponses[0]);
        if (auto sendResp = resp->isSend()) {
            if (sendResp->dispatchComponents.empty()) {
                response->error = make_unique<ResponseError>(
                    (int)LSPErrorCodes::InvalidParams,
                    "Did not find any dispatchComponents for a SEND QueryResponse in textDocument/hover");
                return LSPResult::make(move(gs), move(response));
            }
            string contents = "";
            auto retType = sendResp->retType.type;
            auto &constraint = sendResp->constraint;
            for (auto &dispatchComponent : sendResp->dispatchComponents) {
                if (constraint) {
                    retType = core::Types::instantiate(core::Context(*gs, core::Symbols::root()), retType, *constraint);
                }
                if (dispatchComponent.method.exists()) {
                    if (contents.size() > 0) {
                        contents += " ";
                    }
                    contents +=
                        methodDetail(*gs, dispatchComponent.method, dispatchComponent.receiver, retType, constraint);
                }
            }
            // We use markdown here because if we just use a string, VSCode tries to interpret
            // things like <Class:Foo> as html tags and make them clickable (but the click takes
            // you somewhere nonsensical)
            auto markupContents = make_unique<MarkupContent>(MarkupKind::Markdown, contents);
            response->result = make_unique<Hover>(move(markupContents));
        } else if (auto defResp = resp->isDefinition()) {
            // TODO: Actually send the type signature here. I'm skipping this for now
            // since it's not a very useful feature for the end user (i.e., they should
            // be able to see this right above the definition in ruby)
            response->result = variant<JSONNullObject, unique_ptr<Hover>>(JSONNullObject());
        } else {
            auto markupContents = make_unique<MarkupContent>(MarkupKind::Markdown, resp->getRetType()->show(*gs));
            response->result = make_unique<Hover>(move(markupContents));
        }
    } else if (auto error = get_if<pair<unique_ptr<ResponseError>, unique_ptr<core::GlobalState>>>(&result)) {
        // An error happened while setting up the query.
        response->error = move(error->first);
        gs = move(error->second);
    } else {
        // Should never happen, but satisfy the compiler.
        ENFORCE(false, "Internal error: setupLSPQueryByLoc returned invalid value.");
    }
    return LSPResult::make(move(gs), move(response));
}
} // namespace sorbet::realmain::lsp
