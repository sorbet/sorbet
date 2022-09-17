#ifndef SORBET_LSP_TYPECHECKEPOCHMANAGER_H
#define SORBET_LSP_TYPECHECKEPOCHMANAGER_H

#include "core/core.h"
#include <memory>

namespace sorbet::core::lsp {
class PreemptionTaskManager;
class TypecheckEpochManager final {
public:
    struct TypecheckingStatus {
        bool slowPathRunning;
        bool slowPathWasCanceled;
        uint32_t epoch;
    };

private:
    // Used to linearize operations involving lastCommittedLSPEpoch.
    mutable absl::Mutex epochMutex;
    // Contains the current edit version (epoch) that the processing thread is typechecking or has
    // typechecked last. Is bumped by the typechecking thread.
    std::atomic<uint32_t> currentlyProcessingLSPEpoch GUARDED_BY(epochMutex);
    // Should always be `>= currentlyProcessingLSPEpoch` (modulo overflows).
    // If value in `lspEpochInvalidator` is different from `currentlyProcessingLSPEpoch`, then LSP wants the current
    // request to be cancelled. Is bumped by the preprocessor thread (which determines cancellations).
    std::atomic<uint32_t> lspEpochInvalidator GUARDED_BY(epochMutex);
    // Should always be >= currentlyProcessingLSPEpoch. Is bumped by the typechecking thread.
    // Contains the epoch of the last committed slow path.
    // If lastCommittedLSPEpoch != currentlyProcessingLSPEpoch, then GlobalState is currently running a slow path.
    std::atomic<uint32_t> lastCommittedLSPEpoch GUARDED_BY(epochMutex);
    // Thread ID of the typechecking thread. Lazily set.
    mutable std::optional<std::thread::id> typecheckingThreadId;
    // Thread ID of the preprocess thread. Lazily set.
    mutable std::optional<std::thread::id> messageProcessingThreadId;

    TypecheckingStatus getStatusInternal() const EXCLUSIVE_LOCKS_REQUIRED(epochMutex);

public:
    static void assertConsistentThread(std::optional<std::thread::id> &expectedThreadId, std::string_view method,
                                       std::string_view threadName);
    // Indicates an intent to begin committing a specific epoch.
    // Run only from the typechecking thread.
    void startCommitEpoch(uint32_t epoch);
    // Returns 'true' if the currently running typecheck run has been canceled.
    bool wasTypecheckingCanceled() const;
    // Retrieve the status of typechecking.
    TypecheckingStatus getStatus() const;
    // Tries to cancel a running slow path on this GlobalState or its descendent. Returns true if it succeeded, false if
    // the slow path was unable to be canceled.
    // Run only from processing thread.
    bool tryCancelSlowPath(uint32_t newEpoch);
    // Run only from the typechecking thread.
    // Tries to commit the given epoch. Returns true if the commit succeeeded, or false if it was canceled.
    // The presence of PreemptionTaskManager determines if this commit is preemptible.
    bool tryCommitEpoch(core::GlobalState &gs, uint32_t epoch, bool isCancelable,
                        std::optional<std::shared_ptr<PreemptionTaskManager>> preemptionManager,
                        std::function<void()> typecheck);
    // Grabs the epoch lock, and calls function with the current typechecking status.
    void withEpochLock(std::function<void(TypecheckingStatus)> lambda) const;
};

} // namespace sorbet::core::lsp
#endif
