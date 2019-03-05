#ifndef SORBET_ERROR_FLUSHER_H
#define SORBET_ERROR_FLUSHER_H

#include "core/AutocorrectSuggestion.h"
#include "core/ErrorQueueMessage.h"
#include <vector>

namespace sorbet {
class FileSystem;
namespace core {

class ErrorFlusher {
private:
    std::vector<AutocorrectSuggestion> autocorrects;
    bool printedAtLeastOneError{false};

public:
    ErrorFlusher() = default;
    void flushErrors(spdlog::logger &logger, std::vector<std::unique_ptr<ErrorQueueMessage>> error);
    void flushErrorCount(spdlog::logger &logger, int count);
    void flushAutocorrects(const GlobalState &gs, FileSystem &fs);
};

} // namespace core
} // namespace sorbet

#endif
