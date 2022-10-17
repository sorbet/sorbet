#include "main/lsp/notifications/watchman_state_enter.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
WatchmanStateEnterTask::WatchmanStateEnterTask(const LSPConfiguration &config,
                                               std::unique_ptr<WatchmanStateEnter> params)
    : LSPTask(config, LSPMethod::SorbetWatchmanStateEnter), params(move(params)) {}

LSPTask::Phase WatchmanStateEnterTask::finalPhase() const {
    return LSPTask::Phase::PREPROCESS;
}

void WatchmanStateEnterTask::preprocess(LSPPreprocessor &preprocessor) {}

void WatchmanStateEnterTask::run(LSPTypecheckerDelegate &tc) {}

} // namespace sorbet::realmain::lsp
