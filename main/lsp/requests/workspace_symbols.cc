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

LSPResult LSPLoop::handleWorkspaceSymbols(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                          const WorkspaceSymbolParams &params) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::WorkspaceSymbol);
    if (!opts.lspWorkspaceSymbolsEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Workspace Symbols` LSP feature is experimental and disabled by default.");
        return LSPResult::make(move(gs), move(response));
    }

    prodCategoryCounterInc("lsp.messages.processed", "workspace.symbols");

    vector<unique_ptr<SymbolInformation>> result;
    string_view searchString = params.query;

    for (u4 idx = 1; idx < gs->symbolsUsed(); idx++) {
        core::SymbolRef ref(gs.get(), idx);
        if (hasSimilarName(*gs, ref.data(*gs)->name, searchString)) {
            auto data = symbolRef2SymbolInformation(*gs, ref);
            if (data) {
                result.push_back(move(data));
            }
        }
    }
    response->result = move(result);
    return LSPResult::make(move(gs), move(response));
}
} // namespace sorbet::realmain::lsp