#ifndef RUBY_TYPER_LSPTASK_H
#define RUBY_TYPER_LSPTASK_H

#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPTypecheckerCoordinator.h"

namespace sorbet::realmain::lsp {
/**
 * A work unit that needs to execute on the typechecker thread. Subclasses implement `run`.
 * Contains miscellaneous helper methods that are useful in multiple tasks.
 *
 * NOTE: If `enableMultithreading` is set to `true`, then this task cannot preempt slow path typechecking.
 */
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

    LSPTask(const LSPConfiguration &config, bool enableMultithreading);

public:
    const bool enableMultithreading;

    virtual ~LSPTask() = default;

    // Runs the task. Is only ever invoked from the typechecker thread.
    virtual void run(LSPTypecheckerDelegate &typechecker) = 0;
};

/**
 * A specialized version of LSPTask for LSP requests (which must be responded to).
 */
class LSPRequestTask : public LSPTask {
protected:
    const MessageId id;

    LSPRequestTask(const LSPConfiguration &config, MessageId id, bool enableMultithreading = false);

    virtual std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) = 0;

public:
    void run(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif