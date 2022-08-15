#ifndef SORBET_LSP_QUERY_H
#define SORBET_LSP_QUERY_H

#include "main/lsp/LSPTypechecker.h"

namespace sorbet::realmain::lsp {

class LSPQuery {
public:
    static std::vector<std::unique_ptr<core::lsp::QueryResponse>>
    filterAndDedup(const core::GlobalState &gs,
                   const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &queryResponses);

    static LSPQueryResult byLoc(const LSPConfiguration &config, LSPTypecheckerInterface &typechecker,
                                std::string_view uri, const Position &pos, LSPMethod forMethod,
                                bool errorIfFileIsUntyped = true);
    static LSPQueryResult bySymbolInFiles(const LSPConfiguration &config, LSPTypecheckerInterface &typechecker,
                                          core::SymbolRef symbol, std::vector<core::FileRef> frefs);
    static LSPQueryResult bySymbol(const LSPConfiguration &config, LSPTypecheckerInterface &typechecker,
                                   core::SymbolRef symbol, core::NameRef pkgName = core::NameRef::noName());
};

} // namespace sorbet::realmain::lsp

#endif
