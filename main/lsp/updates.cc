#include "common/Timer.h"
#include "core/Error.h"
#include "core/Files.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/parser.h"
#include "core/errors/resolver.h"
#include "core/lsp/QueryResponse.h"
#include "lsp.h"
#include "namer/namer.h"
#include "resolver/resolver.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::ShowOperation::ShowOperation(LSPLoop &loop, string_view operationName, string_view description)
    : loop(loop), operationName(string(operationName)), description(string(description)) {
    if (loop.enableOperationNotifications) {
        auto params = make_unique<SorbetShowOperationParams>(this->operationName, this->description,
                                                             SorbetOperationStatus::Start);
        loop.sendMessage(
            LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetShowOperation, move(params))));
    }
}

LSPLoop::ShowOperation::~ShowOperation() {
    if (loop.enableOperationNotifications) {
        auto params = make_unique<SorbetShowOperationParams>(operationName, description, SorbetOperationStatus::End);
        loop.sendMessage(
            LSPMessage(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetShowOperation, move(params))));
    }
}

} // namespace sorbet::realmain::lsp
