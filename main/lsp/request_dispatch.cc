#include "common/Timer.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/lsp.h"
#include "main/lsp/notifications/notifications.h"
#include "main/lsp/requests/requests.h"
#include "main/pipeline/pipeline.h"

using namespace std;

namespace sorbet::realmain::lsp {

void LSPLoop::processRequest(const string &json) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(LSPMessage::fromClient(json));
    LSPLoop::processRequests(move(messages));
}

void LSPLoop::processRequest(std::unique_ptr<LSPMessage> msg) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(move(msg));
    processRequests(move(messages));
}

void LSPLoop::processRequests(vector<unique_ptr<LSPMessage>> messages) {
    for (auto &message : messages) {
        preprocessor.preprocessAndEnqueue(move(message));
    }
    {
        absl::MutexLock lck(taskQueueMutex.get());
        ENFORCE(taskQueue->paused == false, "__PAUSE__ not supported in single-threaded mode.");
        for (auto &task : taskQueue->pendingTasks) {
            runTask(move(task));
        }
        taskQueue->pendingTasks.clear();
    }
}

void LSPLoop::runTask(unique_ptr<LSPTask> task) {
    prodCategoryCounterInc("lsp.messages.processed", task->methodString());
    {
        Timer timeit("LSPTask::index");
        timeit.setTag("method", task->methodString());
        task->index(indexer);
    }
    if (task->finalPhase() == LSPTask::Phase::INDEX) {
        // Task doesn't need the typechecker.
        return;
    }

    // Check if the task is a dangerous task, as those are scheduled specially.
    if (auto *dangerousTask = dynamic_cast<LSPDangerousTypecheckerTask *>(task.get())) {
        if (auto *editTask = dynamic_cast<SorbetWorkspaceEditTask *>(dangerousTask)) {
            unique_ptr<SorbetWorkspaceEditTask> edit(editTask);
            task.release();
            if (edit->canTakeFastPath(indexer)) {
                // Can run on fast path synchronously; it should complete quickly.
                typecheckerCoord.syncRun(move(edit));
            } else {
                // Must run on slow path; this method is async in multithreaded environments, and blocks in
                // single threaded environments.
                typecheckerCoord.typecheckOnSlowPath(move(edit));
            }
        } else if (auto *initializedTask = dynamic_cast<InitializedTask *>(dangerousTask)) {
            unique_ptr<InitializedTask> initialized(initializedTask);
            task.release();
            typecheckerCoord.initialize(move(initialized));
        } else {
            // Must be a new type of dangerous task we don't know about.
            // Please do not add new dangerous tasks to the codebase. Try to surface whatever functionality you
            // require safely through LSPTypecheckerCoordinator.
            ENFORCE(false);
        }
    } else {
        // Run synchronously.
        typecheckerCoord.syncRun(move(task));
    }
}
} // namespace sorbet::realmain::lsp
