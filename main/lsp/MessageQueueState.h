#ifndef SORBET_LSP_MESSAGE_QUEUE_STATE_H
#define SORBET_LSP_MESSAGE_QUEUE_STATE_H

#include "common/counters/Counters.h"
#include <deque>
#include <memory>

namespace sorbet::realmain::lsp {

class LSPMessage;

struct MessageQueueState {
    std::deque<std::unique_ptr<LSPMessage>> pendingRequests;
    bool terminate = false;
    int errorCode = 0;
    // Counters collected from other threads.
    CounterState counters;

    class NotifyOnDestruction {
        absl::Mutex &mutex;
        bool &flag;

    public:
        NotifyOnDestruction(MessageQueueState &state, absl::Mutex &mutex) : mutex(mutex), flag(state.terminate){};
        ~NotifyOnDestruction() {
            absl::MutexLock lck(&mutex);
            flag = true;
        }
    };
};

} // namespace sorbet::realmain::lsp
#endif
