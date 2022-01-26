#include "main/lsp/lsp.h"
#include "common/Timer.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "common/statsd/statsd.h"
#include "common/web_tracer_framework/tracing.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "main/lsp/LSPTask.h"
#include "sorbet_version/sorbet_version.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::LSPLoop(std::unique_ptr<core::GlobalState> initialGS, WorkerPool &workers,
                 const std::shared_ptr<LSPConfiguration> &config, std::unique_ptr<KeyValueStore> kvstore)
    : config(config), taskQueueMutex(make_shared<absl::Mutex>()), taskQueue(make_shared<TaskQueueState>()),
      epochManager(initialGS->epochManager), preprocessor(config, taskQueueMutex, taskQueue),
      typecheckerCoord(config, make_shared<core::lsp::PreemptionTaskManager>(initialGS->epochManager), workers),
      indexer(config, move(initialGS), move(kvstore)), emptyWorkers(WorkerPool::create(0, *config->logger)),
      lastMetricUpdateTime(chrono::steady_clock::now()) {}

constexpr chrono::minutes STATSD_INTERVAL = chrono::minutes(5);

bool LSPLoop::shouldSendCountersToStatsd(chrono::time_point<chrono::steady_clock> currentTime) const {
    return !config->opts.statsdHost.empty() && (currentTime - lastMetricUpdateTime) > STATSD_INTERVAL;
}

void LSPLoop::sendCountersToStatsd(chrono::time_point<chrono::steady_clock> currentTime) {
    Timer timeit("LSPLoop::sendCountersToStatsd");
    ENFORCE(this_thread::get_id() == mainThreadId, "sendCounterToStatsd can only be called from the main LSP thread.");
    const auto &opts = config->opts;
    // Record process and version stats. Do this BEFORE clearing the thread counters!
    StatsD::addStandardMetrics();
    auto counters = getAndClearThreadCounters();
    if (!opts.statsdHost.empty()) {
        lastMetricUpdateTime = currentTime;
        auto prefix = fmt::format("{}.lsp.counters", opts.statsdPrefix);
        StatsD::submitCounters(counters, opts.statsdHost, opts.statsdPort, prefix);
    }

    if (!opts.webTraceFile.empty()) {
        timeit.setTag("webtracefile", "true");
        web_tracer_framework::Tracing::storeTraces(counters, opts.webTraceFile);
    } else {
        timeit.setTag("webtracefile", "false");
    }
}

namespace {
class TypecheckCountTask : public LSPTask {
    int &count;

public:
    TypecheckCountTask(const LSPConfiguration &config, int &count)
        : LSPTask(config, LSPMethod::SorbetError), count(count) {}

    bool canPreempt(const LSPIndexer &indexer) const override {
        return false;
    }

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
