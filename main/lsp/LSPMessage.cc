#include "LSPMessage.h"

using namespace std;

namespace sorbet::realmain::lsp {
const rapidjson::Value NULL_VALUE = rapidjson::Value();

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

void LSPMessage::setParams(unique_ptr<rapidjson::Value> params) {
    if (isRequest()) {
        auto &r = asRequest();
        r.params = move(params);
    } else if (isNotification()) {
        auto &n = asNotification();
        n.params = move(params);
    } else {
        Exception::raise("LSPMessage is neither a request nor a notification.");
    }
}

std::string_view LSPMessage::method() const {
    if (isRequest()) {
        return asRequest().method;
    } else if (isNotification()) {
        return asNotification().method;
    } else {
        Exception::raise("LSPMessage is neither a request nor a notification.");
    }
}

const rapidjson::Value &LSPMessage::params() const {
    if (isRequest()) {
        if (auto &params = asRequest().params) {
            // optional unique_ptr => ref
            return **params;
        }
    } else if (isNotification()) {
        if (auto &params = asNotification().params) {
            if (auto singleParam = get_if<unique_ptr<rapidjson::Value>>(&(*params))) {
                // unique_ptr ptr -> ref
                return **singleParam;
            }
        }
    }
    return NULL_VALUE;
}

void LSPMessage::setCounter(int count) {
    if (isRequest()) {
        auto &r = asRequest();
        r.sorbetCounter = count;
    } else if (isNotification()) {
        auto &n = asNotification();
        n.sorbetCounter = count;
    } else if (isResponse()) {
        auto &rs = asResponse();
        rs.sorbetCounter = count;
    } else {
        Exception::raise("LSPMessage is not a request, notification, or a response.");
    }
}

int LSPMessage::counter() const {
    if (isRequest()) {
        return asRequest().sorbetCounter.value_or(-1);
    } else if (isNotification()) {
        return asNotification().sorbetCounter.value_or(-1);
    } else if (isResponse()) {
        return asResponse().sorbetCounter.value_or(-1);
    } else {
        Exception::raise("LSPMessage is not a request, notification, or a response.");
    }
}

void LSPMessage::setTimestamp(double timestamp) {
    if (isRequest()) {
        auto &r = asRequest();
        r.sorbetReceiveTimestamp = timestamp;
    } else if (isNotification()) {
        auto &n = asNotification();
        n.sorbetReceiveTimestamp = timestamp;
    } else if (isResponse()) {
        auto &rs = asResponse();
        rs.sorbetReceiveTimestamp = timestamp;
    } else {
        Exception::raise("LSPMessage is not a request, notification, or a response.");
    }
}

double LSPMessage::timestamp() const {
    if (isRequest()) {
        return asRequest().sorbetReceiveTimestamp.value_or(0.0);
    } else if (isNotification()) {
        return asNotification().sorbetReceiveTimestamp.value_or(0.0);
    } else if (isResponse()) {
        return asResponse().sorbetReceiveTimestamp.value_or(0.0);
    } else {
        Exception::raise("LSPMessage is not a request, notification, or a response.");
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