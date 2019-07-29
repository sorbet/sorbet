#ifndef RUBY_TYPER_LSPLOOP_H
#define RUBY_TYPER_LSPLOOP_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/ErrorQueue.h"
#include "core/NameHash.h"
#include "core/core.h"
#include "main/lsp/LSPMessage.h"
#include "main/options/options.h"
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

    /** Trees that have been indexed (with initialGS) and can be reused between different runs */
    std::vector<ast::ParsedFile> indexed;
    /** Trees that have been indexed (with finalGS) and can be reused between different runs */
    UnorderedMap<int, ast::ParsedFile> indexedFinalGS;
    /** Hashes of global states obtained by resolving every file in isolation. Used for fastpath. */
    std::vector<core::FileHash> globalStateHashes;
    /** List of files that have had errors in last run*/
    std::vector<core::FileRef> filesThatHaveErrors;
    /** Root of LSP client workspace */
    std::string rootUri;
    /** File system root of LSP client workspace. May be empty if it is the current working directory. */
    std::string rootPath;

    /** Concrete error queue shared by all global states */
    std::shared_ptr<core::ErrorQueue> errorQueue;
    /**
     * `initialGS` is used for indexing. It accumulates a huge nametable of all global things,
     * and is updated as global things are added/removed/updated. It is never discarded.
     *
     * Typechecking is never run on `initialGS` directly. Instead, LSPLoop clones `initialGS` and runs type checking on
     * the clone. This clone is what LSPLoop returns within a `TypecheckRun`.
     */
    std::unique_ptr<core::GlobalState> initialGS;
    const options::Options &opts;
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.
    std::shared_ptr<spdlog::logger> logger;
    WorkerPool &workers;
    /**
     * Whether or not the active client has support for snippets in CompletionItems.
     * Note: There is a generated ClientCapabilities class, but it is cumbersome to work with as most fields are
     * optional.
     */
    bool clientCompletionItemSnippetSupport = false;
    /** What hover markup should we send to the client? */
    MarkupKind clientHoverMarkupKind = MarkupKind::Plaintext;
    /** Input file descriptor; used by runLSP to receive LSP messages */
    int inputFd;
    /** Output stream; used by LSP to output messages */
    std::ostream &outputStream;
    /** If true, LSPLoop will skip configatron during type checking */
    const bool skipConfigatron;
    /** If true, all queries will hit the slow path. */
    const bool disableFastPath;
    /** The set of files currently open in the user's editor. */
    UnorderedSet<std::string> openFiles;
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
    /** If true, then LSP sends metadata to the client every time it typechecks files. Used in tests. */
    bool enableTypecheckInfo = false;
    /**
     * The time that LSP last sent metrics to statsd -- if `opts.statsdHost` was specified.
     */
    std::chrono::time_point<std::chrono::steady_clock> lastMetricUpdateTime;
    /** ID of the main thread, which actually processes LSP requests and performs typechecking. */
    std::thread::id mainThreadId;

    /* Send the given message to client */
    void sendMessage(const LSPMessage &msg);

    std::unique_ptr<Location> loc2Location(const core::GlobalState &gs, core::Loc loc);
    void addLocIfExists(const core::GlobalState &gs, std::vector<std::unique_ptr<Location>> &locs, core::Loc loc);
    std::vector<std::unique_ptr<Location>>
    extractLocations(const core::GlobalState &gs,
                     const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                     std::vector<std::unique_ptr<Location>> locations = {});

    core::FileRef updateFile(const std::shared_ptr<core::File> &file);
    /** Invalidate all currently cached trees and re-index them from file system.
     * This runs code that is not considered performance critical and this is expected to be slow */
    void reIndexFromFileSystem();
    struct TypecheckRun {
        std::vector<std::unique_ptr<core::Error>> errors;
        std::vector<core::FileRef> filesTypechecked;
        std::vector<std::unique_ptr<core::lsp::QueryResponse>> responses;
        // The global state, post-typechecking.
        std::unique_ptr<core::GlobalState> gs;
        bool tookFastPath;
    };
    /** Conservatively rerun entire pipeline without caching any trees */
    TypecheckRun runSlowPath();
    /** Returns `true` if the given changes can run on the fast path. */
    bool canTakeFastPath(const std::vector<std::shared_ptr<core::File>> &changedFiles,
                         const std::vector<core::FileHash> &hashes) const;
    /** Apply conservative heuristics to see if we can run a fast path, if not, bail out and run slowPath */
    TypecheckRun tryFastPath(std::unique_ptr<core::GlobalState> gs,
                             const std::vector<std::shared_ptr<core::File>> &changedFiles,
                             const std::vector<core::FileRef> &filesForQuery = {});

    LSPResult pushDiagnostics(TypecheckRun run);

    std::vector<core::FileHash> computeStateHashes(const std::vector<std::shared_ptr<core::File>> &files);
    bool ensureInitialized(const LSPMethod forMethod, const LSPMessage &msg,
                           const std::unique_ptr<core::GlobalState> &currentGs);

    core::FileRef uri2FileRef(std::string_view uri);
    std::string fileRef2Uri(const core::GlobalState &gs, core::FileRef);
    std::string remoteName2Local(std::string_view uri);
    std::string localName2Remote(std::string_view uri);
    std::unique_ptr<core::Loc> lspPos2Loc(core::FileRef fref, const Position &pos, const core::GlobalState &gs);

    /** Used to implement textDocument/documentSymbol
     * Returns `nullptr` if symbol kind is not supported by LSP
     * */
    std::unique_ptr<SymbolInformation> symbolRef2SymbolInformation(const core::GlobalState &gs, core::SymbolRef);
    TypecheckRun runLSPQuery(std::unique_ptr<core::GlobalState> gs, const core::lsp::Query &q,
                             const std::vector<core::FileRef> &filesToQuery);
    std::variant<LSPLoop::TypecheckRun, std::pair<std::unique_ptr<ResponseError>, std::unique_ptr<core::GlobalState>>>
    setupLSPQueryByLoc(std::unique_ptr<core::GlobalState> gs, std::string_view uri, const Position &pos,
                       const LSPMethod forMethod, bool errorIfFileIsUntyped = true);
    TypecheckRun setupLSPQueryBySymbol(std::unique_ptr<core::GlobalState> gs, core::SymbolRef symbol);
    LSPResult handleTextDocumentHover(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                      const TextDocumentPositionParams &params);
    LSPResult handleTextDocumentDocumentSymbol(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                               const DocumentSymbolParams &params);
    LSPResult handleWorkspaceSymbols(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                     const WorkspaceSymbolParams &params);
    std::pair<std::unique_ptr<core::GlobalState>, std::vector<std::unique_ptr<Location>>>
    getReferencesToSymbol(std::unique_ptr<core::GlobalState> gs, core::SymbolRef symbol,
                          std::vector<std::unique_ptr<Location>> locations = {});
    LSPResult handleTextDocumentReferences(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const ReferenceParams &params);
    LSPResult handleTextDocumentDefinition(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const TextDocumentPositionParams &params);
    LSPResult handleTextDocumentCompletion(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const CompletionParams &params);
    LSPResult handleTextDocumentCodeAction(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                           const TextDocumentCodeActionParams &params);
    std::unique_ptr<CompletionItem> getCompletionItem(const core::GlobalState &gs, core::SymbolRef what,
                                                      core::TypePtr receiverType,
                                                      const std::unique_ptr<core::TypeConstraint> &constraint);
    void findSimilarConstantOrIdent(const core::GlobalState &gs, const core::TypePtr receiverType,
                                    std::vector<std::unique_ptr<CompletionItem>> &items);
    void sendShowMessageNotification(MessageType messageType, std::string_view message);
    LSPResult handleTextSignatureHelp(std::unique_ptr<core::GlobalState> gs, const MessageId &id,
                                      const TextDocumentPositionParams &params);
    /**
     * Performs pre-processing on the incoming LSP request and appends it to the queue.
     * Merges changes to the same document + Watchman filesystem updates, and processes pause/ignore requests.
     * If `collectThreadCounters` is `true`, it also merges in thread-local counters into the QueueState counters.
     */
    static void enqueueRequest(const std::shared_ptr<spd::logger> &logger, LSPLoop::QueueState &state,
                               std::unique_ptr<LSPMessage> msg, bool collectThreadCounters = false);

    LSPResult processRequestInternal(std::unique_ptr<core::GlobalState> gs, const LSPMessage &msg);

    void preprocessSorbetWorkspaceEdit(const DidChangeTextDocumentParams &changeParams,
                                       UnorderedMap<std::string, std::string> &updates);
    void preprocessSorbetWorkspaceEdit(const DidOpenTextDocumentParams &openParams,
                                       UnorderedMap<std::string, std::string> &updates);
    void preprocessSorbetWorkspaceEdit(const DidCloseTextDocumentParams &closeParams,
                                       UnorderedMap<std::string, std::string> &updates);
    void preprocessSorbetWorkspaceEdit(const WatchmanQueryResponse &queryResponse,
                                       UnorderedMap<std::string, std::string> &updates);
    LSPResult handleSorbetWorkspaceEdit(std::unique_ptr<core::GlobalState> gs,
                                        const DidChangeTextDocumentParams &changeParams);
    LSPResult handleSorbetWorkspaceEdit(std::unique_ptr<core::GlobalState> gs,
                                        const DidOpenTextDocumentParams &openParams);
    LSPResult handleSorbetWorkspaceEdit(std::unique_ptr<core::GlobalState> gs,
                                        const DidCloseTextDocumentParams &closeParams);
    LSPResult handleSorbetWorkspaceEdit(std::unique_ptr<core::GlobalState> gs,
                                        const WatchmanQueryResponse &queryResponse);
    LSPResult handleSorbetWorkspaceEdits(std::unique_ptr<core::GlobalState> gs,
                                         std::vector<std::unique_ptr<SorbetWorkspaceEdit>> &edits);
    LSPResult commitSorbetWorkspaceEdits(std::unique_ptr<core::GlobalState> gs,
                                         UnorderedMap<std::string, std::string> &updates);

    /** Returns `true` if 5 minutes have elapsed since LSP last sent counters to statsd. */
    bool shouldSendCountersToStatsd(std::chrono::time_point<std::chrono::steady_clock> currentTime);
    /** Sends counters to statsd. */
    void sendCountersToStatsd(std::chrono::time_point<std::chrono::steady_clock> currentTime);

public:
    LSPLoop(std::unique_ptr<core::GlobalState> gs, const options::Options &opts,
            const std::shared_ptr<spd::logger> &logger, WorkerPool &workers, int inputFd, std::ostream &output,
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
                         core::TypePtr retType, const std::unique_ptr<core::TypeConstraint> &constraint);
core::TypePtr getResultType(const core::GlobalState &gs, core::TypePtr type, core::SymbolRef inWhat,
                            core::TypePtr receiver, const std::unique_ptr<core::TypeConstraint> &constr);
SymbolKind symbolRef2SymbolKind(const core::GlobalState &gs, core::SymbolRef);
std::unique_ptr<Range> loc2Range(const core::GlobalState &gs, core::Loc loc);
bool cmpRanges(const Range &a, const Range &b);

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSPLOOP_H
