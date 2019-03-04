#ifndef RUBY_TYPER_LSP_LSPMESSAGE_H
#define RUBY_TYPER_LSP_LSPMESSAGE_H

#include "main/lsp/json_types.h"
#include "rapidjson/document.h"
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
    LSPMessage(RawLSPMessage msg);
    LSPMessage(rapidjson::MemoryPoolAllocator<> &alloc, rapidjson::Document &d);
    LSPMessage(rapidjson::MemoryPoolAllocator<> &alloc, const std::string &json);

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
    std::string_view method() const;

    /**
     * If this is a request or a notification, sets the contents of the `params` field.
     * Raises if it is a response.
     */
    void setParams(std::unique_ptr<rapidjson::Value> params);

    /**
     * Returns the value in the `param` field. If field is missing or, in the case of a notification, is an array,
     * returns a reference to a null rapidjson object.
     */
    const rapidjson::Value &params() const;

    /**
     * Set the internal `sorbet_counter` property on this message.
     */
    void setCounter(int count);

    /**
     * Retrieve the internal `sorbet_counter` property on this message. Returns -1 if not specified.
     */
    int counter() const;

    /**
     * Set the internal `sorbet_receive_timestamp` property on this message.
     */
    void setTimestamp(double timestamp);

    /**
     * Retrieve the internal `sorbet_receive_timestamp` property on this message. Returns 0.0 if not specified.
     */
    double timestamp() const;

    /**
     * Returns the message in JSON form.
     */
    std::string toJSON() const;
};
} // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LSPMESSAGE_H