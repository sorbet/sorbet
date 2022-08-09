#ifndef SORBET_ERROR_FLUSHER_H
#define SORBET_ERROR_FLUSHER_H

#include "core/ErrorQueueMessage.h"
#include <vector>

namespace sorbet {
class FileSystem;
namespace core {

class ErrorFlusher {
public:
    virtual bool wouldFlushErrors(core::FileRef file) const = 0;

    virtual void flushErrors(spdlog::logger &logger, const GlobalState &gs, core::FileRef file,
                             std::vector<std::unique_ptr<ErrorQueueMessage>> errors) = 0;
    virtual ~ErrorFlusher() = default;
};

} // namespace core
} // namespace sorbet

#endif
