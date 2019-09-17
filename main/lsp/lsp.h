#ifndef RUBY_TYPER_LSPLOOP_H
#define RUBY_TYPER_LSPLOOP_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/ErrorQueue.h"
#include "core/NameHash.h"
#include "core/core.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPPreprocessor.h"
#include <chrono>
#include <deque>
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

enum class LSPErrorCodes {
    // Defined by JSON RPC
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602, // todo
    InternalError = -32603,
    ServerErrorStart = -32099,
    ServerErrorEnd = -32000,
    ServerNotInitialized = -32002, // todo: can be found by finalGs = nullptr
    UnknownErrorCode = -32001,

    // Defined by the LSP
    RequestCancelled = -32800,
};

/**
 * The result from processing one or more messages from the client.
 */
struct LSPResult {
    std::unique_ptr<core::GlobalState> gs;
    std::vector<std::unique_ptr<LSPMessage>> responses;

    static LSPResult make(std::unique_ptr<core::GlobalState> gs, std::unique_ptr<ResponseMessage> response);
};

/**
 * A version of ShowOperation that works on the preprocessor thread.
 */
class ShowOperationPreprocessorThread final {
private:
    const LSPConfiguration &config;
    absl::Mutex &mtx;
    std::deque<std::unique_ptr<LSPMessage>> &queue;
    const std::string operationName;
    const std::string description;

public:
    ShowOperationPreprocessorThread(const LSPConfiguration &config, absl::Mutex &mtx,
                                    std::deque<std::unique_ptr<LSPMessage>> &queue, std::string_view operationName,
                                    std::string_view description);
    ~ShowOperationPreprocessorThread();
};
class LSPLoop {
    friend class LSPWrapper;

    /**
     * Object that uses the RAII pattern to notify the client when a *slow* operation
     * starts and ends. Is used to provide user feedback in the status line of VS Code.
     */
    class ShowOperation final {
    private:
        const LSPLoop &loop;
        const std::string operationName;
        const std::string description;

    public:
        ShowOperation(const LSPLoop &loop, std::string_view operationName, std::string_view description);
        ~ShowOperation();
    };

    /** Encapsulates the active configuration for the language server. */
    LSPConfiguration config;
    /** The LSP preprocessor standardizes incoming messages and combines edits. */
    LSPPreprocessor preprocessor;
    /** Trees that have been indexed (with initialGS) and can be reused between different runs */
    std::vector<ast::ParsedFile> indexed;
    /** Trees that have been indexed (with finalGS) and can be reused between different runs */
    UnorderedMap<int, ast::ParsedFile> indexedFinalGS;
    /** Hashes of global states obtained by resolving every file in isolation. Used for fastpath. */
    std::vector<core::FileHash> globalStateHashes;
    /** List of files that have had errors in last run*/
    std::vector<core::FileRef> filesThatHaveErrors;

    /** Concrete error queue shared by all global states */
    std::shared_ptr<core::ErrorQueue> errorQueue;
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.
    std::shared_ptr<spdlog::logger> logger;
    WorkerPool &workers;
    /** Input file descriptor; used by runLSP to receive LSP messages */
    int inputFd = 0;
    /** Output stream; used by LSP to output messages */
    std::ostream &outputStream;
    /**
     * The time that LSP last sent metrics to statsd -- if `opts.statsdHost` was specified.
     */
    std::chrono::time_point<std::chrono::steady_clock> lastMetricUpdateTime;
    /** ID of the main thread, which actually processes LSP requests and performs typechecking. */
    std::thread::id mainThreadId;

    /* Send the given message to client */
    void sendMessage(const LSPMessage &msg) const;

    void addLocIfExists(const core::GlobalState &gs, std::vector<std::unique_ptr<Location>> &locs, core::Loc loc) const;
    std::vector<std::unique_ptr<Location>>
    extractLocations(const core::GlobalState &gs,
                     const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                     std::vector<std::unique_ptr<Location>> locations = {}) const;

    struct TypecheckRun {
        std::vector<std::unique_ptr<core::Error>> errors;
        std::vector<core::FileRef> filesTypechecked;
        // The global state, post-typechecking.
        std::unique_ptr<core::GlobalState> gs;
        // The edit applied to `gs`.
        LSPFileUpdates updates;
        bool tookFastPath = false;
    };
    struct QueryRun {
        std::unique_ptr<core::GlobalState> gs;
        std::vector<std::unique_ptr<core::lsp::QueryResponse>> responses;
        // (Optional) Error that occurred during the query that you can pass on to the client.
        std::unique_ptr<ResponseError> error = nullptr;
    };

    /** Conservatively rerun entire pipeline without caching any trees */
    TypecheckRun runSlowPath(LSPFileUpdates updates) const;
    /** Runs typechecking on the provided updates. */
    TypecheckRun runTypechecking(std::unique_ptr<core::GlobalState> gs, LSPFileUpdates updates) const;
    /** Runs the provided query against the given files, and returns matches. */
    QueryRun runQuery(std::unique_ptr<core::GlobalState> gs, const core::lsp::Query &q,
                      const std::vector<core::FileRef> &filesForQuery) const;
    /** Officially 'commits' the output of a `TypecheckRun` by updating the relevant state on LSPLoop and, if specified,
     * sending diagnostics to the editor. */
    LSPResult commitTypecheckRun(TypecheckRun run);
    LSPResult pushDiagnostics(TypecheckRun run);

