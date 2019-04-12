#include "LSPMessage.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
const std::variant<int, string, JSONNullObject> widenMessageVariant(const std::variant<int, string> id) {
    if (auto intId = get_if<int>(&id)) {
        return *intId;
    } else if (auto stringId = get_if<string>(&id)) {
        return *stringId;
    } else {
        Exception::raise("LSP request message ID must be a number or a string.");
    }
}

MessageId::MessageId(int id) : id(id) {}
MessageId::MessageId(const std::variant<int, string> id) : id(widenMessageVariant(id)) {}
MessageId::MessageId(const std::variant<int, string, JSONNullObject> id) : id(id) {}

MessageId::operator variant<int, string, JSONNullObject>() const {
    return id;
}

bool MessageId::isInt() const {
    return !!get_if<int>(&id);
}

int MessageId::asInt() const {
    if (auto intId = get_if<int>(&id)) {
        return *intId;
    }
    Exception::raise("MessageId is not an int.");
}

bool MessageId::isString() const {
    return !!get_if<string>(&id);
}

string MessageId::asString() const {
    if (auto stringId = get_if<string>(&id)) {
        return *stringId;
    }
    Exception::raise("MessageId is not a string.");
}

unique_ptr<LSPMessage> makeSorbetError(rapidjson::MemoryPoolAllocator<> &alloc, LSPErrorCodes code, string_view message,
                                       optional<rapidjson::Document> d = nullopt) {
    auto errorParams = make_unique<SorbetErrorParams>((int)code, string(message));
    auto isRequest = d.has_value() && (*d).HasMember("id") && (*d)["id"].IsInt();
    if (isRequest) {
        auto req = make_unique<RequestMessage>("2.0", (*d)["id"].GetInt(), LSPMethod::SorbetError, move(errorParams));
        return make_unique<LSPMessage>(move(req));
    } else {
        auto notif = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetError, move(errorParams));
        return make_unique<LSPMessage>(move(notif));
    }
}

unique_ptr<LSPMessage> LSPMessage::fromClient(rapidjson::MemoryPoolAllocator<> &alloc, const string &json) {
    rapidjson::Document d(&alloc);
    if (d.Parse(json.c_str()).HasParseError()) {
        return makeSorbetError(alloc, LSPErrorCodes::ParseError,
                               fmt::format("Last LSP request: `{}` is not a valid json object", json));
    }

    try {
        return make_unique<LSPMessage>(alloc, d);
    } catch (InvalidStringEnumError e) {
        if (e.enumName == "LSPMethod") {
            // We don't support whatever method the client is sending.
            return makeSorbetError(alloc, LSPErrorCodes::MethodNotFound,
                                   fmt::format("Unsupported LSP method: {}", e.value), move(d));
        } else {
            return makeSorbetError(alloc, LSPErrorCodes::InvalidParams,
                                   fmt::format("Unable to deserialize LSP request: {}", e.what()), move(d));
        }
    } catch (DeserializationError e) {
        // The client request was valid JSON, but was invalid in some way.
        return makeSorbetError(alloc, LSPErrorCodes::InvalidParams,
                               fmt::format("Unable to deserialize LSP request: {}", e.what()), move(d));
    }
}

LSPMessage::RawLSPMessage fromJSONValue(rapidjson::MemoryPoolAllocator<> &alloc, rapidjson::Document &d) {
    if (d.HasMember("id")) {
        // Method is required on requests, but responses lack it.
        if (d.HasMember("method")) {
            return RequestMessage::fromJSONValue(alloc, d.GetObject(), "root");
        } else {
            return ResponseMessage::fromJSONValue(alloc, d.GetObject(), "root");
        }
    } else {
        return NotificationMessage::fromJSONValue(alloc, d.GetObject(), "root");
    }
}

LSPMessage::RawLSPMessage fromJSON(rapidjson::MemoryPoolAllocator<> &alloc, const std::string &json) {
    rapidjson::Document d(&alloc);
    d.Parse(json.c_str());
    return fromJSONValue(alloc, d);
}

LSPMessage::LSPMessage(RawLSPMessage msg) : msg(move(msg)) {}

LSPMessage::LSPMessage(rapidjson::MemoryPoolAllocator<> &alloc, rapidjson::Document &d)
    : msg(fromJSONValue(alloc, d)) {}

LSPMessage::LSPMessage(rapidjson::MemoryPoolAllocator<> &alloc, const std::string &json) : msg(fromJSON(alloc, json)) {}

optional<MessageId> LSPMessage::id() const {
    if (isRequest()) {
        return asRequest().id;
    } else if (isResponse()) {
        return asResponse().id;
    }
    return nullopt;
}

bool LSPMessage::isRequest() const {
    auto reqMsg = std::get_if<unique_ptr<RequestMessage>>(&msg);
    return !!reqMsg;
}

bool LSPMessage::isNotification() const {
    auto notifMsg = std::get_if<unique_ptr<NotificationMessage>>(&msg);
    return !!notifMsg;
}

bool LSPMessage::isResponse() const {
    auto respMsg = std::get_if<unique_ptr<ResponseMessage>>(&msg);
    return !!respMsg;
}

const RequestMessage &LSPMessage::asRequest() const {
    auto reqMsg = std::get_if<unique_ptr<RequestMessage>>(&msg);
    if (reqMsg) {
        return *reqMsg->get();
    }
    Exception::raise("LSPMessage is not a request.");
}

RequestMessage &LSPMessage::asRequest() {
    auto reqMsg = std::get_if<unique_ptr<RequestMessage>>(&msg);
    if (reqMsg) {
        return *reqMsg->get();
    }
    Exception::raise("LSPMessage is not a request.");
}

const NotificationMessage &LSPMessage::asNotification() const {
    auto reqMsg = std::get_if<unique_ptr<NotificationMessage>>(&msg);
    if (reqMsg) {
        return *reqMsg->get();
    }
    Exception::raise("LSPMessage is not a notification.");
}

NotificationMessage &LSPMessage::asNotification() {
    auto notifMsg = std::get_if<unique_ptr<NotificationMessage>>(&msg);
    if (notifMsg) {
        return *notifMsg->get();
    }
    Exception::raise("LSPMessage is not a notification.");
}

const ResponseMessage &LSPMessage::asResponse() const {
    auto respMsg = std::get_if<unique_ptr<ResponseMessage>>(&msg);
    if (respMsg) {
        return *respMsg->get();
    }
    Exception::raise("LSPMessage is not a response.");
}

ResponseMessage &LSPMessage::asResponse() {
    auto respMsg = std::get_if<unique_ptr<ResponseMessage>>(&msg);
    if (respMsg) {
        return *respMsg->get();
    }
    Exception::raise("LSPMessage is not a response.");
}

LSPMethod LSPMessage::method() const {
    if (isRequest()) {
        return asRequest().method;
    } else if (isNotification()) {
        return asNotification().method;
    } else {
        Exception::raise("LSPMessage is neither a request nor a notification.");
    }
}

string LSPMessage::toJSON() const {
    if (isRequest()) {
        return asRequest().toJSON();
    } else if (isNotification()) {
        return asNotification().toJSON();
    } else if (isResponse()) {
        return asResponse().toJSON();
    } else {
        Exception::raise("LSPMessage is not a request, notification, or a response.");
    }
}
} // namespace sorbet::realmain::lsp