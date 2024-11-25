#include "main/lsp/ErrorFlusherLSP.h"
#include <memory>

using namespace std;
namespace sorbet::realmain::lsp {

core::ErrorQueueMessage makeErrorQueueMessage(const core::GlobalState &gs, unique_ptr<core::Error> error) {
    core::ErrorQueueMessage msg;
    msg.kind = core::ErrorQueueMessage::Kind::Error;
    msg.whatFile = error->loc.file();
    if (!error->isSilenced) {
        // Serializing errors is expensive, so we only serialize them if the error isn't silenced.
        msg.text = error->toString(gs);
    }
    msg.error = move(error);
    return msg;
}
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

void ErrorFlusherLSP::flushAllErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                                     std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors) {
    for (auto &[file, cachedErrors] : errorCache) {
        for (auto &error : cachedErrors) {
            errors.push_back(make_unique<core::ErrorQueueMessage>(makeErrorQueueMessage(gs, move(error))));
        }
    }
    errorCache.clear();
    flushErrors(logger, gs, file, move(errors));
};
void ErrorFlusherLSP::flushAndRetainErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                                           std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors) {
    auto prevErrorsIt = errorCache.find(file);
    if (prevErrorsIt != errorCache.end()) {
        auto &prevErrors = (*prevErrorsIt).second;
        for (auto &e : prevErrors) {
            errors.push_back(make_unique<core::ErrorQueueMessage>((makeErrorQueueMessage(gs, move(e)))));
        }
    }

    flushErrors(logger, gs, file, move(errors));
}

void ErrorFlusherLSP::clearCacheForFile(const core::GlobalState &gs, core::FileRef fref, int lowerBound, int upperBound) {
    auto &prevErrors = errorCache[fref];
    auto removeIt = std::remove_if(prevErrors.begin(), prevErrors.end(), [lowerBound, upperBound](const unique_ptr<core::Error>& error) {
        if (!error->loc.file().exists()) {
            return true;
        }

        return lowerBound < error->what.code && error->what.code < upperBound;
    });
    prevErrors.erase(removeIt, prevErrors.end());
};
} // namespace sorbet::realmain::lsp
