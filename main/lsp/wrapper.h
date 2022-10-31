#ifndef MAIN_LSP_WRAPPER_H
#define MAIN_LSP_WRAPPER_H

#include "spdlog/spdlog.h"
// has to come before the next spdlog include. This comment stops formatter from reordering them
#include "core/core.h"
#include "main/lsp/LSPMessage.h"
#include "main/options/options.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <string_view>

namespace sorbet {
class WorkerPool;
class KeyValueStore;
} // namespace sorbet

namespace sorbet::realmain::lsp {

class LSPLoop;
class LSPOutputToVector;
class LSPProgrammaticInput;
class LSPConfiguration;

class LSPWrapper {
    // Bugfix: WorkerPool destructor assumes that logger is alive when it runs, so keep around logger until it finishes.
    const std::shared_ptr<spdlog::logger> logger;
    /**
     * Sorbet assumes we 'own' the following three objects; keep them alive to avoid memory errors.
     */
    const std::unique_ptr<WorkerPool> workers;
    const std::shared_ptr<spdlog::sinks::ansicolor_stderr_sink_mt> stderrColorSink;
    const std::shared_ptr<spdlog::logger> typeErrorsConsole;

protected:
    const std::shared_ptr<LSPOutputToVector> output;
    const std::shared_ptr<LSPConfiguration> config_;
    /** The LSP 'server', which runs in the same thread as LSPWrapper (unless multithreading is enabled) */
    const std::shared_ptr<LSPLoop> lspLoop;

    /** Raw constructor. Note: Constructor is unwieldy so we can make class fields `const`. */
    LSPWrapper(std::unique_ptr<core::GlobalState> gs, std::shared_ptr<options::Options> opts,
               std::shared_ptr<spdlog::logger> logger,
               std::shared_ptr<spdlog::sinks::ansicolor_stderr_sink_mt> stderrColorSink,
               std::shared_ptr<spdlog::logger> typeErrorsConsole, std::unique_ptr<KeyValueStore> kvstore,
               bool disableFastPath);

public:
    enum class LSPExperimentalFeature {
        DocumentSymbol = 6,
        SignatureHelp = 7,
        DocumentHighlight = 9,
        DocumentFormat = 10,
        ExperimentalFastPath = 11,
    };

    // N.B.: Sorbet assumes we 'own' this object; keep it alive to avoid memory errors.
    const std::shared_ptr<options::Options> opts;

    virtual ~LSPWrapper();

    const LSPConfiguration &config() const;

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

    /**
     * (For tests only) Retrieve the number of times typechecking has run.
     */
    int getTypecheckCount();

    /**
     * (For tests only) Set a flag that forces the slow path to block indefinitely after saving undo state. Setting
     * this flag to `false` will immediately unblock any currently blocked slow paths.
     */
    void setSlowPathBlocked(bool blocked);
};

class SingleThreadedLSPWrapper final : public LSPWrapper {
    /** Raw constructor. Note: Constructor is unwieldy so we can make class fields `const`. */
    SingleThreadedLSPWrapper(std::unique_ptr<core::GlobalState> gs, std::shared_ptr<options::Options> opts,
                             std::shared_ptr<spdlog::logger> logger,
                             std::shared_ptr<spdlog::sinks::ansicolor_stderr_sink_mt> stderrColorSink,
                             std::shared_ptr<spdlog::logger> typeErrorsConsole, std::unique_ptr<KeyValueStore> kvstore,
                             bool disableFastPath);

public:
    static std::unique_ptr<SingleThreadedLSPWrapper> createWithGlobalState(std::unique_ptr<core::GlobalState> gs,
                                                                           std::shared_ptr<options::Options> options,
                                                                           std::shared_ptr<spdlog::logger> logger,
                                                                           std::unique_ptr<KeyValueStore> kvstore,
                                                                           bool disableFastPath = false);

    static std::unique_ptr<SingleThreadedLSPWrapper>
    create(std::string_view rootPath = std::string_view(),
           std::shared_ptr<options::Options> options = std::make_shared<options::Options>(),
           bool disableFastPath = false);

    /**
     * Send a message to LSP, and returns any responses.
     */
    std::vector<std::unique_ptr<LSPMessage>> getLSPResponsesFor(std::unique_ptr<LSPMessage> message);

    /**
     * Send a message to LSP, and returns any responses.
     */
    std::vector<std::unique_ptr<LSPMessage>> getLSPResponsesFor(const std::string &json);

    /**
     * Sends multiple messages to LSP, and returns any responses.
     */
    std::vector<std::unique_ptr<LSPMessage>> getLSPResponsesFor(std::vector<std::unique_ptr<LSPMessage>> messages);
};

class MultiThreadedLSPWrapper final : public LSPWrapper {
    /** Contains the input object used to feed messages to lspThread. */
    const std::shared_ptr<LSPProgrammaticInput> input;
    /** Contains the thread running LSPLoop. */
    const std::unique_ptr<Joinable> lspThread;

    /** Raw constructor. Note: Constructor is unwieldy so we can make class fields `const`. */
    MultiThreadedLSPWrapper(std::unique_ptr<core::GlobalState> gs, std::shared_ptr<options::Options> opts,
                            std::shared_ptr<spdlog::logger> logger,
                            std::shared_ptr<spdlog::sinks::ansicolor_stderr_sink_mt> stderrColorSink,
                            std::shared_ptr<spdlog::logger> typeErrorsConsole, std::unique_ptr<KeyValueStore> kvstore,
                            bool disableFastPath);

public:
    static std::unique_ptr<MultiThreadedLSPWrapper>
    create(std::string_view rootPath = std::string_view(),
           std::shared_ptr<options::Options> options = std::make_shared<options::Options>(), int numWorkerThreads = 2,
           bool disableFastPath = false);

    ~MultiThreadedLSPWrapper() override;

    /**
     * Sends one message to LSP. Responses can be read asynchronously via `read()`.
     */
    void send(std::unique_ptr<LSPMessage> message);

    /**
     * Sends one message to LSP. Responses can be read asynchronously via `read()`.
     */
    void send(std::vector<std::unique_ptr<LSPMessage>> &messages);

    /**
     * Sends one message to LSP. Responses can be read asynchronously via `read()`.
     */
    void send(const std::string &json);

    /**
     * Blocking read function. Blocks until a read() occurs, or a timeout occurs.
     *
     * A return value of `nullptr` indicates that a timeout has occurred.
     */
    std::unique_ptr<LSPMessage> read(int timeoutMs = 100);
};

} // namespace sorbet::realmain::lsp

#endif // MAIN_LSP_WRAPPER_H
