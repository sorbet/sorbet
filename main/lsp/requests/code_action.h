#ifndef RUBY_TYPER_LSP_REQUESTS_CODE_ACTION_H
#define RUBY_TYPER_LSP_REQUESTS_CODE_ACTION_H

#include "main/lsp/LSPTask.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {
class CodeActionParams;
class CodeActionTask final : public LSPRequestTask {
    // WARNING: we currently do not support more than one lazily resolved code action since that would require copying
    // params which we don't support
    // TODO(bshu): investigate using shared pointers instead of unique pointers for lsp types
    std::unique_ptr<CodeActionParams> params;

public:
    CodeActionTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<CodeActionParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;

    core::packages::Stratum preemptionStratum(FileStratumMapping info) const override;
};

} // namespace sorbet::realmain::lsp

#endif
