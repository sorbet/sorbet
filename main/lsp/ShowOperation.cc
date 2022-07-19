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
        case ShowOperation::Kind::SlowPathBlocking:
            return "SlowPathBlocking";
        case ShowOperation::Kind::SlowPathNonBlocking:
            return "SlowPathNonBlocking";
        case ShowOperation::Kind::FastPath:
            return "FastPath";
        case ShowOperation::Kind::References:
            return "References";
        case ShowOperation::Kind::SymbolSearch:
            return "SymbolSearch";
        case ShowOperation::Kind::Rename:
            return "Rename";
        case ShowOperation::Kind::MoveMethod:
            return "MoveMethod";
    }
}

string_view kindToDescription(ShowOperation::Kind kind) {
    switch (kind) {
        case ShowOperation::Kind::Indexing:
            return "Indexing files...";
        case ShowOperation::Kind::SlowPathBlocking:
            return "Typechecking...";
        case ShowOperation::Kind::SlowPathNonBlocking:
            return "Typechecking in background";
        case ShowOperation::Kind::FastPath:
            return "Typechecking in foreground...";
        case ShowOperation::Kind::References:
            return "Finding all references...";
        case ShowOperation::Kind::SymbolSearch:
            return "Workspace symbol search...";
        case ShowOperation::Kind::Rename:
            return "Renaming...";
        case ShowOperation::Kind::MoveMethod:
            return "Moving...";
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
