#include "main/lsp/notifications/sorbet_resume.h"
#include "main/lsp/LSPPreprocessor.h"

namespace sorbet::realmain::lsp {
SorbetResumeTask::SorbetResumeTask(const LSPConfiguration &config) : LSPTask(config, LSPMethod::RESUME) {}

LSPTask::Phase SorbetResumeTask::finalPhase() const {
    return LSPTask::Phase::PREPROCESS;
}

void SorbetResumeTask::preprocess(LSPPreprocessor &preprocessor) {
    preprocessor.resume();
}

void SorbetResumeTask::run(LSPTypecheckerDelegate &tc) {}
} // namespace sorbet::realmain::lsp
