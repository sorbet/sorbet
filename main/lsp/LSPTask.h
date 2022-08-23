#ifndef RUBY_TYPER_LSPTASK_H
#define RUBY_TYPER_LSPTASK_H

#include "main/lsp/AbstractRenamer.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPTypechecker.h"
#include "main/lsp/json_types.h"

namespace absl {
class Notification;
}

namespace sorbet::realmain::lsp {
class LSPIndexer;
class LSPPreprocessor;
class DocumentHighlight;

enum class FieldAccessorType { None, Reader, Writer, Accessor };

struct AccessorInfo {
    FieldAccessorType accessorType = FieldAccessorType::None;
    core::FieldRef fieldSymbol;
    core::MethodRef readerSymbol;
    core::MethodRef writerSymbol;
};

/**
 * A work unit that needs to execute on the typechecker thread. Subclasses implement `run`.
 * Contains miscellaneous helper methods that are useful in multiple tasks.
 *
 * NOTE: If `enableMultithreading` is set to `true`, then this task cannot preempt slow path typechecking.
 */
class LSPTask {
protected:
    const LSPConfiguration &config;

    // Task helper methods.

    std::vector<std::unique_ptr<core::lsp::QueryResponse>>
    getReferencesToSymbol(LSPTypecheckerInterface &typechecker, core::SymbolRef symbol,
                          std::vector<std::unique_ptr<core::lsp::QueryResponse>> &&priorRefs = {}) const;

    std::vector<std::unique_ptr<core::lsp::QueryResponse>>
    getReferencesToSymbolInPackage(LSPTypecheckerInterface &typechecker, core::NameRef packageName,
                                   core::SymbolRef symbol,
                                   std::vector<std::unique_ptr<core::lsp::QueryResponse>> &&priorRefs = {}) const;

    std::vector<std::unique_ptr<core::lsp::QueryResponse>>
    getReferencesToSymbolInFile(LSPTypecheckerInterface &typechecker, core::FileRef file, core::SymbolRef symbol,
                                std::vector<std::unique_ptr<core::lsp::QueryResponse>> &&priorRefs = {}) const;

    std::vector<std::unique_ptr<DocumentHighlight>>
    getHighlights(LSPTypecheckerInterface &typechecker,
                  const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &responses) const;
    void addLocIfExists(const core::GlobalState &gs, std::vector<std::unique_ptr<Location>> &locs, core::Loc loc) const;
    std::vector<std::unique_ptr<Location>>
    extractLocations(const core::GlobalState &gs,
                     const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                     std::vector<std::unique_ptr<Location>> locations = {}) const;

    // Given a method or field symbol, checks if the symbol belongs to a `prop`, `const`, `attr_reader`, `attr_writer`,
    // etc, and populates an AccessorInfo object.
    AccessorInfo getAccessorInfo(const core::GlobalState &gs, core::SymbolRef symbol) const;

    // Get references to the given accessor. If `info.accessorType` is `None`, it returns references to `fallback` only.
    std::vector<std::unique_ptr<core::lsp::QueryResponse>>
    getReferencesToAccessor(LSPTypecheckerInterface &typechecker, const AccessorInfo info, core::SymbolRef fallback,
                            std::vector<std::unique_ptr<core::lsp::QueryResponse>> &&priorRefs = {}) const;

    // Get references to the given accessor in the given file. If `info.accessorType` is `None`, it returns highlights
    // to `fallback` only.
    std::vector<std::unique_ptr<core::lsp::QueryResponse>>
    getReferencesToAccessorInFile(LSPTypecheckerInterface &typechecker, core::FileRef fref, const AccessorInfo info,
                                  core::SymbolRef fallback,
                                  std::vector<std::unique_ptr<core::lsp::QueryResponse>> &&priorRefs = {}) const;

    LSPTask(const LSPConfiguration &config, LSPMethod method);

public:
    virtual ~LSPTask();

    const LSPMethod method;

