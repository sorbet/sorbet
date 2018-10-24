#ifndef SORBET_ERROR_FLUSHER_H
#define SORBET_ERROR_FLUSHER_H

#include "core/AutocorrectSuggestion.h"
#include "core/ErrorQueueMessage.h"
#include <vector>

namespace sorbet::core {

class ErrorFlusher {
private:
    std::vector<AutocorrectSuggestion> autocorrects;
    bool printedAtLeastOneError{false};
    const std::vector<int> errorCodeWhiteList;
    bool isWhitelisted(int code) const;

public:
    ErrorFlusher(std::vector<int> errorCodeWhiteList) : errorCodeWhiteList(move(errorCodeWhiteList)) {}
    void flushErrors(spdlog::logger &logger, std::vector<std::unique_ptr<ErrorQueueMessage>> errors);
    void flushErrorCount(spdlog::logger &logger, int count);
    void flushAutocorrects(const GlobalState &gs);
};

} // namespace sorbet::core

#endif
