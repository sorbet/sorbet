#ifndef RUBY_TYPER_LSP_LSPPREPROCESSOR_H
#define RUBY_TYPER_LSP_LSPPREPROCESSOR_H

#include "absl/synchronization/mutex.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/MessageQueueState.h"
#include <deque>

namespace sorbet::realmain::lsp {

class LSPTask;
class SorbetWorkspaceEditParams;
class DidChangeTextDocumentParams;
class DidCloseTextDocumentParams;
class DidOpenTextDocumentParams;
class WatchmanQueryResponse;
class CancelParams;

class TaskQueue final {
    absl::Mutex stateMutex;

    std::deque<std::unique_ptr<LSPTask>> pendingTasks ABSL_GUARDED_BY(stateMutex);
    bool terminated ABSL_GUARDED_BY(stateMutex) = false;
    bool paused ABSL_GUARDED_BY(stateMutex) = false;
    int errorCode ABSL_GUARDED_BY(stateMutex) = 0;

    // Counters collected from preprocessor thread
    CounterState counters ABSL_GUARDED_BY(stateMutex);

public:
    TaskQueue() = default;

    TaskQueue(const TaskQueue &other) = delete;
    TaskQueue(TaskQueue &&other) = delete;

    TaskQueue &operator=(const TaskQueue &other) = delete;
    TaskQueue &operator=(TaskQueue &&other) = delete;

    bool isTerminated() const ABSL_SHARED_LOCKS_REQUIRED(stateMutex);
    void terminate() ABSL_EXCLUSIVE_LOCKS_REQUIRED(stateMutex);

    bool isPaused() const ABSL_SHARED_LOCKS_REQUIRED(stateMutex);
    void pause() ABSL_EXCLUSIVE_LOCKS_REQUIRED(stateMutex);
    void resume() ABSL_EXCLUSIVE_LOCKS_REQUIRED(stateMutex);

    int getErrorCode() const ABSL_SHARED_LOCKS_REQUIRED(stateMutex);
    void setErrorCode(int code) ABSL_EXCLUSIVE_LOCKS_REQUIRED(stateMutex);

    CounterState &getCounters() ABSL_EXCLUSIVE_LOCKS_REQUIRED(stateMutex);

    const std::deque<std::unique_ptr<LSPTask>> &tasks() const ABSL_SHARED_LOCKS_REQUIRED(stateMutex);
    std::deque<std::unique_ptr<LSPTask>> &tasks() ABSL_EXCLUSIVE_LOCKS_REQUIRED(stateMutex);

    absl::Mutex *getMutex() ABSL_LOCK_RETURNED(stateMutex);

    bool ready() const ABSL_SHARED_LOCKS_REQUIRED(stateMutex);
};

/**
 * The LSP preprocessor typically runs on an independent thread and performs the following tasks:
 * - Preprocesses and merges contiguous file updates before they are sent to the typechecking thread.
 * - Determines if edits should take the fast or slow path.
 * - Is the source-of-truth for the latest file updates.
 * - Clones initialGS so that the typechecking thread can perform typechecking on the clone.
 * - Early rejects messages that are sent prior to initialization completion.
 * - Determines if a running slow path should be canceled, and undertakes canceling if so.
 */
class LSPPreprocessor final {
    const std::shared_ptr<LSPConfiguration> config;
    const std::shared_ptr<TaskQueue> taskQueue;
    /** ID of the thread that owns the preprocessor and is allowed to invoke methods on it. */
    std::thread::id owner;

    // A map from file path to file contents for open files.
    UnorderedMap<std::string, std::shared_ptr<core::File>> openFiles;

    // Indicates the next version to use on an incoming edit. Used to refer to edits by ID.
    uint32_t nextVersion = 1;

    /**
     * Merges all consecutive file updates into a single update. File updates are also merged if they are only separated
     * by *delayable* requests (see LSPMessage::isDelayable()). Updates are merged into the earliest file update in the
     * sequence.
     *
     * Example: (E = edit, D = delayable non-edit, M = arbitrary non-edit)
     * {[M1][E1][E2][D1][E3]} => {[M1][E1-3][D1]}
     */
    void mergeFileChanges() ABSL_EXCLUSIVE_LOCKS_REQUIRED(taskQueue->getMutex());

    /* The following methods convert edits into SorbetWorkspaceEditParams. */

    std::unique_ptr<SorbetWorkspaceEditParams>
    canonicalizeEdits(uint32_t v, std::unique_ptr<DidChangeTextDocumentParams> changeParams);
    std::unique_ptr<SorbetWorkspaceEditParams> canonicalizeEdits(uint32_t v,
                                                                 std::unique_ptr<DidOpenTextDocumentParams> openParams);
    std::unique_ptr<SorbetWorkspaceEditParams>
    canonicalizeEdits(uint32_t v, std::unique_ptr<DidCloseTextDocumentParams> closeParams);
    std::unique_ptr<SorbetWorkspaceEditParams>
    canonicalizeEdits(uint32_t v, std::unique_ptr<WatchmanQueryResponse> queryResponse) const;

    /**
     * Get the current contents of the file at the given path. Returns "" (empty string view) if file does not yet
     * exist.
     */
    std::string_view getFileContents(std::string_view path) const;

    bool ensureInitialized(const LSPMethod forMethod, const LSPMessage &msg) const;

    std::unique_ptr<LSPTask> getTaskForMessage(LSPMessage &msg);

    std::vector<std::string_view> openFilePaths() const;

public:
    LSPPreprocessor(std::shared_ptr<LSPConfiguration> config, std::shared_ptr<TaskQueue> taskQueue,
                    uint32_t initialVersion = 0);

    /**
     * Performs pre-processing on the incoming LSP request and appends it to the queue.
     *
     * * Merges changes to the same document + Watchman filesystem updates.
     * * Processes pause/ignore requests.
     * * Updates `openFiles` if a file opens/closes.
     * * Updates `config` if the client sends an `initialize` message.
     * * Indexes all files on filesystem if client sends an `initialized` message. If configured, will also send
     * progress notifications.
     *
     * It grabs the mutex before reading/writing `state`.
     */
    void preprocessAndEnqueue(std::unique_ptr<LSPMessage> msg);

    /**
     * [Test method] Pauses preprocessing in multithreaded mode. Makes it possible to deterministically preprocess a set
     * of updates without racing with other threads.
     */
    void pause();

    /**
     * [Test method] Resumes preprocessing in multithreaded mode.
     */
    void resume();

    /**
     * Cancels the request with the given ID if it has not started executing, and sends a response to the client
     * acknowledging the cancellation. Returns true if a task was canceled.
     */
    bool cancelRequest(const CancelParams &params);

    /**
     * Suspend preprocessing indefinitely. Is called before the language server shuts down.
     */
    void exit(int exitCode);

    /**
     * Get the current contents of the file at the given path if it is in `openFiles`.
     */
    std::optional<std::string_view> maybeGetFileContents(std::string_view path) const;

    std::unique_ptr<Joinable> runPreprocessor(MessageQueueState &messageQueue, absl::Mutex &messageQueueMutex);
};

} // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LSPPREPROCESSOR_H
