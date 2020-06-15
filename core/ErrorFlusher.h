#ifndef SORBET_ERROR_FLUSHER_H
#define SORBET_ERROR_FLUSHER_H

#include "core/ErrorQueueMessage.h"
#include <vector>

namespace sorbet {
class FileSystem;
namespace core {

class ErrorFlusher {
public:
    virtual void flushErrors(spdlog::logger &logger, std::vector<std::unique_ptr<ErrorQueueMessage>> errors,
                             const GlobalState &gs, core::FileRef file) = 0;
    virtual ~ErrorFlusher() = default;
};

} // namespace core
} // namespace sorbet

#endif