    bool ensureInitialized(const LSPMethod forMethod, const LSPMessage &msg) const;

    LSPLoop::QueryRun setupLSPQueryByLoc(std::unique_ptr<core::GlobalState> gs, std::string_view uri,
                                         const Position &pos, const LSPMethod forMethod,
                                         bool errorIfFileIsUntyped = true) const;
    QueryRun setupLSPQueryBySymbol(std::unique_ptr<core::GlobalState> gs, core::SymbolRef symbol) const;
    LSPResult handleTextDocumentHover(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                      const TextDocumentPositionParams &params) const;
    LSPResult handleTextDocumentDocumentSymbol(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                               const DocumentSymbolParams &params) const;
    LSPResult handleWorkspaceSymbols(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                     const WorkspaceSymbolParams &params) const;
    std::pair<std::unique_ptr<core::GlobalState>, std::vector<std::unique_ptr<Location>>>
    getReferencesToSymbol(std::unique_ptr<core::GlobalState> gs, core::SymbolRef symbol,
                          std::vector<std::unique_ptr<Location>> locations = {}) const;
    LSPResult handleTextDocumentReferences(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const ReferenceParams &params) const;
    LSPResult handleTextDocumentDefinition(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const TextDocumentPositionParams &params) const;
    LSPResult handleTextDocumentTypeDefinition(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                               const TextDocumentPositionParams &params) const;
    LSPResult handleTextDocumentCompletion(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const CompletionParams &params) const;
    LSPResult handleTextDocumentCodeAction(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const CodeActionParams &params) const;
    std::unique_ptr<CompletionItem> getCompletionItem(const core::GlobalState &gs, core::SymbolRef what,
                                                      core::TypePtr receiverType,
                                                      const std::unique_ptr<core::TypeConstraint> &constraint) const;
    void findSimilarConstantOrIdent(const core::GlobalState &gs, const core::TypePtr receiverType,
                                    std::vector<std::unique_ptr<CompletionItem>> &items) const;
    void sendShowMessageNotification(MessageType messageType, std::string_view message) const;
    LSPResult handleTextSignatureHelp(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                      const TextDocumentPositionParams &params) const;

    LSPResult processRequestInternal(std::unique_ptr<core::GlobalState> gs, const LSPMessage &msg);

    TypecheckRun handleSorbetWorkspaceEdits(std::unique_ptr<core::GlobalState> gs, LSPFileUpdates &updates) const;

    /** Returns `true` if 5 minutes have elapsed since LSP last sent counters to statsd. */
    bool shouldSendCountersToStatsd(std::chrono::time_point<std::chrono::steady_clock> currentTime) const;
    /** Sends counters to statsd. */
    void sendCountersToStatsd(std::chrono::time_point<std::chrono::steady_clock> currentTime);

public:
    LSPLoop(std::unique_ptr<core::GlobalState> initialGS, LSPConfiguration config,
            const std::shared_ptr<spd::logger> &logger, WorkerPool &workers, int inputFd, std::ostream &output);
    /**
     * Runs the language server on a dedicated thread. Returns the final global state if it exits cleanly, or nullopt
     * on error.
     */
    std::optional<std::unique_ptr<core::GlobalState>> runLSP();
    LSPResult processRequest(std::unique_ptr<core::GlobalState> gs, std::unique_ptr<LSPMessage> msg);
    LSPResult processRequest(std::unique_ptr<core::GlobalState> gs, const std::string &json);
    /**
     * Processes a batch of requests. Performs pre-processing to avoid unnecessary work.
     */
    LSPResult processRequests(std::unique_ptr<core::GlobalState> gs, std::vector<std::unique_ptr<LSPMessage>> messages);
};

std::optional<std::string> findDocumentation(std::string_view sourceCode, int beginIndex);
bool hasSimilarName(const core::GlobalState &gs, core::NameRef name, std::string_view pattern);
bool hideSymbol(const core::GlobalState &gs, core::SymbolRef sym);
std::string methodDetail(const core::GlobalState &gs, core::SymbolRef method, core::TypePtr receiver,
                         core::TypePtr retType, const std::unique_ptr<core::TypeConstraint> &constraint);
std::string methodDefinition(const core::GlobalState &gs, core::SymbolRef method, core::TypePtr receiver,
                             core::TypePtr retType, const std::unique_ptr<core::TypeConstraint> &constraint);
core::TypePtr getResultType(const core::GlobalState &gs, core::TypePtr type, core::SymbolRef inWhat,
                            core::TypePtr receiver, const std::unique_ptr<core::TypeConstraint> &constr);
SymbolKind symbolRef2SymbolKind(const core::GlobalState &gs, core::SymbolRef);

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSPLOOP_H
