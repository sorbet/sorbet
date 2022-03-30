#ifndef SORBET_LSP_QUERY_H
#define SORBET_LSP_QUERY_H

#include "main/lsp/LSPTypechecker.h"

namespace sorbet::realmain::lsp {

std::vector<std::unique_ptr<core::lsp::QueryResponse>>
filterAndDedup(const core::GlobalState &gs,
               const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &queryResponses);

LSPQueryResult queryByLoc(const LSPConfiguration &config, LSPTypecheckerInterface &typechecker, std::string_view uri,
                          const Position &pos, LSPMethod forMethod, bool errorIfFileIsUntyped = true);
LSPQueryResult queryBySymbolInFiles(const LSPConfiguration &config, LSPTypecheckerInterface &typechecker,
                                    core::SymbolRef symbol, std::vector<core::FileRef> frefs);
LSPQueryResult queryBySymbol(const LSPConfiguration &config, LSPTypecheckerInterface &typechecker,
                             core::SymbolRef symbol);

} // namespace sorbet::realmain::lsp

#endif
