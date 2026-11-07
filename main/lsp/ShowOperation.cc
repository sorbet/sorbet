#include "main/lsp/ShowOperation.h"
#include "absl/strings/str_cat.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include <atomic>

namespace sorbet::realmain::lsp {
using namespace std;

namespace {

unique_ptr<LSPMessage> makeShowOperation(string operationName, string description, SorbetOperationStatus status) {
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

// ShowOperations are created on several threads, so progress tokens come from a shared counter.
atomic<uint64_t> nextProgressId{0};

unique_ptr<LSPMessage> makeWorkDoneProgress(string token, unique_ptr<WorkDoneProgress> value) {
    return make_unique<LSPMessage>(make_unique<NotificationMessage>(
        "2.0", LSPMethod::$Progress, make_unique<ProgressParams>(move(token), move(value))));
}

} // namespace

ShowOperation::ShowOperation(const LSPConfiguration &config, Kind kind)
    : config(config), operationName(kindToOperationName(kind)), description(kindToDescription(kind)) {
    auto &clientConfig = config.getClientConfig();
    if (clientConfig.enableOperationNotifications) {
        config.output->write(makeShowOperation(this->operationName, this->description, SorbetOperationStatus::Start));
    } else if (clientConfig.clientWorkDoneProgressSupport) {
        auto id = nextProgressId.fetch_add(1);
        auto token = absl::StrCat("sorbet/", this->operationName, "/", id);
        // The spec requires creating server-initiated tokens with a request first. We don't wait
        // for the response: messages are delivered in order, so the client sees the request before the progress.
        // Sorbet ignores the response (see LSPPreprocessor::preprocessAndEnqueue).
        auto createId = absl::StrCat(WORK_DONE_PROGRESS_CREATE_ID_PREFIX, id);
        config.output->write(make_unique<LSPMessage>(
            make_unique<RequestMessage>("2.0", move(createId), LSPMethod::WindowWorkDoneProgressCreate,
                                        make_unique<WorkDoneProgressCreateParams>(token))));
        auto begin = make_unique<WorkDoneProgress>(WorkDoneProgressKind::Begin);
        begin->title = this->description;
        begin->cancellable = false;
        config.output->write(makeWorkDoneProgress(token, move(begin)));
        this->progressToken = move(token);
    }
}

ShowOperation::~ShowOperation() {
    if (config.getClientConfig().enableOperationNotifications) {
        config.output->write(makeShowOperation(std::move(this->operationName), std::move(this->description),
                                               SorbetOperationStatus::End));
    } else if (this->progressToken.has_value()) {
        config.output->write(makeWorkDoneProgress(std::move(*this->progressToken),
                                                  make_unique<WorkDoneProgress>(WorkDoneProgressKind::End)));
    }
}
} // namespace sorbet::realmain::lsp

//
