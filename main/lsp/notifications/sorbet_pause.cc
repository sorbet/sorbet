#include "main/lsp/notifications/sorbet_pause.h"
#include "main/lsp/LSPPreprocessor.h"

namespace sorbet::realmain::lsp {
SorbetPauseTask::SorbetPauseTask(const LSPConfiguration &config) : LSPTask(config, LSPMethod::PAUSE) {}

LSPTask::Phase SorbetPauseTask::finalPhase() const {
    return LSPTask::Phase::PREPROCESS;
}

void SorbetPauseTask::preprocess(LSPPreprocessor &preprocessor) {
    preprocessor.pause();
}

void SorbetPauseTask::run(LSPTypecheckerDelegate &tc) {}
} // namespace sorbet::realmain::lsp
