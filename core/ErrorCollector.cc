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

} // namespace sorbet::core
