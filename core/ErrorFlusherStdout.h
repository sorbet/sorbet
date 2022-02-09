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
    int errorsPrinted{0};
    bool printedErrorLimitMessage{false};

    // Maximum number of errors to print before stopping. 0 means no limit.
    int maxErrors;
    bool printedAtLeastOneError{false};

public:
    ErrorFlusherStdout(int maxErrors) : maxErrors{maxErrors} {}
    ~ErrorFlusherStdout() = default;

    void flushErrors(spdlog::logger &logger, const GlobalState &gs, core::FileRef file,
                     std::vector<std::unique_ptr<ErrorQueueMessage>> errors) override;
    void flushErrorCount(spdlog::logger &logger, int count);
    void flushAutocorrects(const GlobalState &gs, FileSystem &fs);
};

} // namespace sorbet::core

#endif
