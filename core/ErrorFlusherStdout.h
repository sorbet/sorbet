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
    ~ErrorFlusherStdout(){};
    using ErrorFlusher::flushErrors;
    virtual void flushErrors(spdlog::logger &logger, std::vector<std::unique_ptr<ErrorQueueMessage>> error);

    using ErrorFlusher::flushErrorCount;
    virtual void flushErrorCount(spdlog::logger &logger, int count);

    using ErrorFlusher::flushAutocorrects;
    virtual void flushAutocorrects(const GlobalState &gs, FileSystem &fs);
};

} // namespace sorbet::core

#endif
