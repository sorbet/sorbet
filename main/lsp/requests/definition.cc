#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
void LSPLoop::addLocIfExists(const core::GlobalState &gs, rapidjson::Value &result, core::Loc loc) {
    if (loc.file().exists()) {
        result.PushBack(loc2Location(gs, loc), alloc);
    }
}

unique_ptr<core::GlobalState> LSPLoop::handleTextDocumentDefinition(unique_ptr<core::GlobalState> gs,
                                                                    rapidjson::Value &result, rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.definition");
    result.SetArray();
    auto finalGs = move(gs);
    auto run = setupLSPQueryByLoc(move(finalGs), d, LSPMethod::TextDocumentDefinition(), true);
    finalGs = move(run.gs);
    auto &queryResponses = run.responses;
    if (!queryResponses.empty()) {
        auto resp = move(queryResponses[0]);

        if (resp->kind == core::QueryResponse::Kind::IDENT) {
            for (auto &originLoc : resp->retType.origins) {
                addLocIfExists(*finalGs, result, originLoc);
            }
        } else if (resp->kind == core::QueryResponse::Kind::DEFINITION) {
            result.PushBack(loc2Location(*finalGs, resp->termLoc), alloc);
        } else {
            for (auto &component : resp->dispatchComponents) {
                if (component.method.exists()) {
                    addLocIfExists(*finalGs, result, component.method.data(*finalGs)->loc());
                }
            }
        }
    }

    sendResult(d, result);
    return finalGs;
}

} // namespace sorbet::realmain::lsp
