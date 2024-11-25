#ifndef SORBET_ERROR_FLUSHER_LSP_H
#define SORBET_ERROR_FLUSHER_LSP_H

#include "core/ErrorFlusher.h"
#include "core/Error.h"
#include "main/lsp/ErrorReporter.h"

namespace sorbet::realmain::lsp {

class ErrorFlusherLSP : public core::ErrorFlusher {
private:
    uint32_t epoch;
    std::shared_ptr<ErrorReporter> errorReporter;
    UnorderedMap<core::FileRef, std::vector<std::unique_ptr<core::Error>>> errorCache;

public:
    ErrorFlusherLSP(const uint32_t epoch, std::shared_ptr<ErrorReporter> errorReporter);
    ~ErrorFlusherLSP() = default;

    bool wouldFlushErrors(core::FileRef file) const override;

    void flushErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                     std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors) override;

    void flushAndRetainErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                              std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors) override;

    void flushAllErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                             std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors) override;

    void clearCacheForFile(const core::GlobalState &gs, core::FileRef fref, int upperBound, int lowerBound) override;
};

} // namespace sorbet::realmain::lsp

#endif
