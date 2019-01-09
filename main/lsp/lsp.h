#ifndef RUBY_TYPER_LSPLOOP_H
#define RUBY_TYPER_LSPLOOP_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/ErrorQueue.h"
#include "core/core.h"
#include "main/lsp/json_types.h"
#include "main/options/options.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <deque>
#include <optional>

//  _     ____  ____
// | |   / ___||  _ _\
// | |   \___ \| |_) |
// | |___ ___) |  __/
// |_____|____/|_|
//
//
// This is an implementation of LSP protocol(version 3.0) for Ruby typer
// So far we only support errors but the intention is to
// continue adding support to features already in LSP:
// - code navigation(jump to definition, find all usages, etc)
// - refactorings(rename classes)
//
// So far, we only handle changes via "textDocument/didChange" request.
// This is the main request that is used by VSCode.
// VI uses "textDocument/didSave" that is very similar and should be easy to support.
namespace sorbet::realmain::lsp {

/** This structure represents a method defined by LSP.
 * It is used as an enum to indicate properties of method in common request handling code.
 */
struct LSPMethod {
    /* What is the name of this method as specified in the protocol */
    const std::string name;
    /* Is this a notification? Otherwise this is a request and it would need a response */
    const bool isNotification;
    /* Who initiates this request */
    enum class Kind {
        ServerInitiated,
        ClientInitiated,
        Both,
    };
    const Kind kind;
    /* Do we support this method? */
    const bool isSupported = true;

