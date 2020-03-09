#include "LSPMessage.h"
#include "json_types.h"

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

bool MessageId::isNull() const {
    return !!get_if<JSONNullObject>(&id);
}

bool MessageId::equals(const MessageId &id) const {
    if (this->isInt()) {
        return id.isInt() && this->asInt() == id.asInt();
    } else if (this->isString()) {
        return id.isString() && this->asString() == id.asString();
    } else {
        return this->isNull() && id.isNull();
    }
}

unique_ptr<LSPMessage> makeSorbetError(LSPErrorCodes code, string_view message, optional<int> id = nullopt) {
    auto errorParams = make_unique<SorbetErrorParams>((int)code, string(message));
    if (id) {
        auto req = make_unique<RequestMessage>("2.0", *id, LSPMethod::SorbetError, move(errorParams));
        return make_unique<LSPMessage>(move(req));
    } else {
        auto notif = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetError, move(errorParams));
        return make_unique<LSPMessage>(move(notif));
    }
}

unique_ptr<LSPMessage> LSPMessage::fromClient(const string &json) {
    rapidjson::MemoryPoolAllocator<> alloc;
    rapidjson::Document d(&alloc);
    if (d.Parse(json.c_str()).HasParseError()) {
        return makeSorbetError(LSPErrorCodes::ParseError,
                               fmt::format("Last LSP request: `{}` is not a valid json object", json));
    }

    // Grab ID before parsing, as the value may get moved out.
    optional<int> id;
    if (d.HasMember("id") && d["id"].IsInt()) {
        id = d["id"].GetInt();
    }

    try {
        return make_unique<LSPMessage>(d);
    } catch (InvalidStringEnumError e) {
        if (e.enumName == "LSPMethod") {
            // We don't support whatever method the client is sending.
            return makeSorbetError(LSPErrorCodes::MethodNotFound, fmt::format("Unsupported LSP method: {}", e.value),
                                   id);
        } else {
            return makeSorbetError(LSPErrorCodes::InvalidParams,
                                   fmt::format("Unable to deserialize LSP request: {}", e.what()), id);
        }
    } catch (DeserializationError e) {
        // The client request was valid JSON, but was invalid in some way.
        return makeSorbetError(LSPErrorCodes::InvalidParams,
                               fmt::format("Unable to deserialize LSP request: {}", e.what()), id);
    }
}

LSPMessage::RawLSPMessage fromJSONValue(rapidjson::Document &d) {
    if (d.HasMember("id")) {
        // Method is required on requests, but responses lack it.
        if (d.HasMember("method")) {
            return RequestMessage::fromJSONValue(d.GetObject(), "root");
        } else {
            return ResponseMessage::fromJSONValue(d.GetObject(), "root");
        }
    } else {
        return NotificationMessage::fromJSONValue(d.GetObject(), "root");
    }
}

LSPMessage::RawLSPMessage fromJSON(const std::string &json) {
    rapidjson::MemoryPoolAllocator<> alloc;
    rapidjson::Document d(&alloc);
    d.Parse(json.c_str());
    return fromJSONValue(d);
}

LSPMessage::LSPMessage(RawLSPMessage msg) : msg(move(msg)) {}

LSPMessage::LSPMessage(rapidjson::Document &d) : LSPMessage::LSPMessage(fromJSONValue(d)) {}

LSPMessage::LSPMessage(const std::string &json) : LSPMessage::LSPMessage(fromJSON(json)) {}

LSPMessage::~LSPMessage() = default;

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

string LSPMessage::toJSON(bool prettyPrint) const {
    if (isRequest()) {
        return asRequest().toJSON(prettyPrint);
    } else if (isNotification()) {
        return asNotification().toJSON(prettyPrint);
    } else if (isResponse()) {
        return asResponse().toJSON(prettyPrint);
    } else {
        Exception::raise("LSPMessage is not a request, notification, or a response.");
    }
}
} // namespace sorbet::realmain::lsp
