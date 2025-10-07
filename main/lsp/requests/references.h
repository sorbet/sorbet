#ifndef RUBY_TYPER_LSP_REQUESTS_REFERENCES_H
#define RUBY_TYPER_LSP_REQUESTS_REFERENCES_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class ReferenceParams;
class ReferencesTask final : public LSPRequestTask {
    std::unique_ptr<ReferenceParams> params;

public:
    ReferencesTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<ReferenceParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;

    bool needsMultithreading(const LSPIndexer &indexer) const override;
};

} // namespace sorbet::realmain::lsp

#endif
