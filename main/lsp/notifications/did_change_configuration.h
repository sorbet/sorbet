#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_DID_CHANGE_CONFIGURATION_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_DID_CHANGE_CONFIGURATION_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class DidChangeConfigurationParams;
class DidChangeConfigurationTask final : public LSPTask {
    std::unique_ptr<DidChangeConfigurationParams> params;
    // These must be owned strings: the preprocessor may close (and thus forget about) one of these files after this
    // task has been created but before it runs on the typechecker thread.
    const std::vector<std::string> openFilePaths;
    const uint32_t epoch;

public:
    DidChangeConfigurationTask(const LSPConfiguration &config, std::unique_ptr<DidChangeConfigurationParams> params,
                               std::vector<std::string> &&openFiles, const uint32_t epoch);

    LSPTask::Phase finalPhase() const override;

    void index(LSPIndexer &indexer) override;

    void run(LSPTypecheckerDelegate &tc) override;

    core::packages::Stratum preemptionStratum(FileStratumMapping info) const override;
};
} // namespace sorbet::realmain::lsp

#endif