    // Measures the end-to-end latency of this task from the first message received from the client. Can be nullptr
    // if not relevant.
    std::unique_ptr<Timer> latencyTimer;

    enum class Phase {
        PREPROCESS = 1,
        INDEX = 2,
        RUN = 3,
    };

    // Get this task's method as a string.
    ConstExprStr methodString() const;

    /**
     * If `true`, this task can be delayed in favor of processing other tasks sooner. Defaults to `false`.
     */
    virtual bool isDelayable() const;

    // Attempts to cancel the task if it corresponds to the given request ID. Returns `true` if cancelation succeeds.
    // The default implementation returns `false`.
    virtual bool cancel(const MessageId &id);

    virtual bool canPreempt(const LSPIndexer &) const;

    // Returns true if the task can operate on typechecker stale state.
    virtual bool canUseStaleData() const;

    virtual bool needsMultithreading(const LSPIndexer &) const;

    // Returns the phase at which the task is complete. Some tasks only need to interface with the preprocessor or the
    // indexer. The default implementation returns RUN.
    virtual Phase finalPhase() const;

    // Some tasks, like request cancelations, need to interface with the preprocessor. The default implementation is
    // a no-op. Is only ever invoked from the preprocessor thread.
    virtual void preprocess(LSPPreprocessor &preprocessor);

    // Some tasks, like edits, need to interface with the indexer. Is only ever invoked from the processing
    // thread, and is guaranteed to be invoked exactly once. The default implementation is a no-op.
    // May be run from the processing thread (normally) or the typechecking thread (if preempting).
    virtual void index(LSPIndexer &indexer);

    // Runs the task. Is only ever invoked from the typechecker thread. Since it is exceedingly rare for a request to
    // not need to interface with the typechecker, this method must be implemented by all subclasses.
    virtual void run(LSPTypecheckerInterface &typechecker) = 0;
};

/**
 * A specialized version of LSPTask for LSP requests (which must be responded to).
 */
class LSPRequestTask : public LSPTask {
protected:
    const MessageId id;

    LSPRequestTask(const LSPConfiguration &config, MessageId id, LSPMethod method);

    virtual std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerInterface &typechecker) = 0;

public:
    void run(LSPTypecheckerInterface &typechecker) override;

    // Requests cannot override this method, as runRequest must be run (and it only runs during the RUN phase).
    Phase finalPhase() const override;

    bool cancel(const MessageId &id) override;
};

// Doubles as the `methodString` for a `TextDocumentCompletion` LSPTask and also as
// the prefix for any metrics collected during completion itself.
#define LSP_COMPLETION_METRICS_PREFIX "textDocument.completion"

/**
 * A special form of LSPTask that has direct access to the typechecker and controls its own scheduling.
 * Is only used for slow path-related tasks. Do not use for anything else.
 */
class LSPDangerousTypecheckerTask : public LSPTask {
protected:
    LSPDangerousTypecheckerTask(const LSPConfiguration &config, LSPMethod method);

public:
    // Should never be called; throws an exception. May be overridden by tasks that can be dangerous or not dangerous.
    virtual void run(LSPTypecheckerInterface &typechecker) override;
    // Performs the actual work on the task.
    virtual void runSpecial(LSPTypechecker &typechecker, WorkerPool &worker) = 0;
    // Tells the scheduler how long to wait before it can schedule more tasks.
    virtual void schedulerWaitUntilReady() = 0;
};

class LSPIndexer;
class TaskQueue;

/**
 * Represents a preemption task. When run, it will run all tasks at the head of `taskQueue` that can preempt.
 */
class LSPQueuePreemptionTask final : public LSPTask {
    absl::Notification &finished;
    TaskQueue &taskQueue;
    LSPIndexer &indexer;

public:
    LSPQueuePreemptionTask(const LSPConfiguration &config, absl::Notification &finished, TaskQueue &taskQueue,
                           LSPIndexer &indexer);

    void run(LSPTypecheckerInterface &tc) override;
};

} // namespace sorbet::realmain::lsp

#endif
