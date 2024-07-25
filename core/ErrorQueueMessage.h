#ifndef SORBET_ERROR_QUEUE_MESSAGE_H
#define SORBET_ERROR_QUEUE_MESSAGE_H

#include "core/lsp/QueryResponse.h"

namespace sorbet::core {

class Error;

struct ErrorQueueMessage {
    enum class Kind { Error, QueryResponse };
    Kind kind;
    core::FileRef whatFile;
    // The text of the error. Is a `nullopt` if the error is silenced.
    std::optional<std::string> text;
    std::unique_ptr<Error> error;
    std::unique_ptr<lsp::QueryResponse> queryResponse;

    ErrorQueueMessage clone();
};

} // namespace sorbet::core

#endif
