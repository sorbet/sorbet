#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_INDEXER_INITIALIZED_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_INDEXER_INITIALIZED_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {

class IndexerInitializedTask final : public LSPTask {
    std::unique_ptr<core::GlobalState> initialGS;

public:
    IndexerInitializedTask(const LSPConfiguration &config);

    void setIndexerState(std::unique_ptr<core::GlobalState> initialGS);

    Phase finalPhase() const override;

    void index(LSPIndexer &indexer) override;

    void run(LSPTypecheckerInterface &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
