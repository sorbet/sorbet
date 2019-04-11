#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H

#include "common/common.h"
#include "core/core.h"
#include "main/lsp/json_types.h"
#include "rapidjson/document.h"
#include "spdlog/spdlog.h"
#include <mutex>

namespace sorbet::realmain::lsp::watchman {
class WatchmanProcess {
    std::shared_ptr<spdlog::logger> logger;
    rapidjson::MemoryPoolAllocator<> alloc;
    const std::string workSpace;
    const std::vector<std::string> extensions;
    const std::function<void(rapidjson::MemoryPoolAllocator<> &,
                             std::unique_ptr<sorbet::realmain::lsp::WatchmanQueryResponse>)>
        processUpdate;
    const std::function<void(rapidjson::MemoryPoolAllocator<> &, int exitCode)> processExit;
    const std::unique_ptr<Joinable> thread;

private:
    /**
     * Starts up a Watchman subprocess and begins processing file changes. Runs in a dedicated thread.
     */
    void start();

public:
    /**
     * Immediately starts a Watchman subprocess and begins processing file updates in the provided
     * workspace folder. Passes file updates to `processUpdate` function.
     */
    WatchmanProcess(std::shared_ptr<spdlog::logger> logger, std::string_view workSpace,
                    std::vector<std::string> extensions,
                    std::function<void(rapidjson::MemoryPoolAllocator<> &alloc,
                                       std::unique_ptr<sorbet::realmain::lsp::WatchmanQueryResponse>)>
                        processUpdate,
                    std::function<void(rapidjson::MemoryPoolAllocator<> &, int)> processExit);

    ~WatchmanProcess();

    WatchmanProcess(const WatchmanProcess &&) = delete;
    WatchmanProcess(WatchmanProcess &) = delete;
    void operator=(const WatchmanProcess &) = delete;
};
} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H