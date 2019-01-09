#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
unique_ptr<core::GlobalState> LSPLoop::handleTextDocumentHover(unique_ptr<core::GlobalState> gs,
                                                               rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.hover");

    auto finalGs = move(gs);
    auto run = setupLSPQueryByLoc(move(finalGs), d, LSPMethod::TextDocumentHover(), false);
    finalGs = move(run.gs);
    auto &queryResponses = run.responses;
    if (queryResponses.empty()) {
        rapidjson::Value nullValue;
        sendResult(d, nullValue);
        return finalGs;
    }

    auto resp = move(queryResponses[0]);
    if (resp->kind == core::QueryResponse::Kind::SEND) {
        if (resp->dispatchComponents.empty()) {
            sendError(d, (int)LSPErrorCodes::InvalidParams,
                      "Did not find any dispatchComponents for a SEND QueryResponse in "
                      "textDocument/hover");
            return finalGs;
        }
        string contents = "";
        for (auto &dispatchComponent : resp->dispatchComponents) {
            auto retType = resp->retType.type;
            if (resp->constraint) {
                retType = core::Types::instantiate(core::Context(*finalGs, core::Symbols::root()), retType,
                                                   *resp->constraint);
            }
            if (dispatchComponent.method.exists()) {
                if (contents.size() > 0) {
                    contents += " ";
                }
                contents += methodDetail(*finalGs, dispatchComponent.method, dispatchComponent.receiver, retType,
                                         resp->constraint);
            }
        }
        // We use markdown here because if we just use a string, VSCode tries to interpret
        // things like <Class:Foo> as html tags and make them clickable (but the click takes
        // you somewhere nonsensical)
        auto markupContents = make_unique<MarkupContent>(MarkupKind::Markdown, contents);
        sendResult(d, Hover(markupContents->toJSONValueInternal(d)));
    } else if (resp->kind == core::QueryResponse::Kind::IDENT || resp->kind == core::QueryResponse::Kind::CONSTANT ||
               resp->kind == core::QueryResponse::Kind::LITERAL) {
        auto markupContents = make_unique<MarkupContent>(MarkupKind::Markdown, resp->retType.type->show(*finalGs));
        sendResult(d, Hover(markupContents->toJSONValueInternal(d)));
    } else if (resp->kind == core::QueryResponse::Kind::DEFINITION) {
        // TODO: Actually send the type signature here. I'm skipping this for now
        // since it's not a very useful feature for the end user (i.e., they should
        // be able to see this right above the definition in ruby)
        rapidjson::Value nullValue;
        sendResult(d, nullValue);
    } else {
        sendError(d, (int)LSPErrorCodes::InvalidParams, "Unhandled QueryResponse kind in textDocument/hover");
    }
    return finalGs;
}
} // namespace sorbet::realmain::lsp
