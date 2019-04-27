#ifndef RUBY_TYPER_LSP_LSPMESSAGE_H
#define RUBY_TYPER_LSP_LSPMESSAGE_H

#include "common/Counters.h"
#include "main/lsp/json_types.h"
#include "rapidjson/document.h"
#include <chrono>
#include <variant>

namespace sorbet::realmain::lsp {

/**
 * Represents the ID on an LSP message.
 */
class MessageId {
private:
    const std::variant<int, std::string, JSONNullObject> id;

public:
    MessageId(int id);
    MessageId(const std::variant<int, std::string> id);
    MessageId(const std::variant<int, std::string, JSONNullObject> id);

    operator std::variant<int, std::string, JSONNullObject>() const;

    bool isInt() const;
    int asInt() const;
    bool isString() const;
    std::string asString() const;
};

/**
 * Represents a LSP message, which can be either a request, a notification, or a response.
 */
class LSPMessage final {
public:
    typedef std::variant<std::unique_ptr<RequestMessage>, std::unique_ptr<NotificationMessage>,
                         std::unique_ptr<ResponseMessage>>
        RawLSPMessage;

private:
    RawLSPMessage msg;

public:
    /**
     * Parses a message received from a client. Unlike the constructor, this method does not throw an exception if the
     * JSON fails to parse. Instead, it returns a SorbetError LSPMessage. The Sorbet language server properly processes
     * these SorbetErrors to return the error to the client (or to print it in the log), so it can be passed along as if
     * parsing succeeded.
     */
    static std::unique_ptr<LSPMessage> fromClient(const std::string &json);

    LSPMessage(RawLSPMessage msg);
    LSPMessage(rapidjson::Document &d);
    LSPMessage(const std::string &json);

    /** A tracer for following LSP message in time traces */
    FlowId startTracer;

    /** Request counter. */
    int counter;

    /** If `true`, then this LSPMessage contains a canceled LSP request. */
    bool canceled = false;

    /**
     * Returns an ID if the message has one. Otherwise, returns nullopt.
     */
    std::optional<MessageId> id() const;

    /**
     * Returns true if this is a request message.
     */
    bool isRequest() const;

    /**
     * Returns true if this is a notification message.
     */
    bool isNotification() const;

    /**
     * Returns true if this is a response message.
     */
    bool isResponse() const;

    /**
     * Returns a reference to the RequestMessage that this object wraps.
     * Throws if this does not wrap a request.
     */
    const RequestMessage &asRequest() const;

    /**
     * Returns a reference to the RequestMessage that this object wraps.
     * Throws if this does not wrap a request.
     */
    RequestMessage &asRequest();

    /**
     * Returns a reference to the NotificationMessage that this object wraps.
     * Throws if this does not wrap a notification.
     */
    const NotificationMessage &asNotification() const;

    /**
     * Returns a reference to the NotificationMessage that this object wraps.
     * Throws if this does not wrap a notification.
     */
    NotificationMessage &asNotification();

    /**
     * Returns a reference to the ResponseMessage that this object wraps.
     * Throws if this does not wrap a response.
     */
    const ResponseMessage &asResponse() const;

    /**
     * Returns a reference to the ResponseMessage that this object wraps.
     * Throws if this does not wrap a response.
     */
    ResponseMessage &asResponse();

    /**
     * If this is a request or a notification, returns the contents of the `method` field.
     * Raises if it is a response.
     */
    LSPMethod method() const;

    /**
     * Returns the message in JSON form.
     */
    std::string toJSON() const;
};
} // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LSPMESSAGE_H
