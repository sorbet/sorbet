#ifndef SORBET_ERROR_QUEUE_MESSAGE_H
#define SORBET_ERROR_QUEUE_MESSAGE_H

namespace sorbet {
namespace core {

class BasicError;
struct QueryResponse;

struct ErrorQueueMessage {
    enum class Kind { Error, Flush, QueryResponse };
    Kind kind;
    core::FileRef whatFile;
    std::string text;
    std::unique_ptr<BasicError> error;
    std::unique_ptr<QueryResponse> queryResponse;
};

} // namespace core
} // namespace sorbet

#endif
