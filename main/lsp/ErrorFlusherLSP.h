#ifndef SORBET_ERROR_FLUSHER_LSP_H
#define SORBET_ERROR_FLUSHER_LSP_H

#include "core/AutocorrectSuggestion.h"
#include "core/ErrorFlusher.h"
#include "core/ErrorQueueMessage.h"
#include "core/core.h"
#include "main/lsp/ErrorReporter.h"

namespace sorbet::realmain::lsp {

class ErrorFlusherLSP : public core::ErrorFlusher {
private:
    std::vector<core::AutocorrectSuggestion> autocorrects;
    u4 epoch;
    std::shared_ptr<ErrorReporter> errorReporter;

public:
    ErrorFlusherLSP(const u4 epoch, std::shared_ptr<ErrorReporter> errorReporter);
    ~ErrorFlusherLSP() = default;

    void flushErrors(spdlog::logger &logger, std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors,
                     const core::GlobalState &gs, core::FileRef file) override;
    void flushErrorCount(spdlog::logger &logger, int count) override;
    void flushAutocorrects(const core::GlobalState &gs, FileSystem &fs) override;
};

} // namespace sorbet::realmain::lsp

#endif
