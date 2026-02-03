#ifndef RUBY_TYPER_LSP_REQUESTS_INLAY_HINT_H
#define RUBY_TYPER_LSP_REQUESTS_INLAY_HINT_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class InlayHintParams;

class InlayHintTask final : public LSPRequestTask {
    std::unique_ptr<InlayHintParams> params;

public:
    InlayHintTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<InlayHintParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
