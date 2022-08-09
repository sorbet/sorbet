#ifndef SORBET_QUERY_COLLECTOR_H
#define SORBET_QUERY_COLLECTOR_H

#include "core/ErrorFlusher.h"

namespace sorbet::realmain::lsp {

class QueryCollector : public core::ErrorFlusher {
private:
    std::vector<std::unique_ptr<core::lsp::QueryResponse>> queryResponses;

public:
    QueryCollector() = default;
    ~QueryCollector() = default;

    // Can never skip, because we need to run for the sake of reporting query responses
    bool wouldFlushErrors(core::FileRef file) const override {
        return true;
    }
    void flushErrors(spdlog::logger &logger, const core::GlobalState &gs, core::FileRef file,
                     std::vector<std::unique_ptr<core::ErrorQueueMessage>> errors) override;

    std::vector<std::unique_ptr<core::lsp::QueryResponse>> drainQueryResponses();
};

} // namespace sorbet::realmain::lsp

#endif
