#ifndef SORBET_LSP_QUERY_H
#define SORBET_LSP_QUERY_H

#include "main/lsp/LSPTypechecker.h"

namespace sorbet::realmain::lsp {

class LSPQuery {
public:
    static std::vector<std::unique_ptr<core::lsp::QueryResponse>>
    filterAndDedup(const core::GlobalState &gs,
                   const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &queryResponses);

    static LSPQueryResult byLoc(const LSPConfiguration &config, LSPTypecheckerDelegate &typechecker,
                                std::string_view uri, const Position &pos, LSPMethod forMethod,
                                bool emptyResultIfFileIsUntyped = true);
    static LSPQueryResult bySymbolInFiles(const LSPConfiguration &config, LSPTypecheckerDelegate &typechecker,
                                          core::SymbolRef symbol, std::vector<core::FileRef> frefs);
    static LSPQueryResult bySymbol(const LSPConfiguration &config, LSPTypecheckerDelegate &typechecker,
                                   core::SymbolRef symbol,
                                   core::packages::MangledName pkgName = core::packages::MangledName());
};

} // namespace sorbet::realmain::lsp

#endif
