#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_FENCE_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_FENCE_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class SorbetFenceTask final : public LSPTask {
    int id;

public:
    SorbetFenceTask(const LSPConfiguration &config, int id);

    bool canPreempt(const LSPIndexer &indexer) const override;

    void run(LSPTypecheckerDelegate &tc) override;
};
} // namespace sorbet::realmain::lsp

#endif
