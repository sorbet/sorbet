#ifndef RUBY_TYPER_LSP_REQUESTS_GET_COUNTERS_H
#define RUBY_TYPER_LSP_REQUESTS_GET_COUNTERS_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class GetCountersTask final : public LSPTask {
    MessageId id;

public:
    GetCountersTask(const LSPConfiguration &config, MessageId id);

    bool canPreempt(const LSPIndexer &) const override;

    LSPTask::Phase finalPhase() const override;

    void index(LSPIndexer &indexer) override;

    void run(LSPTypecheckerDelegate &typechecker) override;
};
} // namespace sorbet::realmain::lsp

#endif
