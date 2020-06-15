#include "main/lsp/AutocorrectFlusher.h"

using namespace std;
namespace sorbet::realmain::lsp {

void AutocorrectFlusher::flushErrors(spdlog::logger &logger, vector<unique_ptr<core::ErrorQueueMessage>> errors,
                                     const core::GlobalState &gs, core::FileRef file) {
    for (auto &error : errors) {
        if (error->kind == core::ErrorQueueMessage::Kind::Error) {
            if (error->error->isSilenced) {
                continue;
            }

            prodHistogramAdd("error", error->error->what.code, 1);
            collectedErrors.emplace_back(move(error->error));
        }
    }
}
} // namespace sorbet::realmain::lsp
