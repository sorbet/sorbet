#include "main/cache/cache.h"
#include "common/kvstore/KeyValueStore.h"
#include "main/options/options.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "sorbet_version/sorbet_version.h"

using namespace std;

namespace sorbet::realmain::cache {
unique_ptr<OwnedKeyValueStore> maybeCreateKeyValueStore(shared_ptr<::spdlog::logger> logger,
                                                        const options::Options &opts) {
    if (opts.cacheDir.empty()) {
        return nullptr;
    }
    auto flavor = opts.lspExperimentalFastPathEnabled ? "experimentalfastpath" : "normalfastpath";
    return make_unique<OwnedKeyValueStore>(make_unique<KeyValueStore>(logger, sorbet_full_version_string, opts.cacheDir,
                                                                      move(flavor), opts.maxCacheSizeBytes));
}

unique_ptr<OwnedKeyValueStore> ownIfUnchanged(const core::GlobalState &gs, unique_ptr<KeyValueStore> kvstore) {
    if (kvstore == nullptr) {
        return nullptr;
    }

    auto ownedKvstore = make_unique<OwnedKeyValueStore>(move(kvstore));
    if (payload::kvstoreUnchangedSinceGsCreation(gs, ownedKvstore)) {
        return ownedKvstore;
    }

    // Some other process has written to kvstore; don't use.
    return nullptr;
}

void maybeCacheGlobalStateAndFiles(unique_ptr<KeyValueStore> kvstore, const options::Options &opts,
                                   core::GlobalState &gs, WorkerPool &workers, vector<ast::ParsedFile> &indexed) {
    if (kvstore == nullptr) {
        return;
    }
    auto ownedKvstore = make_unique<OwnedKeyValueStore>(move(kvstore));
    // TODO: Move these methods into this file.
    auto wroteGlobalState = payload::retainGlobalState(gs, opts, ownedKvstore);
    if (wroteGlobalState) {
        // Only write changes to disk if GlobalState changed since the last time.
        pipeline::cacheTreesAndFiles(gs, workers, indexed, ownedKvstore);
        auto sizeBytes = ownedKvstore->cacheSize();
        OwnedKeyValueStore::bestEffortCommit(gs.tracer(), move(ownedKvstore));
        prodCounterInc("cache.committed");

        size_t usedPercent = round((sizeBytes * 100.0) / opts.maxCacheSizeBytes);
        prodCounterSet("cache.used_bytes", sizeBytes);
        prodCounterSet("cache.used_percent", usedPercent);
        gs.tracer().debug("sorbet_version={} cache_used_bytes={} cache_used_percent={}", sorbet_full_version_string,
                          sizeBytes, usedPercent);
    } else {
        prodCounterInc("cache.aborted");
        OwnedKeyValueStore::abort(move(ownedKvstore));
    }
}

} // namespace sorbet::realmain::cache
