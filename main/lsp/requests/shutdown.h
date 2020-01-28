#ifndef RUBY_TYPER_LSP_REQUESTS_SHUTDOWN_H
#define RUBY_TYPER_LSP_REQUESTS_SHUTDOWN_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class ShutdownTask final : public LSPRequestTask {
public:
    ShutdownTask(const LSPConfiguration &config, MessageId id);

    bool canPreempt(const LSPIndexer &indexer) const override;

protected:
    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &ts) override;
};
} // namespace sorbet::realmain::lsp

#endif
