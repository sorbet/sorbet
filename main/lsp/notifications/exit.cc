#include "main/lsp/notifications/exit.h"
#include "main/lsp/LSPIndexer.h"
#include "main/lsp/LSPPreprocessor.h"

using namespace std;

namespace sorbet::realmain::lsp {
ExitTask::ExitTask(const LSPConfiguration &config, int exitCode)
    : LSPTask(config, LSPMethod::Exit), exitCode(exitCode) {}

LSPTask::Phase ExitTask::finalPhase() const {
    return LSPTask::Phase::PREPROCESS;
}

void ExitTask::preprocess(LSPPreprocessor &preprocessor) {
    preprocessor.exit(exitCode);
}

void ExitTask::run(LSPTypecheckerDelegate &tc) {}

} // namespace sorbet::realmain::lsp