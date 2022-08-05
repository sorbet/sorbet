#ifndef SORBET_ERROR_FLUSHER_STDOUT_H
#define SORBET_ERROR_FLUSHER_STDOUT_H

#include "core/AutocorrectSuggestion.h"
#include "core/ErrorFlusher.h"
#include "core/ErrorQueueMessage.h"
#include <vector>

namespace sorbet::core {

class ErrorFlusherStdout : public ErrorFlusher {
private:
    std::vector<AutocorrectSuggestion> autocorrects;
    bool printedAtLeastOneError{false};

public:
    ErrorFlusherStdout() = default;
    ~ErrorFlusherStdout() = default;

    // wouldFlushErrors is only ever false in LSP mode, where a later edit can produce newer
    // errors, making it not useful to report errors for the given file.
    bool wouldFlushErrors(core::FileRef file) const override {
        return true;
    }
    void flushErrors(spdlog::logger &logger, const GlobalState &gs, core::FileRef file,
                     std::vector<std::unique_ptr<ErrorQueueMessage>> errors) override;
    void flushErrorCount(spdlog::logger &logger, int count);
    void flushAutocorrects(const GlobalState &gs, FileSystem &fs);
};

} // namespace sorbet::core

#endif
