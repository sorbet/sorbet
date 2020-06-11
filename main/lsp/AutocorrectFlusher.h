#ifndef SORBET_AUTOCORRECT_FLUSHER_H
#define SORBET_AUTOCORRECT_FLUSHER_H

#include "core/ErrorFlusher.h"

namespace sorbet::realmain::lsp {

class AutocorrectFlusher : public core::ErrorFlusher {
public:
    std::vector<std::unique_ptr<core::Error>> collectedErrors;
    AutocorrectFlusher() = default;
    ~AutocorrectFlusher() = default;

    void flushErrors(spdlog::logger &logger, std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors,
                     const core::GlobalState &gs, core::FileRef file) override;
    void flushErrorCount(spdlog::logger &logger, int count) override;
    void flushAutocorrects(const core::GlobalState &gs, FileSystem &fs) override;
};

} // namespace sorbet::realmain::lsp

#endif
