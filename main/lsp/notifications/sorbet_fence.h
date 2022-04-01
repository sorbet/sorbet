#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_FENCE_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_FENCE_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class SorbetFenceParams;
class SorbetFenceTask final : public LSPTask {
    std::unique_ptr<SorbetFenceParams> params;

public:
    SorbetFenceTask(const LSPConfiguration &config, std::unique_ptr<SorbetFenceParams> params);

    bool canPreempt(const LSPIndexer &indexer) const override;

    bool canUseStaleData() const override;

    void run(LSPTypecheckerInterface &tc) override;
};
} // namespace sorbet::realmain::lsp

#endif
