#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
using namespace sorbet::realmain::lsp;
// Is this a notification the server should be sending?
bool isServerNotification(const LSPMethod method) {
    switch (method) {
        case LSPMethod::$CancelRequest:
        case LSPMethod::TextDocumentPublishDiagnostics:
        case LSPMethod::WindowShowMessage:
        case LSPMethod::SorbetShowOperation:
        case LSPMethod::SorbetTypecheckRunInfo:
            return true;
        default:
            return false;
    }
}
} // namespace

void LSPOutput::write(std::unique_ptr<LSPMessage> msg) {
    // Sanity check that message is acceptable.
    if (msg->isResponse()) {
        ENFORCE(msg->asResponse().result || msg->asResponse().error,
                "A valid ResponseMessage must have a result or an error.");
    } else if (msg->isNotification()) {
        ENFORCE(isServerNotification(msg->method()));
    }
    rawWrite(move(msg));
}

} // namespace sorbet::realmain::lsp
