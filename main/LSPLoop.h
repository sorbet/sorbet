#ifndef RUBY_TYPER_LSPLOOP_H
#define RUBY_TYPER_LSPLOOP_H

#include "ast/ast.h"
#include "core/Files.h"
#include "realmain.h"
#define RAPIDJSON_ASSERT(x) ENFORCE(x)
#define RAPIDJSON_HAS_STDSTRING 1
#include "common/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
namespace sorbet {
namespace realmain {

//  _     ____  ____
// | |   / ___||  _ \-
// | |   \___ \| |_) |
// | |___ ___) |  __/
// |_____|____/|_|
//
//
// This is an implementation of LSP protocol(version 3.0) for Ruby typer
// So far we only support diagnostics but the intention is to
// continue adding support to features already in LSP:
// - code navigation(jump to definition, find all usages, etc)
// - refactorings(rename classes)
//
// So far, we only handle changes via "textDocument/didChange" request.
// This is the main request that is used by VSCode.
// VI uses "textDocument/didSave" that is very similar and should be easy to support.
namespace LSP {
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
};

const LSPMethod CancelRequest{"$/cancelRequest", true, LSPMethod::Kind::Both};
const LSPMethod Initialize{"initialize", false, LSPMethod::Kind::ClientInitiated};
const LSPMethod Inititalized{"initialized", true, LSPMethod::Kind::ClientInitiated};
const LSPMethod Shutdown{"shutdown", false, LSPMethod::Kind::ClientInitiated};
const LSPMethod Exit{"exit", true, LSPMethod::Kind::ClientInitiated};
const LSPMethod RegisterCapability{"client/registerCapability", false, LSPMethod::Kind::ServerInitiated};
const LSPMethod UnRegisterCapability{"client/unregisterCapability", false, LSPMethod::Kind::ServerInitiated};
const LSPMethod DidChangeWatchedFiles{"workspace/didChangeWatchedFiles", true, LSPMethod::Kind::ClientInitiated};
const LSPMethod PushDiagnostics{"textDocument/publishDiagnostics", true, LSPMethod::Kind::ServerInitiated};
const LSPMethod TextDocumentDidOpen{"textDocument/didOpen", true, LSPMethod::Kind::ClientInitiated};
const LSPMethod TextDocumentDidChange{"textDocument/didChange", true, LSPMethod::Kind::ClientInitiated};
const LSPMethod TextDocumentDocumentSymbol{"textDocument/documentSymbol", false, LSPMethod::Kind::ClientInitiated};
const LSPMethod ReadFile{"ruby-typer/ReadFile", false, LSPMethod::Kind::ServerInitiated};
const LSPMethod WorkspaceSymbolsRequest{"workspace/symbol", false, LSPMethod::Kind::ClientInitiated};

/** List of all LSP Methods that we are aware of */
const LSPMethod ALL[] = {CancelRequest,
                         Initialize,
                         Inititalized,
                         Shutdown,
                         Exit,
                         RegisterCapability,
                         UnRegisterCapability,
                         DidChangeWatchedFiles,
                         PushDiagnostics,
                         TextDocumentDidChange,
                         TextDocumentDocumentSymbol,
                         ReadFile,
                         WorkspaceSymbolsRequest};

const LSPMethod getMethod(const absl::string_view name);

enum class LSPErrorCodes {
    // Defined by JSON RPC
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602, // todo
    InternalError = -32603,
    ServerErrorStart = -32099,
    ServerErrorEnd = -32000,
    ServerNotInitialized = -32002, // todo: can be found by finalGS = nullptr
    UnknownErrorCode = -32001,

    // Defined by the LSP
    RequestCancelled = -32800,
};

} // namespace LSP

class LSPLoop {
    int requestCounter = 1;
    struct ResponseHandler {
        std::function<void(rapidjson::Value &)> onResult;
        std::function<void(rapidjson::Value &)> onError;
    };
    std::map<std::string, ResponseHandler> awaitingResponse;
    /** LSP loop reuses a single arena for all json allocations. We never free memory used for JSON */
    rapidjson::MemoryPoolAllocator<> alloc;
    /** Trees that have been indexed and can be reused between different runs */
    std::vector<std::unique_ptr<const ast::Expression>> indexed;
    /** List of errors that have never yet been sent to LSP client */
    std::vector<core::FileRef> updatedErrors;
    /** LSP clients do not store errors on their side. We have to resend the entire list of errors
     * every time we send a single new one */
    std::unordered_map<core::FileRef, std::vector<std::unique_ptr<core::BasicError>>> errorsAccumulated;
    /** Root of LSP client workspace */
    std::string rootUri;

    /** GlobalState that is used for indexing. It effectively accumulates a huge nametable.
     * It is never discarded. */
    std::unique_ptr<core::GlobalState> initialGS;
    /** A clone of `initialGs` that is used for typechecking.
     * Discarded on every clean run.
     */
    std::unique_ptr<core::GlobalState> finalGs;
    const Options &opts;
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.
    std::shared_ptr<spdlog::logger> &logger;
    WorkerPool &workers;

    /* Send the following document to client */
    void sendRaw(rapidjson::Document &raw);

    /* Send `data` as payload of notification 'meth' */
    void sendNotification(LSP::LSPMethod meth, rapidjson::Value &data);
    /* Send `data` as payload of request `meth`. Execute `onComplete` when Client replies, execute
     * `onFail` if client reports failure.
     * TODO(dmitry): misbehaving clients may drop requests on floor without reporting either success or
     * error. We will currently leak lambdas&their captures in such case.
     */
    void sendRequest(LSP::LSPMethod meth, rapidjson::Value &data, std::function<void(rapidjson::Value &)> onComplete,
                     std::function<void(rapidjson::Value &)> onFail);

    void sendResult(rapidjson::Document &forRequest, rapidjson::Value &result);
    void sendError(rapidjson::Document &forRequest, int errorCode, std::string errorStr);

    void pushErrors();
    void drainErrors();
    rapidjson::Value loc2Range(core::Loc loc);
    rapidjson::Value loc2Location(core::Loc loc);

    /** Returns true if there is no need to continue processing this document as it is a reply to
     * already registered request*/
    bool handleReplies(rapidjson::Document &d);

    void addNewFile(std::shared_ptr<core::File> file);
    /** Invalidate all currently cached trees and re-index them from file system.
     * This runs code that is not considered performance critical and this is expected to be slow */
    void reIndexFromFileSystem();
    /** Conservatively rerun entire pipeline without caching any trees */
    void runSlowPath(std::vector<std::shared_ptr<core::File>> changedFiles);
    /** Apply conservative heuristics to see if we can run a fast path, if not, bail out and run slowPath */
    void tryFastPath(std::vector<std::shared_ptr<core::File>> changedFiles);

    core::FileRef uri2FileRef(const absl::string_view uri);
    std::string fileRef2Uri(core::FileRef);

    std::string remoteName2Local(const absl::string_view uri);
    std::string localName2Remote(const absl::string_view uri);

    /** Used to implement textDocument/documentSymbol
     * Returns `nullptr` if symbol kind is not supported by LSP
     * */
    std::unique_ptr<rapidjson::Value> symbolRef2SymbolInformation(core::SymbolRef);

public:
    LSPLoop(std::unique_ptr<core::GlobalState> gs, const Options &opts, std::shared_ptr<spd::logger> &logger,
            WorkerPool &workers)
        : initialGS(move(gs)), opts(opts), logger(logger), workers(workers) {}
    void runLSP();

    void invalidateAllErrors();
};

} // namespace realmain
} // namespace sorbet
#endif // RUBY_TYPER_LSPLOOP_H
