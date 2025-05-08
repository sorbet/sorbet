#include "core/NullFlusher.h"

using namespace std;
namespace sorbet::core {
void NullFlusher::flushErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                              vector<unique_ptr<core::ErrorQueueMessage>> errors) {}
} // namespace sorbet::core
