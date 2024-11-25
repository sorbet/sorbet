#include "core/NullFlusher.h"

using namespace std;
namespace sorbet::core {
void NullFlusher::flushErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                              std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors) {}

void NullFlusher::flushAndRetainErrors(spdlog::logger &logger, const GlobalState &gs, core::FileRef file,
                                       std::vector<std::unique_ptr<ErrorQueueMessage>> errors) {}

void NullFlusher::flushAllErrors(spdlog::logger &logger, const GlobalState &gs, core::FileRef file,
                                 std::vector<std::unique_ptr<ErrorQueueMessage>> errors){};

void NullFlusher::clearCacheForFile(const core::GlobalState &gs, core::FileRef fref, int lowerBound, int upperBound){};
} // namespace sorbet::core
