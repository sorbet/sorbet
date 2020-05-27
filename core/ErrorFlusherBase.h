#ifndef SORBET_ERROR_FLUSHER_BASE_H
#define SORBET_ERROR_FLUSHER_BASE_H

#include "core/AutocorrectSuggestion.h"
#include "core/ErrorQueueMessage.h"
#include <vector>

namespace sorbet {
class FileSystem;
namespace core {

class ErrorFlusherBase {
public:
    virtual void flushErrors(spdlog::logger &logger, std::vector<std::unique_ptr<ErrorQueueMessage>> error) = 0;
    virtual void flushErrorCount(spdlog::logger &logger, int count) = 0;
    virtual void flushAutocorrects(const GlobalState &gs, FileSystem &fs) = 0;
};

} // namespace core
} // namespace sorbet

#endif