    inline bool operator==(const LSPMethod &other) const {
        return other.name == this->name;
    }
    inline bool operator!=(const LSPMethod &other) const {
        return other.name != this->name;
    }
    static const inline LSPMethod CancelRequest() {
        return LSPMethod{"$/cancelRequest", true, LSPMethod::Kind::Both};
    };
    static const inline LSPMethod Initialize() {
        return LSPMethod{"initialize", false, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod Initialized() {
        return LSPMethod{"initialized", true, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod Shutdown() {
        return LSPMethod{"shutdown", false, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod Exit() {
        return LSPMethod{"exit", true, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod RegisterCapability() {
        return LSPMethod{"client/registerCapability", false, LSPMethod::Kind::ServerInitiated};
    };
    static const inline LSPMethod UnRegisterCapability() {
        return LSPMethod{"client/unregisterCapability", false, LSPMethod::Kind::ServerInitiated};
    };
    static const inline LSPMethod DidChangeWatchedFiles() {
        return LSPMethod{"workspace/didChangeWatchedFiles", true, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod PushDiagnostics() {
        return LSPMethod{"textDocument/publishDiagnostics", true, LSPMethod::Kind::ServerInitiated};
    };
    static const inline LSPMethod TextDocumentDidOpen() {
        return LSPMethod{"textDocument/didOpen", true, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod TextDocumentDidChange() {
        return LSPMethod{"textDocument/didChange", true, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod TextDocumentDocumentSymbol() {
        return LSPMethod{"textDocument/documentSymbol", false, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod TextDocumentDefinition() {
        return LSPMethod{"textDocument/definition", false, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod TextDocumentHover() {
        return LSPMethod{"textDocument/hover", false, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod TextDocumentCompletion() {
        return LSPMethod{"textDocument/completion", false, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod TextDocumentRefernces() {
        return LSPMethod{"textDocument/references", false, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod TextDocumentSignatureHelp() {
        return LSPMethod{"textDocument/signatureHelp", false, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod WorkspaceSymbols() {
        return LSPMethod{"workspace/symbol", false, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod WindowShowMessage() {
        return LSPMethod{"window/showMessage", true, LSPMethod::Kind::ServerInitiated};
    };
    static const inline LSPMethod Pause() {
        return LSPMethod{"__PAUSE__", true, LSPMethod::Kind::ClientInitiated};
    };
    static const inline LSPMethod Resume() {
        return LSPMethod{"__RESUME__", true, LSPMethod::Kind::ClientInitiated};
    };
    static const std::vector<LSPMethod> ALL_METHODS;
    static const LSPMethod getByName(std::string_view name);
};

/** List of all LSP Methods that we are aware of */

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

/*
 * The LSP*Capabilities structs are transcriptions from the specification, but
 * for now we're only encoding the fields we actually look at.
 */
struct LSPTextDocumentClientCapabilities {
    struct {
        struct {
            bool snippetSupport = false;
        } completionItem;
    } completion;
};

struct LSPClientCapabilities {
    LSPTextDocumentClientCapabilities textDocument;
};

class LSPLoop {
    int requestCounter = 1;
    struct ResponseHandler {
        std::function<void(rapidjson::Value &)> onResult;
        std::function<void(rapidjson::Value &)> onError;
    };
    UnorderedMap<std::string, ResponseHandler> awaitingResponse;
    /** LSP loop reuses a single arena for all json allocations. We never free memory used for JSON */
    rapidjson::MemoryPoolAllocator<> alloc;
    /** Trees that have been indexed and can be reused between different runs */
    std::vector<ast::ParsedFile> indexed;
    /** Hashes of global states obtained by resolving every file in isolation. Used for fastpath. */
    std::vector<unsigned int> globalStateHashes;
    /** List of files that have had errors in last run*/
    std::vector<core::FileRef> filesThatHaveErrors;
    /** Root of LSP client workspace */
    std::string rootUri;

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
    std::shared_ptr<spdlog::logger> &logger;
    WorkerPool &workers;
    LSPClientCapabilities clientCapabilities;
    /** Input stream; used by runLSP to receive LSP messages */
    std::istream &inputStream;
    /** Output stream; used by LSP to output messages */
    std::ostream &outputStream;
    /** If true, LSPLoop will typecheck test files */
    const bool typecheckTestFiles;
    /** If true, LSPLoop will skip configatron during type checking */
    const bool skipConfigatron;
    /** If true, all queries will hit the slow path. */
    const bool disableFastPath;

    /* Send the following document to client */
    void sendRaw(rapidjson::Document &raw);

    /* Send `data` as payload of notification 'meth' */
    void sendNotification(LSPMethod meth, rapidjson::Document &d, const JSONBaseType &data);
    /* Send `data` as payload of request `meth`. Execute `onComplete` when Client replies, execute
     * `onFail` if client reports failure.
     * TODO(dmitry): misbehaving clients may drop requests on floor without reporting either success or
     * error. We will currently leak lambdas&their captures in such case.
     */
    void sendRequest(LSPMethod meth, rapidjson::Value &data, std::function<void(rapidjson::Value &)> onComplete,
                     std::function<void(rapidjson::Value &)> onFail);

    void sendResult(rapidjson::Document &forRequest, rapidjson::Value &result);
    void sendResult(rapidjson::Document &forRequest, const JSONBaseType &result);
    void sendResult(rapidjson::Document &forRequest, const std::vector<std::unique_ptr<JSONBaseType>> &result);
    void sendError(rapidjson::Document &forRequest, int errorCode, const std::string &errorStr);

    std::unique_ptr<Location> loc2Location(const core::GlobalState &gs, core::Loc loc);
    void addLocIfExists(const core::GlobalState &gs, std::vector<std::unique_ptr<JSONBaseType>> &locs, core::Loc loc);

    /** Returns true if there is no need to continue processing this document as it is a reply to
     * already registered request*/
    bool handleReplies(rapidjson::Document &d);

    core::FileRef addNewFile(const std::shared_ptr<core::File> &file);
    /** Invalidate all currently cached trees and re-index them from file system.
     * This runs code that is not considered performance critical and this is expected to be slow */
    void reIndexFromFileSystem();
    struct TypecheckRun {
        std::vector<std::unique_ptr<core::Error>> errors;
        std::vector<core::FileRef> filesTypechecked;
        std::vector<std::unique_ptr<core::QueryResponse>> responses;
        // The global state, post-typechecking.
        std::unique_ptr<core::GlobalState> gs;
    };
    /** Conservatively rerun entire pipeline without caching any trees */
    TypecheckRun runSlowPath(const std::vector<std::shared_ptr<core::File>> &changedFiles);
    /** Apply conservative heuristics to see if we can run a fast path, if not, bail out and run slowPath */
    TypecheckRun tryFastPath(std::unique_ptr<core::GlobalState> gs,
                             std::vector<std::shared_ptr<core::File>> &changedFiles, bool allFiles = false);

    std::unique_ptr<core::GlobalState> pushDiagnostics(rapidjson::Document &d, TypecheckRun filesTypechecked);

    std::vector<unsigned int> computeStateHashes(const std::vector<std::shared_ptr<core::File>> &files);
    bool ensureInitialized(LSPMethod forMethod, rapidjson::Document &d,
                           const std::unique_ptr<core::GlobalState> &currentGs);

    core::FileRef uri2FileRef(std::string_view uri);
    std::string fileRef2Uri(const core::GlobalState &gs, core::FileRef);
    std::string remoteName2Local(std::string_view uri);
    std::string localName2Remote(std::string_view uri);
    std::unique_ptr<core::Loc> lspPos2Loc(core::FileRef source, rapidjson::Document &d, const core::GlobalState &gs);

    /** Used to implement textDocument/documentSymbol
     * Returns `nullptr` if symbol kind is not supported by LSP
     * */
    std::unique_ptr<SymbolInformation> symbolRef2SymbolInformation(const core::GlobalState &gs, core::SymbolRef);
    TypecheckRun runLSPQuery(std::unique_ptr<core::GlobalState> gs, core::Loc loc, core::SymbolRef symbol,
                             std::vector<std::shared_ptr<core::File>> &changedFiles, bool allFiles = false);
    TypecheckRun setupLSPQueryByLoc(std::unique_ptr<core::GlobalState> gs, rapidjson::Document &d,
                                    const LSPMethod &forMethod, bool errorIfFileIsUntyped);
    TypecheckRun setupLSPQueryBySymbol(std::unique_ptr<core::GlobalState> gs, core::SymbolRef symbol,
                                       const LSPMethod &forMethod);
    std::unique_ptr<core::GlobalState> handleTextDocumentHover(std::unique_ptr<core::GlobalState> gs,
                                                               rapidjson::Document &d);
    std::unique_ptr<core::GlobalState> handleTextDocumentDocumentSymbol(std::unique_ptr<core::GlobalState> gs,
                                                                        rapidjson::Document &d);
    std::unique_ptr<core::GlobalState> handleWorkspaceSymbols(std::unique_ptr<core::GlobalState> gs,
                                                              rapidjson::Document &d);
    std::unique_ptr<core::GlobalState> handleTextDocumentReferences(std::unique_ptr<core::GlobalState> gs,
                                                                    rapidjson::Document &d);
    std::unique_ptr<core::GlobalState> handleTextDocumentDefinition(std::unique_ptr<core::GlobalState> gs,
                                                                    rapidjson::Document &d);
    std::unique_ptr<core::GlobalState> handleTextDocumentCompletion(std::unique_ptr<core::GlobalState> gs,
                                                                    rapidjson::Document &d);
    std::unique_ptr<CompletionItem> getCompletionItem(const core::GlobalState &gs, core::SymbolRef what,
                                                      const core::QueryResponse &resp);
    void sendShowMessageNotification(MessageType messageType, rapidjson::Document &d, const std::string &message);
    bool isTestFile(const std::shared_ptr<core::File> &file);
    std::unique_ptr<core::GlobalState> handleTextSignatureHelp(std::unique_ptr<core::GlobalState> gs,
                                                               rapidjson::Document &d);
    static void mergeDidChanges(std::deque<rapidjson::Document> &pendingRequests);
    static std::deque<rapidjson::Document>::iterator
    findRequestToBeCancelled(std::deque<rapidjson::Document> &pendingRequests,
                             rapidjson::Document &cancellationRequest);
    static std::deque<rapidjson::Document>::iterator
    findFirstPositionAfterLSPInitialization(std::deque<rapidjson::Document> &pendingRequests);

public:
    LSPLoop(std::unique_ptr<core::GlobalState> gs, const options::Options &opts, std::shared_ptr<spd::logger> &logger,
            WorkerPool &workers, std::istream &input, std::ostream &output, bool typecheckTestFiles = false,
            bool skipConfigatron = false, bool disableFastPath = false);
    std::unique_ptr<core::GlobalState> runLSP();
    std::unique_ptr<core::GlobalState> processRequest(std::unique_ptr<core::GlobalState> gs, rapidjson::Document &d);
    std::unique_ptr<core::GlobalState> processRequest(std::unique_ptr<core::GlobalState> gs, const std::string &json);
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
