#ifndef SORBET_ERROR_QUEUE_MESSAGE_H
#define SORBET_ERROR_QUEUE_MESSAGE_H

namespace sorbet::core {

class Error;
struct QueryResponse;

struct ErrorQueueMessage {
    enum class Kind { Error, Flush, QueryResponse };
    Kind kind;
    core::FileRef whatFile;
    std::string text;
    std::unique_ptr<Error> error;
    std::unique_ptr<QueryResponse> queryResponse;
};

} // namespace sorbet::core

#endif
