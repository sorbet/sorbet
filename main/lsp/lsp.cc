#include "main/lsp/lsp.h"
#include "common/Timer.h"
#include "common/statsd/statsd.h"
#include "common/typecase.h"
#include "common/web_tracer_framework/tracing.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::LSPLoop(std::unique_ptr<core::GlobalState> initialGS, const std::shared_ptr<LSPConfiguration> &config,
                 WorkerPool &workers)
    : config(config), preprocessor(move(initialGS), config), typecheckerCoord(config, workers),
      lastMetricUpdateTime(chrono::steady_clock::now()) {}

LSPQueryResult LSPLoop::queryByLoc(LSPTypechecker &typechecker, string_view uri, const Position &pos,
                                   const LSPMethod forMethod, bool errorIfFileIsUntyped) const {
    Timer timeit(config->logger, "setupLSPQueryByLoc");
    const core::GlobalState &gs = typechecker.state();
    auto fref = config->uri2FileRef(gs, uri);
    if (!fref.exists()) {
        auto error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidParams,
            fmt::format("Did not find file at uri {} in {}", uri, convertLSPMethodToString(forMethod)));
        return LSPQueryResult{{}, move(error)};
    }

    if (errorIfFileIsUntyped && fref.data(gs).strictLevel < core::StrictLevel::True) {
        config->logger->info("Ignoring request on untyped file `{}`", uri);
        // Act as if the query returned no results.
        return LSPQueryResult{{}};
    }

    auto loc = config->lspPos2Loc(fref, pos, gs);
    return typechecker.query(core::lsp::Query::createLocQuery(loc), {fref});
}

LSPQueryResult LSPLoop::queryBySymbol(LSPTypechecker &typechecker, core::SymbolRef sym,
                                      optional<string_view> overSingleFile) const {
    Timer timeit(config->logger, "setupLSPQueryBySymbol");
    ENFORCE(sym.exists());
    vector<core::FileRef> frefs;
    if (overSingleFile.has_value()) {
        auto fref = config->uri2FileRef(typechecker.state(), overSingleFile.value());
        if (fref.exists()) {
            frefs.emplace_back(fref);
        }
    } else {
        const core::GlobalState &gs = typechecker.state();
        const core::NameHash symNameHash(gs, sym.data(gs)->name.data(gs));
        // Locate files that contain the same Name as the symbol. Is an overapproximation, but a good first filter.
        int i = -1;
        for (auto &hash : typechecker.getFileHashes()) {
            i++;
            const auto &usedSends = hash.usages.sends;
            const auto &usedConstants = hash.usages.constants;
            auto ref = core::FileRef(i);

            const bool fileIsValid = ref.exists() && ref.data(gs).sourceType == core::File::Type::Normal;
            if (fileIsValid &&
                (std::find(usedSends.begin(), usedSends.end(), symNameHash) != usedSends.end() ||
                 std::find(usedConstants.begin(), usedConstants.end(), symNameHash) != usedConstants.end())) {
                frefs.emplace_back(ref);
            }
        }
    }

    return typechecker.query(core::lsp::Query::createSymbolQuery(sym), frefs);
}

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

int LSPLoop::getTypecheckCount() {
    int count = 0;
    typecheckerCoord.syncRun([&count](const auto &tc) -> void { count = tc.state().lspTypecheckCount.load(); }, false);
    return count;
}

} // namespace sorbet::realmain::lsp
