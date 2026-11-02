#include "main/cache/cache.h"
#include "absl/cleanup/cleanup.h"
#include "common/FileOps.h"
#include "common/Random.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/exception/Exception.h"
#include "common/kvstore/CacheLock.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/serialize/serialize.h"
#include "main/options/options.h"
#include "sorbet_version/sorbet_version.h"
#include <cerrno>
#include <charconv>
#include <cstdio>
#include <filesystem>
#include <unistd.h>

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
    auto version = fmt::format("{}|{}", sorbet::full_version_string, opts.cacheSensitiveOptions.serialize());
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
    const uint8_t *maybeUUIDBytes = kvstore->read(core::serialize::Serializer::NAME_TABLE_UUID_KEY).data;
    if (maybeUUIDBytes) {
        // If `NAME_TABLE_UUID_KEY` is in kvstore but it's not what `gs.kvstoreUuid` contains, this implies that some
        // other process wrote a different GlobalState to the cache.
        return gs.kvstoreUuid != core::serialize::Serializer::loadGlobalStateUUID(gs, maybeUUIDBytes);
    } else {
        // `NAME_TABLE_UUID_KEY` is not in kvstore, which implies the cache is empty. If kvstoreUuid != 0, that implies
        // we tried to write GlobalState to the cache once before, and since we don't see `NAME_TABLE_UUID_KEY` in there
        // now, someone else overwrote/dropped the cache.
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
        //   - `NAME_TABLE_UUID_KEY` is not in kvstore && `gs.kvstoreUuid != 0` (kvstore overwritten
        //      with empty cache after we'd already written GlobalState to the cache once), or
        //   - `NAME_TABLE_UUID_KEY` is in kvstore, BUT it's not what `gs.kvstoreUuid` contains
        //      (some other process wrote a different GlobalState to the cache)
        return false;
    }

    // Generate a new UUID, since this GS has changed since it was read.
    //
    // It would be valid to always generate a new UUID and write it to the cache, but as an
    // optimization we can skip that when nothing has been modified.
    if (gs.wasNameTableModified()) {
        Timer timeit(gs.tracer(), "retainGlobalState.writeNameTable");
        gs.kvstoreUuid = Random::uniformU4();
        kvstore->write(core::serialize::Serializer::NAME_TABLE_UUID_KEY, core::serialize::Serializer::storeUUID(gs));

        // Write the diff entry before updating the metadata (diff count + hash size), so that a
        // failure to write the diff doesn't corrupt the existing cached data.
        kvstore->write(core::serialize::Serializer::nameTableDiffKey(gs.getNameTableDiffCount()),
                       core::serialize::Serializer::storeNameTableDiff(gs));

        gs.incrementNameTableDiffCount();
        kvstore->write(core::serialize::Serializer::NAME_TABLE_DIFF_COUNT_AND_HASH_SIZE_KEY,
                       core::serialize::Serializer::storeNameTableDiffCountAndHashSize(gs.getNameTableDiffCount(), gs));
        gs.markNameTableAsCached();
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
        gs.tracer().debug("sorbet_version={} cache_used_bytes={} cache_used_percent={}", sorbet::full_version_string,
                          sizeBytes, usedPercent);
    } else {
        prodCounterInc("cache.aborted");
        kvstore = OwnedKeyValueStore::abort(move(ownedKvstore));
    }

    return kvstore;
}

namespace {
// Missing files and directories containing unrelated files are acceptable.
bool removeCacheDir(const string &path) noexcept {
    auto dataMdb = fmt::format("{}/data.mdb", path);
    if (0 != remove(dataMdb.c_str())) {
        switch (errno) {
            // Another process may have already removed the database.
            case ENOENT:
                break;

            default:
                return false;
        }
    }

    auto lockMdb = fmt::format("{}/lock.mdb", path);
    if (0 != remove(lockMdb.c_str())) {
        switch (errno) {
            // Another process may have already removed the lock file.
            case ENOENT:
                break;

            default:
                return false;
        }
    }

    auto accessLock = fmt::format("{}/{}", path, CacheLock::ACCESS_LOCK_FILE);
    if (0 != remove(accessLock.c_str()) && errno != ENOENT) {
        return false;
    }

    // Leave directories containing unrelated files intact.
    if (0 != rmdir(path.c_str())) {
        switch (errno) {
            // It's fine if the directory is missing, isn't actually a directory, or wasn't empty.
            case ENOENT:
            case ENOTDIR:
            case ENOTEMPTY:
                break;

            default:
                return false;
        }
    }

    return true;
}
} // namespace

