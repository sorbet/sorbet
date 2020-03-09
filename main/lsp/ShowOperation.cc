#include "main/lsp/ShowOperation.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {
using namespace std;

namespace {
unique_ptr<LSPMessage> makeShowOperation(std::string operationName, std::string description,
                                         SorbetOperationStatus status) {
    return make_unique<LSPMessage>(make_unique<NotificationMessage>(
        "2.0", LSPMethod::SorbetShowOperation,
        make_unique<SorbetShowOperationParams>(move(operationName), move(description), status)));
}
} // namespace

ShowOperation::ShowOperation(const LSPConfiguration &config, std::string operationName, std::string description)
    : config(config), operationName(move(operationName)), description(move(description)) {
    if (config.getClientConfig().enableOperationNotifications) {
        config.output->write(makeShowOperation(this->operationName, this->description, SorbetOperationStatus::Start));
    }
}

ShowOperation::~ShowOperation() {
    if (config.getClientConfig().enableOperationNotifications) {
        config.output->write(makeShowOperation(this->operationName, this->description, SorbetOperationStatus::End));
    }
}
} // namespace sorbet::realmain::lsp
