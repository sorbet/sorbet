#ifndef MAIN_LSP_WRAPPER_H
#define MAIN_LSP_WRAPPER_H

#include "spdlog/spdlog.h"
// has to come before the next spdlog include. This comment stops formatter from reordering them
#include "lsp.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/json_types.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <sstream>
#include <string_view>
namespace sorbet::realmain::lsp {

class LSPWrapper {
private:
    static const std::string EMPTY_STRING;

    /** If true, then LSPLoop is initialized and is ready to receive requests. */
    bool initialized = false;

    /** The LSP 'server', which runs in the same thread as LSPWrapper. */
    std::unique_ptr<LSPLoop> lspLoop;

    /** The global state of type checking, as calculated by LSP. */
    std::unique_ptr<core::GlobalState> gs;

    /**
     * Sorbet assumes we 'own' this object; keep it alive to avoid memory errors.
     */
    std::unique_ptr<WorkerPool> workers;
    std::shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink;
    std::shared_ptr<spd::logger> typeErrorsConsole;

    /** The output stream used by LSP. Lets us drain all response messages after sending messages to LSP. */
    std::stringstream lspOstream;

    /** Contains shared constructor logic. */
    void instantiate(std::unique_ptr<core::GlobalState> gs, const std::shared_ptr<spdlog::logger> &logger,
                     bool disableFastPath);

    std::vector<std::unique_ptr<LSPMessage>> drainLSPResponses();

public:
    enum class LSPExperimentalFeature {
        Hover = 1,
        GoToDefinition = 2,
        FindReferences = 3,
        Autocomplete = 4,
        WorkspaceSymbols = 5,
        DocumentSymbol = 6,
        SignatureHelp = 7,
    };

    /** Memory allocator for rapidjson objects. */
    rapidjson::MemoryPoolAllocator<> alloc;

    // N.B.: Sorbet assumes we 'own' this object; keep it alive to avoid memory errors.
    options::Options opts;

    LSPWrapper(std::string_view rootPath = EMPTY_STRING, bool disableFastPath = false);
    LSPWrapper(std::unique_ptr<core::GlobalState> gs, options::Options &&opts,
               const std::shared_ptr<spdlog::logger> &logger, bool disableFastPath);

    /**
     * Send a message to LSP, and returns any responses.
     */
    std::vector<std::unique_ptr<LSPMessage>> getLSPResponsesFor(const LSPMessage &message);

    /**
     * Send a message to LSP, and returns any responses.
     */
    std::vector<std::unique_ptr<LSPMessage>> getLSPResponsesFor(const std::string &json);

    /**
     * Sends multiple messages to LSP, and returns any responses.
     */
    std::vector<std::unique_ptr<LSPMessage>> getLSPResponsesFor(std::vector<std::unique_ptr<LSPMessage>> &messages);

    /**
     * (For tests only) Retrieve the number of times typechecking has run.
     */
    int getTypecheckCount() const;

    /**
     * Enable an experimental LSP feature.
     * Note: Use this method *before* the client performs initialization with the server.
     */
    void enableExperimentalFeature(LSPExperimentalFeature feature);

    /**
     * Enable all experimental LSP features.
     * Note: Use this method *before* the client performs initialization with the server.
     */
    void enableAllExperimentalFeatures();
};

} // namespace sorbet::realmain::lsp

#endif // MAIN_LSP_WRAPPER_H
