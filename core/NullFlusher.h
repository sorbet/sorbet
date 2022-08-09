#ifndef SORBET_NULL_FLUSHER_H
#define SORBET_NULL_FLUSHER_H

#include "core/ErrorFlusher.h"

namespace sorbet::core {

class NullFlusher : public core::ErrorFlusher {
public:
    NullFlusher() = default;
    ~NullFlusher() = default;

    bool wouldFlushErrors(core::FileRef file) const override {
        return true;
    }
    void flushErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                     std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors) override;
};

} // namespace sorbet::core

#endif
