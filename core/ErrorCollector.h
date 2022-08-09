#ifndef SORBET_ERROR_COLLECTOR_H
#define SORBET_ERROR_COLLECTOR_H

#include "core/ErrorFlusher.h"

namespace sorbet::core {

class ErrorCollector : public core::ErrorFlusher {
private:
    std::vector<std::unique_ptr<core::Error>> collectedErrors;

public:
    ErrorCollector() = default;
    ~ErrorCollector() = default;
    bool wouldFlushErrors(core::FileRef file) const override {
        return true;
    }
    void flushErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                     std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors) override;
    std::vector<std::unique_ptr<core::Error>> drainErrors();
};

} // namespace sorbet::core

#endif
