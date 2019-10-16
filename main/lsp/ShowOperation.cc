#include "main/lsp/ShowOperation.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"

namespace sorbet::realmain::lsp {
using namespace std;

unique_ptr<LSPMessage> makeShowOperation(std::string_view operationName, std::string_view description,
                                         SorbetOperationStatus status) {
    return make_unique<LSPMessage>(make_unique<NotificationMessage>(
        "2.0", LSPMethod::SorbetShowOperation,
        make_unique<SorbetShowOperationParams>(string(operationName), string(description), status)));
}

ShowOperation::ShowOperation(LSPOutput &output, const LSPConfiguration &config, std::string_view operationName,
                             std::string_view description)
    : output(output), config(config), operationName(string(operationName)), description(string(description)) {
    if (config.enableOperationNotifications) {
        output.write(makeShowOperation(this->operationName, this->description, SorbetOperationStatus::Start));
    }
}

ShowOperation::~ShowOperation() {
    if (config.enableOperationNotifications) {
        output.write(makeShowOperation(this->operationName, this->description, SorbetOperationStatus::End));
    }
}
} // namespace sorbet::realmain::lsp
