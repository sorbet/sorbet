#ifndef RUBY_TYPER_LSPLOOP_H
#define RUBY_TYPER_LSPLOOP_H

#include "core/core.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPPreprocessor.h"
#include "main/lsp/LSPTypecheckerCoordinator.h"
#include <chrono>
#include <optional>

//  _     ____  ____
// | |   / ___||  _ _\
// | |   \___ \| |_) |
// | |___ ___) |  __/
// |_____|____/|_|
//
//
// This is an implementation of LSP protocol (version 3.13) for Sorbet
namespace sorbet::realmain::lsp {

class LSPInput;
class LSPConfiguration;

class LSPLoop {
    friend class LSPWrapper;

    /** Encapsulates the active configuration for the language server. */
    std::shared_ptr<const LSPConfiguration> config;
    /** The LSP preprocessor standardizes incoming messages and combines edits. */
    LSPPreprocessor preprocessor;
    /** The LSP typechecker coordinator typechecks file updates and runs queries. */
    LSPTypecheckerCoordinator typecheckerCoord;
    /**
     * The time that LSP last sent metrics to statsd -- if `opts.statsdHost` was specified.
     */
    std::chrono::time_point<std::chrono::steady_clock> lastMetricUpdateTime;
    /** ID of the main thread, which actually processes LSP requests and performs typechecking. */
    std::thread::id mainThreadId;
    /** Global state that we keep up-to-date with file edits. We do _not_ typecheck using this global state! We clone
     * this global state every time we need to perform a slow path typechecking operation. */
    std::unique_ptr<core::GlobalState> initialGS;
    /** Contains file hashes for the files stored in `initialGS`. Used to determine if an edit can be typechecked
     * incrementally. */
    std::vector<core::FileHash> globalStateHashes;
    /** Contains a copy of the last edit committed on the slow path. Used in slow path cancelation logic. */
    LSPFileUpdates pendingTypecheckUpdates;
    /** Contains globalStateHashes evicted with `pendingTypecheckUpdates`. Used in slow path cancelation logic. */
    UnorderedMap<int, core::FileHash> pendingTypecheckEvictedStateHashes;

    std::unique_ptr<KeyValueStore> kvstore; // always null for now.
    /** A WorkerPool with 0 workers. */
    std::unique_ptr<WorkerPool> emptyWorkers;

    void processRequestInternal(LSPMessage &msg);

    /** Returns `true` if 5 minutes have elapsed since LSP last sent counters to statsd. */
    bool shouldSendCountersToStatsd(std::chrono::time_point<std::chrono::steady_clock> currentTime) const;
    /** Sends counters to statsd. */
    void sendCountersToStatsd(std::chrono::time_point<std::chrono::steady_clock> currentTime);

    /** Commits the given edit to `initialGS`, and returns a canonical LSPFileUpdates object containing indexed trees
     * and file hashes. Also handles canceling the running slow path. */
    LSPFileUpdates commitEdit(SorbetWorkspaceEditParams &edit);

public:
    LSPLoop(std::unique_ptr<core::GlobalState> initialGS, WorkerPool &workers,
            const std::shared_ptr<LSPConfiguration> &config);
    /**
     * Runs the language server on a dedicated thread. Returns the final global state if it exits cleanly, or nullopt
     * on error.
     *
     * Reads input messages from the provided input object.
     */
    std::optional<std::unique_ptr<core::GlobalState>> runLSP(std::shared_ptr<LSPInput> input);
    void processRequest(std::unique_ptr<LSPMessage> msg);
    void processRequest(const std::string &json);
    /**
     * Processes a batch of requests. Performs pre-processing to avoid unnecessary work.
     */
    void processRequests(std::vector<std::unique_ptr<LSPMessage>> messages);
    /**
     * (For tests only) Retrieve the number of times typechecking has run.
     */
    int getTypecheckCount();
};

std::optional<std::string> findDocumentation(std::string_view sourceCode, int beginIndex);
bool hasSimilarName(const core::GlobalState &gs, core::NameRef name, std::string_view pattern);
bool hideSymbol(const core::GlobalState &gs, core::SymbolRef sym);
std::unique_ptr<MarkupContent> formatRubyMarkup(MarkupKind markupKind, std::string_view rubyMarkup,
                                                std::optional<std::string_view> explanation);
std::string prettyTypeForMethod(const core::GlobalState &gs, core::SymbolRef method, core::TypePtr receiver,
                                core::TypePtr retType, const core::TypeConstraint *constraint);
std::string prettyTypeForConstant(const core::GlobalState &gs, core::SymbolRef constant);
core::TypePtr getResultType(const core::GlobalState &gs, core::TypePtr type, core::SymbolRef inWhat,
                            core::TypePtr receiver, const core::TypeConstraint *constr);
SymbolKind symbolRef2SymbolKind(const core::GlobalState &gs, core::SymbolRef sym);

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSPLOOP_H
