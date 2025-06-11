#include "main/cache/cache.h"
#include "common/FileOps.h"
#include "common/Random.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/serialize/serialize.h"
#include "main/options/options.h"
#include "sorbet_version/sorbet_version.h"

using namespace std;

namespace sorbet::realmain::cache {

namespace {
unique_ptr<KeyValueStore> openCache(shared_ptr<::spdlog::logger> logger, string cacheDir,
                                    const options::Options &opts) {
    // We currently only support one flavor of cache: "default". Each flavor is a separate database in the LMDB
    // environment, and we'll write all cached trees to that database during indexing. This means that supporting
    // multiple populated flavors in a single environment will increase the size of the environment proportionally to
    // the number of files stored * the number of flavors used. For this reason, we need to be very careful when
    // considering adding a new cache flavor.
    auto flavor = "default";
    auto version = fmt::format("{}|{}", sorbet_full_version_string, opts.cacheSensitiveOptions.serialize());
    return make_unique<KeyValueStore>(std::move(logger), version, std::move(cacheDir), flavor, opts.maxCacheSizeBytes);
}
} // namespace

unique_ptr<OwnedKeyValueStore> maybeCreateKeyValueStore(shared_ptr<::spdlog::logger> logger,
                                                        const options::Options &opts) {
    if (opts.cacheDir.empty()) {
        return nullptr;
    }
    auto ownedKvstore = make_unique<OwnedKeyValueStore>(openCache(logger, opts.cacheDir, opts));
    auto kvstore = OwnedKeyValueStore::bestEffortCommit(*logger, move(ownedKvstore));
    return make_unique<OwnedKeyValueStore>(move(kvstore));
}

namespace {

/**
 * Returns 'true' if the given GlobalState was originally created from the current contents of
 * kvstore (e.g., kvstore has not since been modified).
 */
bool kvstoreChangedSinceGsCreation(const core::GlobalState &gs, const unique_ptr<OwnedKeyValueStore> &kvstore) {
    const uint8_t *maybeGsBytes = kvstore->read(core::serialize::Serializer::NAME_TABLE_KEY).data;
    if (maybeGsBytes) {
        // If `NAME_TABLE_KEY` is in kvstore but it's not what `gs.kvstoreUuid` contains,
        // this implies that some other process wrote a different GlobalState to the cache.
        return gs.kvstoreUuid != core::serialize::Serializer::loadGlobalStateUUID(gs, maybeGsBytes);
    } else {
        // `NAME_TABLE_KEY` is not in kvstore, which implies the cache is empty.
        // If kvstoreUuid != 0, that implies we tried to write GlobalState to the cache once before,
        // and since we don't see `NAME_TABLE_KEY` in there now, someone else overwrote/dropped
        // the cache.
        return gs.kvstoreUuid != 0;
    }
}

/**
 * Writes the GlobalState to kvstore, but only if it was modified. Returns 'true' if the cache
 * now contains the contents of the current GlobalState (either because it already contained it, or
 * it was just written).
 */
bool retainGlobalState(core::GlobalState &gs, const unique_ptr<OwnedKeyValueStore> &kvstore) {
    if (!kvstore) {
        return false;
    }

    if (gs.hadCriticalError()) {
        // If an exception or InternalError happened, something has gone wrong, and we don't know
        // what the contents of GlobalState might be. Don't write into the cache.
        return false;
    }

    // Verify that no other GlobalState was written to kvstore between when we read GlobalState and wrote it
    // into the database.
    if (kvstoreChangedSinceGsCreation(gs, kvstore)) {
        // Either
        //   - `NAME_TABLE_KEY` is not in kvstore && `gs.kvstoreUuid != 0` (kvstore overwritten
        //      with empty cache after we'd already written GlobalState to the cache once), or
        //   - `NAME_TABLE_KEY` is in kvstore, BUT it's not what `gs.kvstoreUuid` contains
        //      (some other process wrote a different GlobalState to the cache)
        return false;
    }

    // Generate a new UUID, since this GS has changed since it was read.
    //
    // It would be valid to always generate a new UUID and write it to the cache, but as an
    // optimization we can skip that when nothing has been modified.
    if (gs.wasModified()) {
        Timer timeit(gs.tracer(), "retainGlobalState.writeNameTable");
        gs.kvstoreUuid = Random::uniformU4();
        kvstore->write(core::serialize::Serializer::NAME_TABLE_KEY, core::serialize::Serializer::storeNameTable(gs));
    }

    return true;
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
                    if (!job->cached() && !file.hasIndexErrors()) {
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
    if (kvstoreChangedSinceGsCreation(gs, ownedKvstore)) {
        // Some other process has written to kvstore; don't use.
        return nullptr;
    }

    return ownedKvstore;
}

unique_ptr<KeyValueStore> maybeCacheGlobalStateAndFiles(unique_ptr<KeyValueStore> kvstore, const options::Options &opts,
                                                        core::GlobalState &gs, WorkerPool &workers,
                                                        const vector<ast::ParsedFile> &indexed) {
    if (kvstore == nullptr) {
        return kvstore;
    }
    auto ownedKvstore = make_unique<OwnedKeyValueStore>(move(kvstore));
    auto cacheHasGlobalState = retainGlobalState(gs, ownedKvstore);
    if (cacheHasGlobalState) {
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
        kvstore = OwnedKeyValueStore::abort(move(ownedKvstore));
    }

    return kvstore;
}

SessionCache::SessionCache(string path) : path{std::move(path)} {}

SessionCache::~SessionCache() noexcept(false) {
    if (!FileOps::dirExists(this->path)) {
        return;
    }

    auto dataMdb = fmt::format("{}/data.mdb", this->path);
    if (FileOps::exists(dataMdb)) {
        FileOps::removeFile(dataMdb);
    }

    auto lockMdb = fmt::format("{}/lock.mdb", this->path);
    if (FileOps::exists(lockMdb)) {
        FileOps::removeFile(lockMdb);
    }

    // Fail silently if the directory has files other than those created by LMDB. This should be fine though, as we will
    // have removed the largest files.
    FileOps::removeEmptyDir(this->path);
}

unique_ptr<SessionCache> SessionCache::make(unique_ptr<const OwnedKeyValueStore> kvstore, ::spdlog::logger &logger,
                                            const options::Options &opts) {
    if (kvstore == nullptr || opts.cacheDir.empty()) {
        return nullptr;
    }

    string path;

    // Pretty unlikely that we'll see a collision, as we're removing the directory on exit and also generating random
    // names, but make two attempts to find a new one anyway.
    for (int i = 0; i < 2; i++) {
        // Store the session cache as a sub folder of the cacheDir, so we don't write additional cache data to
        // an unexpected location.
        path = fmt::format("{}/session-{:x}", opts.cacheDir, Random::uniformU4());
        if (!FileOps::dirExists(path)) {
            break;
        }
        path.clear();
    }

    if (path.empty()) {
        Exception::raise("Cache copying failed: failed to make a unique temp directory");
    }

    kvstore->copyTo(path);

    OwnedKeyValueStore::abort(std::move(kvstore));

    // Explicit construction because the constructor is private.
    return unique_ptr<SessionCache>(new SessionCache{std::move(path)});
}

string_view SessionCache::kvstorePath() const {
    return string_view(this->path);
}

unique_ptr<KeyValueStore> SessionCache::open(shared_ptr<::spdlog::logger> logger, const options::Options &opts) const {
    // If the session copy has disappeared, we return a nullptr to force downstream consumers to explicitly handle the
    // empty cache.
    if (!FileOps::dirExists(this->path)) {
        return nullptr;
    }

    auto kvstore = make_unique<const OwnedKeyValueStore>(openCache(std::move(logger), this->path, opts));

    // If the name table entry is missing, this indicates that the cache is completely fresh and doesn't originate in a
    // copy from the result of indexing. This can happen if only the `data.mdb` file was removed from `this->path`, and
    // the KeyValueStore created a fresh database when created.
    const auto nameTableEntry = kvstore->read(core::serialize::Serializer::NAME_TABLE_KEY);
    if (nameTableEntry.len == 0) {
        return nullptr;
    }

    // We abort here because there will be no outstanding transactions present.
    return OwnedKeyValueStore::abort(std::move(kvstore));
}

} // namespace sorbet::realmain::cache
