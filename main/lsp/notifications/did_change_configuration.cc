#include "main/lsp/notifications/did_change_configuration.h"
#include "main/lsp/LSPIndexer.h"
#include "main/lsp/LSPPreprocessor.h"

namespace sorbet::realmain::lsp {
DidChangeConfigurationTask::DidChangeConfigurationTask(const LSPConfiguration &config,
                                                       std::unique_ptr<DidChangeConfigurationParams> params,
                                                       std::vector<std::string_view> &&openFiles, const uint32_t epoch)
    : LSPTask(config, LSPMethod::WorkspaceDidChangeConfiguration), params(move(params)), openFilePaths(move(openFiles)),
      epoch(epoch) {}

LSPTask::Phase DidChangeConfigurationTask::finalPhase() const {
    // We want this to run all the way so that the changes to
    // Global State get propagated through.
    return LSPTask::Phase::RUN;
}

void DidChangeConfigurationTask::index(LSPIndexer &indexer) {
    indexer.updateGsFromOptions(*params);
}

void DidChangeConfigurationTask::run(LSPTypecheckerDelegate &tc) {
    tc.updateGsFromOptions(*params);
    std::vector<core::FileRef> openFileRefs;
    for (auto const &path : openFilePaths) {
        openFileRefs.push_back(tc.state().findFileByPath(path));
    }
    auto updates = tc.getNoopUpdate(openFileRefs);
    updates.epoch = epoch;
    tc.typecheckOnFastPath(std::move(updates), {});
}
} // namespace sorbet::realmain::lsp
