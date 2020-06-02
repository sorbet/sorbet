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

    void flushErrors(spdlog::logger &logger, std::vector<std::unique_ptr<ErrorQueueMessage>> errors) override;
    void flushErrorCount(spdlog::logger &logger, int count) override;
    void flushAutocorrects(const GlobalState &gs, FileSystem &fs) override;
};

} // namespace sorbet::core

#endif
