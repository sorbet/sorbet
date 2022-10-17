#include "main/lsp/notifications/watchman_state_leave.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
WatchmanStateLeaveTask::WatchmanStateLeaveTask(const LSPConfiguration &config,
                                               std::unique_ptr<WatchmanStateLeave> params)
    : LSPTask(config, LSPMethod::SorbetWatchmanStateLeave), params(move(params)) {}

LSPTask::Phase WatchmanStateLeaveTask::finalPhase() const {
    return LSPTask::Phase::PREPROCESS;
}

void WatchmanStateLeaveTask::preprocess(LSPPreprocessor &preprocessor) {}

void WatchmanStateLeaveTask::run(LSPTypecheckerDelegate &tc) {}

} // namespace sorbet::realmain::lsp
