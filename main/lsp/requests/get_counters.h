#ifndef RUBY_TYPER_LSP_REQUESTS_GET_COUNTERS_H
#define RUBY_TYPER_LSP_REQUESTS_GET_COUNTERS_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class GetCountersTask final : public LSPRequestTask {
public:
    GetCountersTask(const LSPConfiguration &config, MessageId id);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};
} // namespace sorbet::realmain::lsp

#endif
