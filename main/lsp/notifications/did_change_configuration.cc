#include "main/lsp/notifications/did_change_configuration.h"
#include "main/lsp/LSPIndexer.h"
#include "main/lsp/LSPPreprocessor.h"

namespace sorbet::realmain::lsp {
DidChangeConfigurationTask::DidChangeConfigurationTask(const LSPConfiguration &config,
                                                       std::unique_ptr<DidChangeConfigurationParams> params)
    : LSPTask(config, LSPMethod::WorkspaceDidChangeConfiguration), params(move(params)) {}

LSPTask::Phase DidChangeConfigurationTask::finalPhase() const {
    return LSPTask::Phase::RUN;
}

void DidChangeConfigurationTask::preprocess(LSPPreprocessor &preprocessor) {
    // preprocessor.resume();
}

void DidChangeConfigurationTask::index(LSPIndexer &indexer) {
    indexer.updateGsFromOptions(*params);
}

void DidChangeConfigurationTask::run(LSPTypecheckerDelegate &tc) {
    tc.updateGsFromOptions(*params);
}
} // namespace sorbet::realmain::lsp
