#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_INDEXER_INITIALIZATION_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_INDEXER_INITIALIZATION_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {

class IndexerInitializationTask final : public LSPTask {
    std::vector<std::shared_ptr<core::File>> files;

public:
    IndexerInitializationTask(const LSPConfiguration &config, std::vector<std::shared_ptr<core::File>> &&files);

    Phase finalPhase() const override;

    void index(LSPIndexer &indexer) override;

    void run(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
