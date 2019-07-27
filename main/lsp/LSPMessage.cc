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

optional<MessageId> LSPMessage::id() const {
    if (isRequest()) {
        return asRequest().id;
    } else if (isResponse()) {
        return asResponse().id;
    }
    return nullopt;
}

bool LSPMessage::isDelayable() const {
    if (isResponse() || canceled) {
        // Client responses to our inquiries or canceled requests should never block file update merges.
        return true;
    }
    switch (method()) {
        // These shouldn't be delayed or moved.
        case LSPMethod::Exit:
        case LSPMethod::Initialize:
        case LSPMethod::Initialized:
        case LSPMethod::Shutdown:
        case LSPMethod::PAUSE:
        case LSPMethod::RESUME:
        // Definition, reference, and workspace symbol requests are typically requested directly by the user, so we
        // shouldn't delay processing them.
        case LSPMethod::TextDocumentDefinition:
        case LSPMethod::TextDocumentCodeAction:
        case LSPMethod::TextDocumentReferences:
        case LSPMethod::WorkspaceSymbol:
        // These requests involve a specific file location, and should never be delayed.
        case LSPMethod::TextDocumentHover:
        case LSPMethod::TextDocumentCompletion:
        case LSPMethod::TextDocumentSignatureHelp:
        // These are file updates. They shouldn't be delayed (but they can be combined/expedited).
        case LSPMethod::TextDocumentDidOpen:
        case LSPMethod::TextDocumentDidChange:
        case LSPMethod::TextDocumentDidClose:
        case LSPMethod::SorbetWorkspaceEdit:
        case LSPMethod::SorbetWatchmanFileChange:
            return false;
        // VS Code requests document symbols automatically and in the background. It's OK to delay these requests.
        case LSPMethod::TextDocumentDocumentSymbol:
        // Sorbet processes these requests before they hit the server's queue.
        case LSPMethod::$CancelRequest:
        // Sorbet produces SorbetErrors for a variety of common things, including when it receives a message type it
        // doesn't care about (like textDocument/didSave). We should be able to merge file updates through them.
        case LSPMethod::SorbetError:
        // These will never show up in the server's queue, but are included for complete case coverage.
        case LSPMethod::WindowShowMessage:
        case LSPMethod::TextDocumentPublishDiagnostics:
        case LSPMethod::SorbetShowOperation:
        case LSPMethod::SorbetTypecheckRunInfo:
            return true;
    }
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