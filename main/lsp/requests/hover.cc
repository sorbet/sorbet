#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet {
namespace realmain {
namespace lsp {
void LSPLoop::handleTextDocumentHover(rapidjson::Value &result, rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.hover");
    result.SetObject();

    if (setupLSPQueryByLoc(d, LSPMethod::TextDocumentHover(), false)) {
        auto queryResponses = errorQueue->drainQueryResponses();
        if (queryResponses.empty()) {
            rapidjson::Value nullreply;
            sendResult(d, nullreply);
            return;
        }

        auto resp = move(queryResponses[0]);
        if (resp->kind == core::QueryResponse::Kind::SEND) {
            if (resp->dispatchComponents.empty()) {
                sendError(d, (int)LSPErrorCodes::InvalidParams,
                          "Did not find any dispatchComponents for a SEND QueryResponse in "
                          "textDocument/hover");
                return;
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
                    contents +=
                        methodDetail(dispatchComponent.method, dispatchComponent.receiver, retType, resp->constraint);
                }
            }
            rapidjson::Value markupContents;
            markupContents.SetObject();
            // We use markdown here because if we just use a string, VSCode tries to interpret
            // things like <Class:Foo> as html tags and make them clickable (but the click takes
            // you somewhere nonsensical)
            markupContents.AddMember("kind", "markdown", alloc);
            markupContents.AddMember("value", contents, alloc);
            result.AddMember("contents", markupContents, alloc);
            sendResult(d, result);
        } else if (resp->kind == core::QueryResponse::Kind::IDENT ||
                   resp->kind == core::QueryResponse::Kind::CONSTANT ||
                   resp->kind == core::QueryResponse::Kind::LITERAL) {
            rapidjson::Value markupContents;
            markupContents.SetObject();
            markupContents.AddMember("kind", "markdown", alloc);
            markupContents.AddMember("value", resp->retType.type->show(*finalGs), alloc);
            result.AddMember("contents", markupContents, alloc);
            sendResult(d, result);
        } else {
            sendError(d, (int)LSPErrorCodes::InvalidParams, "Unhandled QueryResponse kind in textDocument/hover");
        }
    }
}
} // namespace lsp
} // namespace realmain
} // namespace sorbet