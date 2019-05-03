#ifndef RUBY_TYPER_LSPLOOP_H
#define RUBY_TYPER_LSPLOOP_H

#include "main/lsp/LSPMessage.h"
#include "main/lsp/lsp_state.h"
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

class LSPLoop {
    friend class LSPWrapper;

    /** Used to store the state of LSPLoop's internal request queue.  */
    struct QueueState {
        std::deque<std::unique_ptr<LSPMessage>> pendingRequests;
        bool terminate;
        bool paused;
        int requestCounter;
        int errorCode;
        // Counters collected from worker threads.
        CounterState counters;
    };

    /**
     * Object that uses the RAII pattern to notify the client when a *slow* operation
     * starts and ends. Is used to provide user feedback in the status line of VS Code.
     */
    class ShowOperation final {
    private:
        LSPLoop &loop;
        const std::string operationName;
        const std::string description;

    public:
        ShowOperation(LSPLoop &loop, std::string_view operationName, std::string_view description);
        ~ShowOperation();
    };

    /** List of files that have had errors in last run*/
    std::vector<core::FileRef> filesThatHaveErrors;
    /** Root of LSP client workspace */
    std::string rootUri;
    /** File system root of LSP client workspace. May be empty if it is the current working directory. */
    std::string rootPath;

    const options::Options &opts;
    std::shared_ptr<spdlog::logger> logger;
    /**
     * Whether or not the active client has support for snippets in CompletionItems.
     * Note: There is a generated ClientCapabilities class, but it is cumbersome to work with as most fields are
     * optional.
     */
    bool clientCompletionItemSnippetSupport = false;
    /** Input file descriptor; used by runLSP to receive LSP messages */
    int inputFd;
    /** Output stream; used by LSP to output messages */
    std::ostream &outputStream;
    /**
     * Set to true once the server is initialized.
     * TODO(jvilk): Use to raise server not initialized errors.
     */
    bool initialized = false;
    /**
     * If true, then LSP will send the client notifications at the start and end of slow operations.
     * We don't want to send these notifications to clients that don't know what to do with them,
     * so this boolean gets set when the client sends the `initialize` request with
     * `params.initializationOptions.supportsOperationNotifications` set to `true`.
     */
    bool enableOperationNotifications = false;
    /**
     * The time that LSP last sent metrics to statsd -- if `opts.statsdHost` was specified.
     */
    std::chrono::time_point<std::chrono::steady_clock> lastMetricUpdateTime;
    /** ID of the main thread, which actually processes LSP requests and performs typechecking. */
    std::thread::id mainThreadId;
    /** Contains the main LSP state. */
    LSPState state;

    /* Send the given message to client */
    void sendMessage(const LSPMessage &msg);

    std::unique_ptr<Location> loc2Location(const core::GlobalState &gs, core::Loc loc);
    void addLocIfExists(const core::GlobalState &gs, std::vector<std::unique_ptr<Location>> &locs, core::Loc loc);

    LSPResult pushDiagnostics(TypecheckRun filesTypechecked);

    bool ensureInitialized(const LSPMethod forMethod, const LSPMessage &msg,
                           const std::unique_ptr<core::GlobalState> &currentGs);

    core::FileRef uri2FileRef(std::string_view uri) EXCLUSIVE_LOCKS_REQUIRED(state.mtx);
    std::string fileRef2Uri(const core::GlobalState &gs, core::FileRef);
    std::string remoteName2Local(std::string_view uri);
    std::string localName2Remote(std::string_view uri);
    std::unique_ptr<core::Loc> lspPos2Loc(core::FileRef source, const Position &pos, const core::GlobalState &gs);

    /** Used to implement textDocument/documentSymbol
     * Returns `nullptr` if symbol kind is not supported by LSP
     * */
    std::unique_ptr<SymbolInformation> symbolRef2SymbolInformation(const core::GlobalState &gs, core::SymbolRef);
    std::variant<TypecheckRun, std::pair<std::unique_ptr<ResponseError>, std::unique_ptr<core::GlobalState>>>
    setupLSPQueryByLoc(std::unique_ptr<core::GlobalState> gs, std::string_view uri, const Position &pos,
                       const LSPMethod forMethod, bool errorIfFileIsUntyped) LOCKS_EXCLUDED(state.mtx);
    TypecheckRun setupLSPQueryBySymbol(std::unique_ptr<core::GlobalState> gs, core::SymbolRef symbol)
        LOCKS_EXCLUDED(state.mtx);
    LSPResult handleTextDocumentHover(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                      const TextDocumentPositionParams &params);
    LSPResult handleTextDocumentDocumentSymbol(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                               const DocumentSymbolParams &d) LOCKS_EXCLUDED(state.mtx);
    LSPResult handleWorkspaceSymbols(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                     const WorkspaceSymbolParams &params);
    LSPResult handleTextDocumentReferences(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const ReferenceParams &params) LOCKS_EXCLUDED(state.mtx);
    LSPResult handleTextDocumentDefinition(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const TextDocumentPositionParams &params);
    LSPResult handleTextDocumentCompletion(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const CompletionParams &params);
    std::unique_ptr<CompletionItem> getCompletionItem(const core::GlobalState &gs, core::SymbolRef what,
                                                      core::TypePtr receiverType,
                                                      const std::shared_ptr<core::TypeConstraint> &constraint);
    void findSimilarConstantOrIdent(const core::GlobalState &gs, const core::TypePtr receiverType,
                                    std::vector<std::unique_ptr<CompletionItem>> &items);
    void sendShowMessageNotification(MessageType messageType, std::string_view message);
    LSPResult handleTextSignatureHelp(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                      const TextDocumentPositionParams &params) LOCKS_EXCLUDED(state.mtx);
    /**
     * Performs pre-processing on the incoming LSP request and appends it to the queue.
     * Merges changes to the same document + Watchman filesystem updates, and processes pause/ignore requests.
     * If `collectThreadCounters` is `true`, it also merges in thread-local counters into the QueueState counters.
     */
    static void enqueueRequest(const std::shared_ptr<spdlog::logger> &logger, LSPLoop::QueueState &queue,
                               std::unique_ptr<LSPMessage> msg, bool collectThreadCounters = false);

