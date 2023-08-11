#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_DID_CHANGE_CONFIGURATION_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_DID_CHANGE_CONFIGURATION_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class DidChangeConfigurationParams;
class DidChangeConfigurationTask final : public LSPTask {
    std::unique_ptr<DidChangeConfigurationParams> params;
    const std::vector<std::string_view> openFilePaths;
    const uint32_t epoch;

public:
    DidChangeConfigurationTask(const LSPConfiguration &config, std::unique_ptr<DidChangeConfigurationParams> params,
                               std::vector<std::string_view> &&openFiles, const uint32_t epoch);

    LSPTask::Phase finalPhase() const override;

    void index(LSPIndexer &indexer) override;

    void run(LSPTypecheckerDelegate &tc) override;
};
} // namespace sorbet::realmain::lsp

#endif
