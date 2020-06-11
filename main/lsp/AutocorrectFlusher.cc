#include "main/lsp/AutocorrectFlusher.h"

using namespace std;
namespace sorbet::realmain::lsp {

void AutocorrectFlusher::flushErrors(spdlog::logger &logger, vector<unique_ptr<core::ErrorQueueMessage>> errors,
                                     const core::GlobalState &gs, core::FileRef file) {
    for (auto &error : errors) {
        if (error->error->isSilenced) {
            continue;
        }

        prodHistogramAdd("error", error->error->what.code, 1);
        collectedErrors.emplace_back(move(error->error));
    }
}

void AutocorrectFlusher::flushErrorCount(spdlog::logger &logger, int count) {}

void AutocorrectFlusher::flushAutocorrects(const core::GlobalState &gs, FileSystem &fs) {}

} // namespace sorbet::realmain::lsp
