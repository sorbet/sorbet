#ifndef RUBY_TYPER_LSP_LSPPREPROCESSOR_H
#define RUBY_TYPER_LSP_LSPPREPROCESSOR_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/ErrorQueue.h"
#include "core/NameHash.h"
#include "core/core.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/TimeTravelingGlobalState.h"
#include <chrono>
#include <deque>
#include <optional>

namespace sorbet::realmain::lsp {

/** Used to store the state of LSP's internal request queue.  */
struct QueueState {
    std::deque<std::unique_ptr<LSPMessage>> pendingRequests;
    bool terminate = false;
    bool paused = false;
    int errorCode = 0;
    // Counters collected from worker threads.
    CounterState counters;
};

/**
 * The LSP preprocessor typically runs on an independent thread and performs the following tasks:
 * - Preprocesses and merges contiguous file updates before they are sent to the processor thread.
 * - Determines if edits should take the fast or slow path.
 * - Is the source-of-truth for the latest file updates.
 * - Clones initialGS so that the processor thread can perform typechecking on the clone.
 */
class LSPPreprocessor final {
private:
    /**
     * This global state is used for indexing. It accumulates a huge nametable of all global things,
     * and is updated as global things are added/removed/updated. It is never discarded.
     *
     * Typechecking is never run on this global state directly. Instead, LSPPreprocessor clones `initialGS` and passes
     * it to the processing thread for use during typechecking.
     */
    TimeTravelingGlobalState ttgs;
    LSPConfiguration config;
    WorkerPool &workers;
    std::shared_ptr<spdlog::logger> logger;
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.

    // The current set of open files as of the latest edit preprocessed. Used to canonicalize file edits into a
    // standard format.
    UnorderedSet<std::string> openFiles;

    // ErrorQueue used for finalGS. Separate from initialGS's ErrorQueue to prevent indexing errors from making it to
    // the client.
    std::shared_ptr<core::ErrorQueue> finalGSErrorQueue;

    // Indicates the next ID to use on an incoming message. Used to refer to messages by ID.
    int nextMessageId = 1;

    /**
     * Merges all consecutive file updates into a single update. File updates are also merged if they are only separated
     * by *delayable* requests (see LSPMessage::isDelayable()). Updates are merged into the earliest file update in the
     * sequence.
     *
     * Example: (E = edit, D = delayable non-edit, M = arbitrary non-edit)
     * {[M1][E1][E2][D1][E3]} => {[M1][E1-3][D1]}
     */
    void mergeFileChanges(QueueState &state);

    std::unique_ptr<LSPMessage> makeAndCommitWorkspaceEdit(std::unique_ptr<SorbetWorkspaceEditParams> params,
                                                           std::unique_ptr<SorbetWorkspaceEditCounts> counts,
                                                           std::unique_ptr<LSPMessage> oldMsg);

    /* The following methods convert edits into LSPFileUpdates. */

    void canonicalizeEdits(std::unique_ptr<DidChangeTextDocumentParams> changeParams, LSPFileUpdates &updates) const;
    void canonicalizeEdits(std::unique_ptr<DidOpenTextDocumentParams> openParams, LSPFileUpdates &updates) const;
    void canonicalizeEdits(std::unique_ptr<DidCloseTextDocumentParams> closeParams, LSPFileUpdates &updates) const;
    void canonicalizeEdits(std::unique_ptr<WatchmanQueryResponse> queryResponse, LSPFileUpdates &updates) const;
    void mergeEdits(int toId, LSPFileUpdates &to, int fromId, LSPFileUpdates &from);

    /**
     * Returns a global state for typechecking, cloned from initialGS. Note: The clone does not share an error queue
     * with initialGS.
     */
    std::unique_ptr<core::GlobalState> getTypecheckingGS() const;

public:
    LSPPreprocessor(std::unique_ptr<core::GlobalState> initialGS, LSPConfiguration config, WorkerPool &workers,
                    const std::shared_ptr<spdlog::logger> &logger);

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
    void preprocessAndEnqueue(QueueState &state, std::unique_ptr<LSPMessage> msg, absl::Mutex &stateMtx);

    std::unique_ptr<Joinable> runPreprocessor(QueueState &incomingQueue, absl::Mutex &incomingMtx,
                                              QueueState &processingQueue, absl::Mutex &processingMtx);
};

} // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LSPPREPROCESSOR_H