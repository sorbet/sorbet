#include "main/cache/cache.h"
#include "common/Random.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/serialize/serialize.h"
#include "main/options/options.h"
#include "sorbet_version/sorbet_version.h"

using namespace std;

namespace sorbet::realmain::cache {
unique_ptr<OwnedKeyValueStore> maybeCreateKeyValueStore(shared_ptr<::spdlog::logger> logger,
                                                        const options::Options &opts) {
    if (opts.cacheDir.empty()) {
        return nullptr;
    }
    // Despite being called "experimental," this feature is actually stable. We just didn't want to
    // bust all existing caches when we promoted the experimental-at-the-time incremental fast path
    // to the stable version.
    auto flavor = "experimentalfastpath";
    return make_unique<OwnedKeyValueStore>(make_unique<KeyValueStore>(logger, sorbet_full_version_string, opts.cacheDir,
                                                                      move(flavor), opts.maxCacheSizeBytes));
}

namespace {

/**
 * Returns 'true' if the given GlobalState was originally created from the current contents of
 * kvstore (e.g., kvstore has not since been modified).
 */
bool kvstoreUnchangedSinceGsCreation(const core::GlobalState &gs, const unique_ptr<OwnedKeyValueStore> &kvstore) {
    const uint8_t *maybeGsBytes = kvstore->read(core::serialize::Serializer::GLOBAL_STATE_KEY).data;
    const bool storedUidMatches =
        maybeGsBytes && gs.kvstoreUuid == core::serialize::Serializer::loadGlobalStateUUID(gs, maybeGsBytes);
    const bool noPreviouslyStoredUuid = !maybeGsBytes && gs.kvstoreUuid == 0;
    return storedUidMatches || noPreviouslyStoredUuid;
}

/**
 * Writes the GlobalState to kvstore, but only if it was modified. Returns 'true' if a write happens.
 */
bool retainGlobalState(core::GlobalState &gs, const unique_ptr<OwnedKeyValueStore> &kvstore) {
    if (kvstore && gs.wasModified() && !gs.hadCriticalError()) {
        // Verify that no other GlobalState was written to kvstore between when we read GlobalState and wrote it
        // into the database.
        if (kvstoreUnchangedSinceGsCreation(gs, kvstore)) {
            // Generate a new UUID, since this GS has changed since it was read.
            gs.kvstoreUuid = Random::uniformU4();
            kvstore->write(core::serialize::Serializer::GLOBAL_STATE_KEY,
                           core::serialize::Serializer::storePayloadAndNameTable(gs));
            return true;
        }
    }
    return false;
}

bool cacheTreesAndFiles(const core::GlobalState &gs, WorkerPool &workers, absl::Span<const ast::ParsedFile> parsedFiles,
                        const unique_ptr<OwnedKeyValueStore> &kvstore) {
    if (kvstore == nullptr) {
        return false;
    }

    Timer timeit(gs.tracer(), "pipeline::cacheTreesAndFiles");

    // Compress files in parallel.
    auto fileq = make_shared<ConcurrentBoundedQueue<const ast::ParsedFile *>>(parsedFiles.size());
    for (auto &parsedFile : parsedFiles) {
        fileq->push(&parsedFile, 1);
    }

    auto resultq = make_shared<BlockingBoundedQueue<vector<pair<string, vector<uint8_t>>>>>(parsedFiles.size());
    workers.multiplexJob("compressTreesAndFiles", [fileq, resultq, &gs]() {
        vector<pair<string, vector<uint8_t>>> threadResult;
        int processedByThread = 0;
        const ast::ParsedFile *job = nullptr;
        unique_ptr<Timer> timeit;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;
                    if (timeit == nullptr) {
                        timeit = make_unique<Timer>(gs.tracer(), "cacheTreesAndFilesWorker");
                    }

                    if (!job->file.exists()) {
                        continue;
                    }

                    auto &file = job->file.data(gs);
                    if (!file.cached() && !file.hasParseErrors()) {
                        threadResult.emplace_back(core::serialize::Serializer::fileKey(file),
                                                  core::serialize::Serializer::storeTree(file, *job));
                        // Stream out compressed files so that writes happen in parallel with processing.
                        if (processedByThread > 100) {
                            resultq->push(move(threadResult), processedByThread);
                            processedByThread = 0;
                        }
                    }
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    size_t written = 0;
    {
        vector<pair<string, vector<uint8_t>>> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                for (auto &a : threadResult) {
                    kvstore->write(move(a.first), move(a.second));
                    written++;
                }
            }
        }
    }
    prodCounterAdd("types.input.files.kvstore.write", written);
    return written != 0;
}

} // namespace

unique_ptr<OwnedKeyValueStore> ownIfUnchanged(const core::GlobalState &gs, unique_ptr<KeyValueStore> kvstore) {
    if (kvstore == nullptr) {
        return nullptr;
    }

    auto ownedKvstore = make_unique<OwnedKeyValueStore>(move(kvstore));
    if (kvstoreUnchangedSinceGsCreation(gs, ownedKvstore)) {
        return ownedKvstore;
    }

    // Some other process has written to kvstore; don't use.
    return nullptr;
}

unique_ptr<KeyValueStore> maybeCacheGlobalStateAndFiles(unique_ptr<KeyValueStore> kvstore, const options::Options &opts,
                                                        core::GlobalState &gs, WorkerPool &workers,
                                                        const vector<ast::ParsedFile> &indexed) {
    if (kvstore == nullptr) {
        return kvstore;
    }
    auto ownedKvstore = make_unique<OwnedKeyValueStore>(move(kvstore));
    auto wroteGlobalState = retainGlobalState(gs, ownedKvstore);
    if (wroteGlobalState) {
        cacheTreesAndFiles(gs, workers, indexed, ownedKvstore);
        auto sizeBytes = ownedKvstore->cacheSize();
        kvstore = OwnedKeyValueStore::bestEffortCommit(gs.tracer(), move(ownedKvstore));
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

    return kvstore;
}

} // namespace sorbet::realmain::cache
