#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_INITIALIZED_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_INITIALIZED_H

#include "absl/synchronization/notification.h"
#include "main/cache/cache.h"
#include "main/lsp/LSPFileUpdates.h"
#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class InitializedTask final : public LSPTask {
    LSPConfiguration &mutableConfig;
    LSPFileUpdates updates;
    LSPPreprocessor *preprocessor;
    std::unique_ptr<core::GlobalState> gs;
    std::unique_ptr<cache::SessionCache> sessionCache;

public:
    InitializedTask(LSPConfiguration &config);

    void preprocess(LSPPreprocessor &preprocessor) override;
    void index(LSPIndexer &indexer) override;
    void run(LSPTypecheckerDelegate &typechecker) override;

    void setGlobalState(std::unique_ptr<core::GlobalState> gs);
    void setSessionCache(std::unique_ptr<cache::SessionCache> sessionCache);
    bool canPreempt(const LSPIndexer &indexer) const override;
};
} // namespace sorbet::realmain::lsp

#endif
