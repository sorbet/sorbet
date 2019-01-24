#ifndef SORBET_ERROR_QUEUE_MESSAGE_H
#define SORBET_ERROR_QUEUE_MESSAGE_H

#include "core/lsp/QueryResponse.h"

namespace sorbet::core {

class Error;

struct ErrorQueueMessage {
    enum class Kind { Error, Flush, QueryResponse };
    Kind kind;
    core::FileRef whatFile;
    std::string text;
    std::unique_ptr<Error> error;
    std::unique_ptr<lsp::QueryResponse> queryResponse;
};

} // namespace sorbet::core

#endif
