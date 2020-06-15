#include "main/lsp/ErrorFlusherLSP.h"

using namespace std;
namespace sorbet::realmain::lsp {
ErrorFlusherLSP::ErrorFlusherLSP(const u4 epoch, shared_ptr<ErrorReporter> errorReporter)
    : epoch(epoch), errorReporter(errorReporter){};

void ErrorFlusherLSP::flushErrors(spdlog::logger &logger, vector<unique_ptr<core::ErrorQueueMessage>> errors,
                                  const core::GlobalState &gs, core::FileRef file) {
    vector<std::unique_ptr<core::Error>> errorsAccumulated;

    for (auto &error : errors) {
        if (error->error->isSilenced) {
            continue;
        }

        prodHistogramAdd("error", error->error->what.code, 1);
        errorsAccumulated.emplace_back(move(error->error));
    }

    errorReporter->pushDiagnostics(epoch, file, errorsAccumulated, gs);
}
} // namespace sorbet::realmain::lsp
