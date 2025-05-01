#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_INDEXER_INITIALIZATION_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_INDEXER_INITIALIZATION_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {

class IndexerInitializationTask final : public LSPTask {
public:
    IndexerInitializationTask(const LSPConfiguration &config);

    Phase finalPhase() const override;

    void index(LSPIndexer &indexer) override;

    void run(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
