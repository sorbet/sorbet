#include "main/lsp/ErrorFlusherLSP.h"

using namespace std;
namespace sorbet::realmain::lsp {
ErrorFlusherLSP::ErrorFlusherLSP(const uint32_t epoch, shared_ptr<ErrorReporter> errorReporter)
    : epoch(epoch), errorReporter(errorReporter){};

bool ErrorFlusherLSP::wouldFlushErrors(core::FileRef file) const {
    return errorReporter->wouldReportForFile(epoch, file);
}

void ErrorFlusherLSP::flushErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                                  vector<unique_ptr<core::ErrorQueueMessage>> errors) {
    vector<std::unique_ptr<core::Error>> errorsAccumulated;

    for (auto &error : errors) {
        if (error->kind == core::ErrorQueueMessage::Kind::Error) {
            if (error->error->isSilenced) {
                continue;
            }

            prodHistogramAdd("error", error->error->what.code, 1);
            errorsAccumulated.emplace_back(move(error->error));
        }
    }

    errorReporter->pushDiagnostics(epoch, file, errorsAccumulated, gs);
}
} // namespace sorbet::realmain::lsp
