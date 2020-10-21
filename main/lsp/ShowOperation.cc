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

string_view kindToOperationName(ShowOperation::Kind kind) {
    switch (kind) {
        case ShowOperation::Kind::Indexing:
            return "Indexing";
            break;
        case ShowOperation::Kind::SlowPathBlocking:
            return "SlowPathBlocking";
            break;
        case ShowOperation::Kind::SlowPathNonBlocking:
            return "SlowPathNonBlocking";
            break;
        case ShowOperation::Kind::References:
            return "References";
            break;
        case ShowOperation::Kind::SymbolSearch:
            return "SymbolSearch";
            break;
    }
}

string_view kindToDescription(ShowOperation::Kind kind) {
    switch (kind) {
        case ShowOperation::Kind::Indexing:
            return "Indexing files...";
            break;
        case ShowOperation::Kind::SlowPathBlocking:
            return "Typechecking...";
            break;
        case ShowOperation::Kind::SlowPathNonBlocking:
            return "Typchecking in background";
            break;
        case ShowOperation::Kind::References:
            return "Finding all references...";
            break;
        case ShowOperation::Kind::SymbolSearch:
            return "Workspace symbol search...";
            break;
    }
}

} // namespace

ShowOperation::ShowOperation(const LSPConfiguration &config, Kind kind)
    : config(config), operationName(kindToOperationName(kind)), description(kindToDescription(kind)) {
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

//