const string_view SessionCache::SESSION_DIR_PREFIX = "sorbet-session-";
const string_view SessionCache::STAGING_DIR_PREFIX = "sorbet-publish-";

SessionCache::SessionCache(string path) : path{std::move(path)} {}

SessionCache::~SessionCache() noexcept(false) {
    removeCacheDir(this->path);
}

void SessionCache::reapOldCaches(const options::Options &opts) {
    if (opts.cacheDir.empty() || !FileOps::dirExists(opts.cacheDir)) {
        return;
    }

    for (const auto &dir : FileOps::listSubdirs(opts.cacheDir)) {
        string_view pidStr;
        if (dir.find(SESSION_DIR_PREFIX) == 0) {
            pidStr = string_view(dir).substr(SESSION_DIR_PREFIX.size());
        } else if (dir.find(STAGING_DIR_PREFIX) == 0) {
            pidStr = string_view(dir).substr(STAGING_DIR_PREFIX.size());
        } else {
            continue;
        }

        pid_t pid = -1;
        auto res = std::from_chars(pidStr.begin(), pidStr.end(), pid);
        if (res.ec != std::errc{} || res.ptr != pidStr.end() || pid <= 0) {
            continue;
        }

        if (processExists(pid) != ProcessStatus::Missing) {
            continue;
        }

        removeCacheDir(fmt::format("{}/{}", opts.cacheDir, dir));
    }
}

unique_ptr<SessionCache> SessionCache::make(unique_ptr<const OwnedKeyValueStore> kvstore, ::spdlog::logger &logger,
                                            const options::Options &opts) {
    if (kvstore == nullptr || opts.cacheDir.empty()) {
        return nullptr;
    }

    auto pid = getpid();
    auto path = fmt::format("{}/{}{}", opts.cacheDir, SESSION_DIR_PREFIX, pid);

    // If the directory already exists, we're reusing a PID from a previous run of Sorbet.
    if (FileOps::dirExists(path) && !removeCacheDir(path)) {
        throw RemoveDirException(fmt::format("Failed to remove existing session cache '{}': {}", path, errno));
    }

    kvstore->copyTo(path);

    OwnedKeyValueStore::abort(std::move(kvstore));

    // Explicit construction because the constructor is private.
    return unique_ptr<SessionCache>(new SessionCache{std::move(path)});
}

string_view SessionCache::kvstorePath() const {
    return string_view(this->path);
}

unique_ptr<OwnedKeyValueStore> SessionCache::open(shared_ptr<::spdlog::logger> logger,
                                                  const options::Options &opts) const {
    // If the session copy has disappeared, we return a nullptr to force downstream consumers to explicitly handle the
    // empty cache.
    if (!FileOps::dirExists(this->path)) {
        return nullptr;
    }

    auto kvstore = make_unique<OwnedKeyValueStore>(openCache(std::move(logger), this->path, opts));

    // If the name table entry is missing, this indicates that the cache is completely fresh and doesn't originate in a
    // copy from the result of indexing. This can happen if only the `data.mdb` file was removed from `this->path`, and
    // the KeyValueStore created a fresh database when created.
    const auto nameTableDiffCountEntry =
        kvstore->read(core::serialize::Serializer::NAME_TABLE_DIFF_COUNT_AND_HASH_SIZE_KEY);
    if (nameTableDiffCountEntry.len == 0) {
        return nullptr;
    }

    return kvstore;
}

void SessionCache::commit(shared_ptr<::spdlog::logger> logger, const options::Options &opts) const {
    if (opts.cacheDir.empty()) {
        return;
    }
    auto kvstore = this->open(std::move(logger), opts);
    if (kvstore == nullptr) {
        return;
    }

    // Copy into the destination filesystem before atomically replacing data.mdb. This preserves both the
    // shared database on a failed copy and the session database for subsequent use.
    auto stagingPath = fmt::format("{}/{}{}", opts.cacheDir, STAGING_DIR_PREFIX, getpid());
    removeCacheDir(stagingPath);
    absl::Cleanup cleanupStagingDirectory = [&stagingPath] { removeCacheDir(stagingPath); };

    kvstore->copyTo(stagingPath);
    kvstore.reset();

    // Skip publication if the shared cache is still in use.
    auto accessLock = CacheLock::acquireExclusive(opts.cacheDir, /* blocking */ false);
    if (!accessLock.has_value()) {
        prodCounterInc("cache.publish_failed");
        return;
    }
    filesystem::rename(fmt::format("{}/data.mdb", stagingPath), fmt::format("{}/data.mdb", opts.cacheDir));
    prodCounterInc("cache.publish_success");
}

} // namespace sorbet::realmain::cache
