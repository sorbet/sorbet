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

enum class LSPErrorCodes {
    // Defined by JSON RPC
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602, // todo
    InternalError = -32603,
    ServerErrorStart = -32099,
    ServerErrorEnd = -32000,
    ServerNotInitialized = -32002,
    UnknownErrorCode = -32001,

    // Defined by the LSP
    RequestCancelled = -32800,
};

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

    void addLocIfExists(const core::GlobalState &gs, std::vector<std::unique_ptr<Location>> &locs, core::Loc loc) const;
    std::vector<std::unique_ptr<Location>>
    extractLocations(const core::GlobalState &gs,
                     const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                     std::vector<std::unique_ptr<Location>> locations = {}) const;

    LSPQueryResult queryByLoc(LSPTypecheckerDelegate &typechecker, std::string_view uri, const Position &pos,
                              const LSPMethod forMethod, bool errorIfFileIsUntyped = true) const;
    LSPQueryResult queryBySymbolInFiles(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol,
                                        std::vector<core::FileRef> frefs) const;
    LSPQueryResult queryBySymbol(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol) const;

    std::unique_ptr<ResponseMessage>
    handleTextDocumentDocumentHighlight(LSPTypecheckerDelegate &typechecker, const MessageId &id,
                                        const TextDocumentPositionParams &params) const;
    std::unique_ptr<ResponseMessage> handleTextDocumentHover(LSPTypecheckerDelegate &typechecker, const MessageId &id,
                                                             const TextDocumentPositionParams &params) const;
    std::unique_ptr<ResponseMessage> handleTextDocumentDocumentSymbol(LSPTypecheckerDelegate &typechecker,
                                                                      const MessageId &id,
                                                                      const DocumentSymbolParams &params) const;
    std::unique_ptr<ResponseMessage> handleWorkspaceSymbols(LSPTypecheckerDelegate &typechecker, const MessageId &id,
                                                            const WorkspaceSymbolParams &params) const;
    std::vector<std::unique_ptr<Location>>
    getReferencesToSymbol(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol,
                          std::vector<std::unique_ptr<Location>> locations = {}) const;
    std::vector<std::unique_ptr<DocumentHighlight>>
    getHighlightsToSymbolInFile(LSPTypecheckerDelegate &typechecker, std::string_view uri, core::SymbolRef symbol,
                                std::vector<std::unique_ptr<DocumentHighlight>> highlights = {}) const;
    std::unique_ptr<ResponseMessage> handleTextDocumentReferences(LSPTypecheckerDelegate &typechecker,
                                                                  const MessageId &id,
                                                                  const ReferenceParams &params) const;
    std::unique_ptr<ResponseMessage> handleTextDocumentDefinition(LSPTypecheckerDelegate &typechecker,
                                                                  const MessageId &id,
                                                                  const TextDocumentPositionParams &params) const;
    std::unique_ptr<ResponseMessage> handleTextDocumentTypeDefinition(LSPTypecheckerDelegate &typechecker,
                                                                      const MessageId &id,
                                                                      const TextDocumentPositionParams &params) const;
    std::unique_ptr<ResponseMessage> handleTextDocumentCompletion(LSPTypecheckerDelegate &typechecker,
                                                                  const MessageId &id,
                                                                  const CompletionParams &params) const;
    std::unique_ptr<ResponseMessage> handleTextDocumentCodeAction(LSPTypecheckerDelegate &typechecker,
                                                                  const MessageId &id,
                                                                  const CodeActionParams &params) const;
    std::unique_ptr<CompletionItem> getCompletionItemForMethod(LSPTypecheckerDelegate &typechecker,
                                                               core::SymbolRef what, core::TypePtr receiverType,
                                                               const core::TypeConstraint *constraint,
                                                               const core::Loc queryLoc, std::string_view prefix,
                                                               size_t sortIdx) const;
    void findSimilarConstants(const core::GlobalState &gs, const core::lsp::ConstantResponse &resp,
                              const core::Loc queryLoc, std::vector<std::unique_ptr<CompletionItem>> &items) const;
    std::unique_ptr<ResponseMessage> handleTextSignatureHelp(LSPTypecheckerDelegate &typechecker, const MessageId &id,
                                                             const TextDocumentPositionParams &params) const;

    void processRequestInternal(LSPMessage &msg);

    /** Returns `true` if 5 minutes have elapsed since LSP last sent counters to statsd. */
    bool shouldSendCountersToStatsd(std::chrono::time_point<std::chrono::steady_clock> currentTime) const;
    /** Sends counters to statsd. */
    void sendCountersToStatsd(std::chrono::time_point<std::chrono::steady_clock> currentTime);
    /** Helper method: If message is an edit taking the slow path, and slow path cancelation is enabled, signal to
     * GlobalState that we will be starting a commit of the edits. */
    void maybeStartCommitSlowPathEdit(const LSPMessage &msg) const;

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
SymbolKind symbolRef2SymbolKind(const core::GlobalState &gs, core::SymbolRef);

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSPLOOP_H