    LSPResult processRequestInternal(std::unique_ptr<core::GlobalState> gs, const LSPMessage &msg)
        LOCKS_EXCLUDED(state.mtx);

    void preprocessSorbetWorkspaceEdit(const DidChangeTextDocumentParams &changeParams,
                                       UnorderedMap<std::string, std::string> &updates)
        EXCLUSIVE_LOCKS_REQUIRED(state.mtx);
    void preprocessSorbetWorkspaceEdit(const DidOpenTextDocumentParams &openParams,
                                       UnorderedMap<std::string, std::string> &updates)
        EXCLUSIVE_LOCKS_REQUIRED(state.mtx);
    void preprocessSorbetWorkspaceEdit(const DidCloseTextDocumentParams &closeParams,
                                       UnorderedMap<std::string, std::string> &updates)
        EXCLUSIVE_LOCKS_REQUIRED(state.mtx);
    void preprocessSorbetWorkspaceEdit(const WatchmanQueryResponse &queryResponse,
                                       UnorderedMap<std::string, std::string> &updates)
        EXCLUSIVE_LOCKS_REQUIRED(state.mtx);
    void preprocessSorbetWorkspaceEdits(const std::vector<std::unique_ptr<SorbetWorkspaceEdit>> &edits,
                                        UnorderedMap<std::string, std::string> &updates)
        EXCLUSIVE_LOCKS_REQUIRED(state.mtx);
    LSPResult handleSorbetWorkspaceEdit(std::unique_ptr<core::GlobalState> gs,
                                        const DidChangeTextDocumentParams &changeParams) LOCKS_EXCLUDED(state.mtx);
    LSPResult handleSorbetWorkspaceEdit(std::unique_ptr<core::GlobalState> gs,
                                        const DidOpenTextDocumentParams &openParams) LOCKS_EXCLUDED(state.mtx);
    LSPResult handleSorbetWorkspaceEdit(std::unique_ptr<core::GlobalState> gs,
                                        const DidCloseTextDocumentParams &closeParams) LOCKS_EXCLUDED(state.mtx);
    LSPResult handleSorbetWorkspaceEdit(std::unique_ptr<core::GlobalState> gs,
                                        const WatchmanQueryResponse &queryResponse) LOCKS_EXCLUDED(state.mtx);
    LSPResult handleSorbetWorkspaceEdits(std::unique_ptr<core::GlobalState> gs,
                                         const std::vector<std::unique_ptr<SorbetWorkspaceEdit>> &edits)
        LOCKS_EXCLUDED(state.mtx);
    LSPResult commitSorbetWorkspaceEdits(std::unique_ptr<core::GlobalState> gs,
                                         UnorderedMap<std::string, std::string> &updates)
        EXCLUSIVE_LOCKS_REQUIRED(state.mtx);

    /** Returns `true` if 5 minutes have elapsed since LSP last sent counters to statsd. */
    bool shouldSendCountersToStatsd(std::chrono::time_point<std::chrono::steady_clock> currentTime);
    /** Sends counters to statsd. */
    void sendCountersToStatsd(std::chrono::time_point<std::chrono::steady_clock> currentTime);

public:
    LSPLoop(std::unique_ptr<core::GlobalState> gs, const options::Options &opts,
            const std::shared_ptr<spdlog::logger> &logger, WorkerPool &workers, int inputFd, std::ostream &output,
            bool skipConfigatron = false, bool disableFastPath = false);
    std::unique_ptr<core::GlobalState> runLSP();
    LSPResult processRequest(std::unique_ptr<core::GlobalState> gs, const LSPMessage &msg);
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
                         core::TypePtr retType, std::shared_ptr<core::TypeConstraint> constraint);
core::TypePtr getResultType(const core::GlobalState &gs, core::SymbolRef ofWhat, core::TypePtr receiver,
                            std::shared_ptr<core::TypeConstraint> constr);
SymbolKind symbolRef2SymbolKind(const core::GlobalState &gs, core::SymbolRef);
std::unique_ptr<Range> loc2Range(const core::GlobalState &gs, core::Loc loc);

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSPLOOP_H
