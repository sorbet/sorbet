#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H

#include "absl/synchronization/mutex.h"
#include "common/common.h"
#include "core/core.h"
#include "spdlog/spdlog.h"

namespace sorbet::realmain::lsp {
class WatchmanQueryResponse;
class WatchmanStateEnter;
class WatchmanStateLeave;
} // namespace sorbet::realmain::lsp

namespace sorbet::realmain::lsp::watchman {
class WatchmanProcess {
protected:
    std::shared_ptr<spdlog::logger> logger;

private:
    const std::string watchmanPath;
    const std::string workSpace;
    const std::vector<std::string> extensions;
    const std::unique_ptr<Joinable> thread;
    // Mutex that must be held before reading or writing stopped.
    absl::Mutex mutex;
    // If true, the process has been stopped.
    bool stopped = false;

    /**
     * Starts up a Watchman subprocess and begins processing file changes. Runs in a dedicated thread.
     */
    void start();

    void exitWithCode(int code, const std::optional<std::string> &);

    bool isStopped();

protected:
    virtual void processQueryResponse(std::unique_ptr<sorbet::realmain::lsp::WatchmanQueryResponse>) = 0;

    virtual void processStateEnter(std::unique_ptr<sorbet::realmain::lsp::WatchmanStateEnter>) = 0;

    virtual void processStateLeave(std::unique_ptr<sorbet::realmain::lsp::WatchmanStateLeave>) = 0;

    virtual void processExit(int core, const std::optional<std::string> &) = 0;

public:
    /**
     * Immediately starts a Watchman subprocess and begins processing file updates in the provided
     * workspace folder. Passes file updates to `processUpdate` function.
     */
    WatchmanProcess(std::shared_ptr<spdlog::logger> logger, std::string_view watchmanPath, std::string_view workSpace,
                    std::vector<std::string> extensions);

    virtual ~WatchmanProcess();

    WatchmanProcess(const WatchmanProcess &&) = delete;
    WatchmanProcess(WatchmanProcess &) = delete;
    void operator=(const WatchmanProcess &) = delete;
};
} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H
