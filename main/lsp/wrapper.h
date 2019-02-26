#include "spdlog/spdlog.h"
// has to come before the next spdlog include. This comment stops formatter from reordering them
#include "lsp.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/json_types.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <sstream>
namespace sorbet::realmain::lsp {

class LSPWrapper {
private:
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

public:
    /** Memory allocator for rapidjson objects. */
    rapidjson::MemoryPoolAllocator<> alloc;

    // N.B.: Sorbet assumes we 'own' this object; keep it alive to avoid memory errors.
    options::Options opts;

    LSPWrapper(bool disableFastPath = false);
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
};

} // namespace sorbet::realmain::lsp
