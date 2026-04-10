#include "main/lsp/notifications/did_change_configuration.h"
#include "main/lsp/LSPIndexer.h"
#include "main/lsp/LSPPreprocessor.h"

using namespace std;

namespace sorbet::realmain::lsp {
DidChangeConfigurationTask::DidChangeConfigurationTask(const LSPConfiguration &config,
                                                       unique_ptr<DidChangeConfigurationParams> params,
                                                       vector<string_view> &&openFiles, const uint32_t epoch)
    : LSPTask(config, LSPMethod::WorkspaceDidChangeConfiguration), params(move(params)), openFilePaths(move(openFiles)),
      epoch(epoch) {}

LSPTask::Phase DidChangeConfigurationTask::finalPhase() const {
    // We want this to run all the way so that the changes to
    // Global State get propagated through.
    return LSPTask::Phase::RUN;
}

void DidChangeConfigurationTask::index(LSPIndexer &indexer) {
    indexer.updateConfigAndGsFromOptions(*params);
}

namespace {

vector<core::FileRef> getOpenFileRefs(const LSPTypecheckerDelegate &tc, absl::Span<const string_view> openFilePaths) {
    vector<core::FileRef> openFileRefs;
    openFileRefs.reserve(openFilePaths.size());

    for (auto path : openFilePaths) {
        openFileRefs.push_back(tc.state().findFileByPath(path));
    }

    return openFileRefs;
}

} // namespace

void DidChangeConfigurationTask::run(LSPTypecheckerDelegate &tc) {
    tc.updateConfigAndGsFromOptions(*params);
    auto openFileRefs = getOpenFileRefs(tc, this->openFilePaths);
    auto updates = tc.getNoopUpdate(openFileRefs);
    updates->epoch = epoch;
    tc.typecheckOnFastPath(std::move(updates), {});
}
} // namespace sorbet::realmain::lsp
