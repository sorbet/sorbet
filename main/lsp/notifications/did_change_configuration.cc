#include "main/lsp/notifications/did_change_configuration.h"
#include "main/lsp/LSPIndexer.h"
#include "main/lsp/LSPPreprocessor.h"

namespace sorbet::realmain::lsp {
DidChangeConfigurationTask::DidChangeConfigurationTask(const LSPConfiguration &config,
                                                       std::unique_ptr<DidChangeConfigurationParams> params)
    : LSPTask(config, LSPMethod::WorkspaceDidChangeConfiguration), params(move(params)), openFilePaths() {}

LSPTask::Phase DidChangeConfigurationTask::finalPhase() const {
    // We want this to run all the way so that the changes to
    // Global State get propagated through.
    return LSPTask::Phase::RUN;
}

void DidChangeConfigurationTask::preprocess(LSPPreprocessor &preprocessor) {
    auto tmp = preprocessor.openFilePaths();
    openFilePaths = std::move(tmp);
}

void DidChangeConfigurationTask::index(LSPIndexer &indexer) {
    indexer.updateGsFromOptions(*params);
}

void DidChangeConfigurationTask::run(LSPTypecheckerDelegate &tc) {
    tc.updateGsFromOptions(*params);
    tc.retypecheckFromPaths(std::move(openFilePaths));
}
} // namespace sorbet::realmain::lsp
