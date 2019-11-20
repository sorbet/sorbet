#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/json_types.h"
#include "spdlog/spdlog.h"
#include <iostream>

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
// Is this a notification the server should be sending?
bool isServerNotification(const LSPMethod method) {
    switch (method) {
        case LSPMethod::$CancelRequest:
        case LSPMethod::TextDocumentPublishDiagnostics:
        case LSPMethod::WindowShowMessage:
        case LSPMethod::SorbetTypecheckRunInfo:
        case LSPMethod::SorbetShowOperation:
            return true;
        default:
            return false;
    }
}
} // namespace

void LSPOutput::write(unique_ptr<LSPMessage> msg) {
    // Sanity check that message is acceptable.
    if (msg->isResponse()) {
        ENFORCE(msg->asResponse().result || msg->asResponse().error,
                "A valid ResponseMessage must have a result or an error.");
    } else if (msg->isNotification()) {
        ENFORCE(isServerNotification(msg->method()));
    }
    {
        // Protect with a lock to make it possible for multiple threads to concurrently write to the same output object.
        absl::MutexLock lock(&mtx);
        rawWrite(move(msg));
    }
}

void LSPOutput::write(unique_ptr<ResponseMessage> msg) {
    write(make_unique<LSPMessage>(move(msg)));
}

void LSPOutput::write(unique_ptr<NotificationMessage> msg) {
    write(make_unique<LSPMessage>(move(msg)));
}

LSPStdout::LSPStdout(shared_ptr<spdlog::logger> &logger) : logger(logger) {}

void LSPStdout::rawWrite(unique_ptr<LSPMessage> msg) {
    auto json = msg->toJSON();
    string outResult = fmt::format("Content-Length: {}\r\n\r\n{}", json.length(), json);
    logger->debug("Write: {}\n", json);
    cout << outResult << flush;
}

void LSPOutputToVector::rawWrite(unique_ptr<LSPMessage> msg) {
    output.push_back(move(msg));
}

vector<unique_ptr<LSPMessage>> LSPOutputToVector::getOutput() {
    absl::MutexLock lock(&mtx);
    return move(output);
}

} // namespace sorbet::realmain::lsp
