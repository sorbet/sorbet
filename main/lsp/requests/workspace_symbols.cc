#include "core/lsp/QueryResponse.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

/**
 * Converts a symbol into a SymbolInformation object.
 * Returns `nullptr` if symbol kind is not supported by LSP
 */
unique_ptr<SymbolInformation> symbolRef2SymbolInformation(const LSPConfiguration &config, const core::GlobalState &gs,
                                                          core::SymbolRef symRef) {
    auto sym = symRef.data(gs);
    if (!sym->loc().file().exists() || hideSymbol(gs, symRef)) {
        return nullptr;
    }

    auto location = config.loc2Location(gs, sym->loc());
    if (location == nullptr) {
        return nullptr;
    }
    auto result =
        make_unique<SymbolInformation>(sym->name.show(gs), symbolRef2SymbolKind(gs, symRef), std::move(location));
    result->containerName = sym->owner.data(gs)->showFullName(gs);
    return result;
}

LSPResult LSPLoop::handleWorkspaceSymbols(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                          const WorkspaceSymbolParams &params) const {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::WorkspaceSymbol);
    if (!config->opts.lspWorkspaceSymbolsEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Workspace Symbols` LSP feature is experimental and disabled by default.");
        return LSPResult::make(move(gs), move(response));
    }

    prodCategoryCounterInc("lsp.messages.processed", "workspace.symbols");

    vector<unique_ptr<SymbolInformation>> result;
    string_view searchString = params.query;
    ShowOperation op(*config, "WorkspaceSymbols", fmt::format("Searching for symbol `{}`...", searchString));

    for (u4 idx = 1; idx < gs->symbolsUsed(); idx++) {
        core::SymbolRef ref(gs.get(), idx);
        if (hasSimilarName(*gs, ref.data(*gs)->name, searchString)) {
            auto data = symbolRef2SymbolInformation(*config, *gs, ref);
            if (data) {
                result.push_back(move(data));
            }
        }
    }
    response->result = move(result);
    return LSPResult::make(move(gs), move(response));
}
} // namespace sorbet::realmain::lsp
