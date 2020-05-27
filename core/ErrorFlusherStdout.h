#ifndef SORBET_ERROR_FLUSHER_STDOUT_H
#define SORBET_ERROR_FLUSHER_STDOUT_H

#include "core/AutocorrectSuggestion.h"
#include "core/ErrorFlusherBase.h"
#include "core/ErrorQueueMessage.h"
#include <vector>

namespace sorbet::core {

class ErrorFlusherStdout : public ErrorFlusherBase {
private:
    std::vector<AutocorrectSuggestion> autocorrects;
    bool printedAtLeastOneError{false};

public:
    ErrorFlusherStdout() = default;

    using ErrorFlusherBase::flushErrors;
    virtual void flushErrors(spdlog::logger &logger, std::vector<std::unique_ptr<ErrorQueueMessage>> error);

    using ErrorFlusherBase::flushErrorCount;
    virtual void flushErrorCount(spdlog::logger &logger, int count);

    using ErrorFlusherBase::flushAutocorrects;
    virtual void flushAutocorrects(const GlobalState &gs, FileSystem &fs);
};

} // namespace sorbet::core

#endif
