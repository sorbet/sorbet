#ifndef RUBY_TYPER_LSPTASK_H
#define RUBY_TYPER_LSPTASK_H

#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPTypecheckerCoordinator.h"

namespace sorbet::realmain::lsp {
class LSPTask {
protected:
    const LSPConfiguration &config;

    // Task helper methods.

    std::vector<std::unique_ptr<Location>>
    getReferencesToSymbol(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol,
                          std::vector<std::unique_ptr<Location>> locations = {}) const;
    std::vector<std::unique_ptr<DocumentHighlight>>
    getHighlightsToSymbolInFile(LSPTypecheckerDelegate &typechecker, std::string_view uri, core::SymbolRef symbol,
                                std::vector<std::unique_ptr<DocumentHighlight>> highlights = {}) const;
    void addLocIfExists(const core::GlobalState &gs, std::vector<std::unique_ptr<Location>> &locs, core::Loc loc) const;
    std::vector<std::unique_ptr<Location>>
    extractLocations(const core::GlobalState &gs,
                     const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                     std::vector<std::unique_ptr<Location>> locations = {}) const;

    LSPQueryResult queryByLoc(LSPTypecheckerDelegate &typechecker, std::string_view uri, const Position &pos,
                              LSPMethod forMethod, bool errorIfFileIsUntyped = true) const;
    LSPQueryResult queryBySymbolInFiles(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol,
                                        std::vector<core::FileRef> frefs) const;
    LSPQueryResult queryBySymbol(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol) const;

    LSPTask(const LSPConfiguration &config);

public:
    virtual ~LSPTask() = default;
    virtual void run(LSPTypecheckerDelegate &delegate) = 0;
};

// A specialized version of LSPTask for LSP requests (which must be responded to).
class LSPRequestTask : public LSPTask {
protected:
    const MessageId id;

    LSPRequestTask(const LSPConfiguration &config, MessageId id);

    virtual std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &delegate) = 0;

public:
    void run(LSPTypecheckerDelegate &delegate) override;
};

} // namespace sorbet::realmain::lsp

#endif