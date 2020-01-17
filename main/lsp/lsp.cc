#include "main/lsp/lsp.h"
#include "common/Timer.h"
#include "common/statsd/statsd.h"
#include "common/typecase.h"
#include "common/web_tracer_framework/tracing.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"
#include "main/lsp/LSPTask.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::LSPLoop(std::unique_ptr<core::GlobalState> initialGS, WorkerPool &workers,
                 const std::shared_ptr<LSPConfiguration> &config)
    : config(config), preprocessor(config), typecheckerCoord(config, workers),
      lastMetricUpdateTime(chrono::steady_clock::now()), initialGS(move(initialGS)),
      emptyWorkers(WorkerPool::create(0, *config->logger)) {}

constexpr chrono::minutes STATSD_INTERVAL = chrono::minutes(5);

bool LSPLoop::shouldSendCountersToStatsd(chrono::time_point<chrono::steady_clock> currentTime) const {
    return !config->opts.statsdHost.empty() && (currentTime - lastMetricUpdateTime) > STATSD_INTERVAL;
}

void LSPLoop::sendCountersToStatsd(chrono::time_point<chrono::steady_clock> currentTime) {
    ENFORCE(this_thread::get_id() == mainThreadId, "sendCounterToStatsd can only be called from the main LSP thread.");
    const auto &opts = config->opts;
    // Record rusage-related stats.
    StatsD::addRusageStats();
    auto counters = getAndClearThreadCounters();
    if (!opts.statsdHost.empty()) {
        lastMetricUpdateTime = currentTime;

        auto prefix = fmt::format("{}.lsp.counters", opts.statsdPrefix);
        StatsD::submitCounters(counters, opts.statsdHost, opts.statsdPort, prefix);
    }
    if (!opts.webTraceFile.empty()) {
        web_tracer_framework::Tracing::storeTraces(counters, opts.webTraceFile);
    }
}

namespace {
class TypecheckCountTask : public LSPTask {
    int &count;

public:
    TypecheckCountTask(const LSPConfiguration &config, int &count) : LSPTask(config, true), count(count) {}

    void run(LSPTypecheckerDelegate &tc) override {
        count = tc.state().lspTypecheckCount;
    }
};
} // namespace

int LSPLoop::getTypecheckCount() {
    int count = 0;
    typecheckerCoord.syncRun(make_unique<TypecheckCountTask>(*config, count));
    return count;
}

} // namespace sorbet::realmain::lsp
