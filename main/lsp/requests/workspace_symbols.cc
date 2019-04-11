#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

unique_ptr<SymbolInformation> LSPLoop::symbolRef2SymbolInformation(const core::GlobalState &gs,
                                                                   core::SymbolRef symRef) {
    auto sym = symRef.data(gs);
    if (!sym->loc().file().exists() || hideSymbol(gs, symRef)) {
        return nullptr;
    }

    auto result = make_unique<SymbolInformation>(sym->name.show(gs), symbolRef2SymbolKind(gs, symRef),
                                                 loc2Location(gs, sym->loc()));
    result->containerName = sym->owner.data(gs)->showFullName(gs);
    return result;
}

unique_ptr<core::GlobalState> LSPLoop::handleWorkspaceSymbols(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                                              const WorkspaceSymbolParams &params) {
    ResponseMessage response("2.0", id, LSPMethod::WorkspaceSymbol);
    if (!opts.lspWorkspaceSymbolsEnabled) {
        response.error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Workspace Symbols` LSP feature is experimental and disabled by default.");
        sendResponse(response);
        return gs;
    }

    prodCategoryCounterInc("lsp.messages.processed", "workspace.symbols");

    vector<unique_ptr<SymbolInformation>> result;
    string_view searchString = params.query;

    auto finalGs = move(gs);
    for (u4 idx = 1; idx < finalGs->symbolsUsed(); idx++) {
        core::SymbolRef ref(finalGs.get(), idx);
        if (hasSimilarName(*finalGs, ref.data(*finalGs)->name, searchString)) {
            auto data = symbolRef2SymbolInformation(*finalGs, ref);
            if (data) {
                result.push_back(move(data));
            }
        }
    }
    response.result = move(result);
    sendResponse(response);
    return finalGs;
}
} // namespace sorbet::realmain::lsp