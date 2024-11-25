#include "core/ErrorCollector.h"

using namespace std;
namespace sorbet::core {

void ErrorCollector::flushErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                                 std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors) {
    for (auto &error : errors) {
        if (error->kind == core::ErrorQueueMessage::Kind::Error) {
            if (error->error->isSilenced) {
                continue;
            }

            collectedErrors.emplace_back(move(error->error));
        }
    }
}

[[nodiscard]] vector<unique_ptr<core::Error>> ErrorCollector::drainErrors() {
    return move(collectedErrors);
}

void ErrorCollector::flushAndRetainErrors(spdlog::logger &logger, const GlobalState &gs, core::FileRef file,
                                          std::vector<std::unique_ptr<ErrorQueueMessage>> errors) {}

void ErrorCollector::flushAllErrors(spdlog::logger &logger, const GlobalState &gs, core::FileRef file,
                                    std::vector<std::unique_ptr<ErrorQueueMessage>> errors) {
    flushErrors(logger, gs, file, move(errors));
};

    void ErrorCollector::clearCacheForFile(const GlobalState &gs, core::FileRef fref, int lowerBound, int upperBound) {};
} // namespace sorbet::core
