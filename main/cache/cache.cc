#include "main/cache/cache.h"
#include "absl/synchronization/notification.h"
#include "common/Random.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/serialize/serialize.h"
#include "main/options/options.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "sorbet_version/sorbet_version.h"

using namespace std;

namespace sorbet::realmain::cache {

namespace {
bool kvstoreUnchangedSinceGsCreation(const core::GlobalState &gs, const u1 *maybeGsBytes) {
    const bool storedUidMatches =
        maybeGsBytes && gs.kvstoreUuid == core::serialize::Serializer::loadGlobalStateUUID(gs, maybeGsBytes);
    const bool noPreviouslyStoredUuid = !maybeGsBytes && gs.kvstoreUuid == 0;
    return storedUidMatches || noPreviouslyStoredUuid;
}

/** Returns 'true' if the given GlobalState was originally created from the current contents of kvstore (e.g., kvstore
 * has not since been modified). */
bool kvstoreUnchangedSinceGsCreation(const core::GlobalState &gs, const unique_ptr<OwnedKeyValueStore> &kvstore) {
    return kvstoreUnchangedSinceGsCreation(gs, kvstore->read(payload::GLOBAL_STATE_KEY).data);
}

/** Writes the GlobalState to kvstore, but only if kvstore hasn't been updated in the mean time. Returns 'true' if a
 * write happens. */
bool retainGlobalState(core::GlobalState &gs, const realmain::options::Options &options,
                       const unique_ptr<OwnedKeyValueStore> &kvstore) {
    ENFORCE_NO_TIMER(kvstore && gs.wasModified() && !gs.hadCriticalError());
    auto maybeGsBytes = kvstore->read(payload::GLOBAL_STATE_KEY);
    // Verify that no other GlobalState was written to kvstore between when we read GlobalState and wrote it
    // into the databaase.
    if (kvstoreUnchangedSinceGsCreation(gs, maybeGsBytes.data)) {
        Timer timeit(gs.tracer(), "write_global_state.kvstore");
        // Generate a new UUID, since this GS has changed since it was read.
        gs.kvstoreUuid = Random::uniformU4();
        kvstore->write(payload::GLOBAL_STATE_KEY, core::serialize::Serializer::storePayloadAndNameTable(gs));
        return true;
    }
    return false;
}
} // namespace

unique_ptr<OwnedKeyValueStore> maybeCreateKeyValueStore(const options::Options &opts) {
    if (opts.cacheDir.empty()) {
        return nullptr;
    }
    return make_unique<OwnedKeyValueStore>(make_unique<KeyValueStore>(sorbet_full_version_string, opts.cacheDir,
                                                                      opts.skipRewriterPasses ? "nodsl" : "default"));
}

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

unique_ptr<Joinable> maybeCacheGlobalStateAndFiles(unique_ptr<KeyValueStore> kvstore, shared_ptr<spdlog::logger> tracer,
                                                   const options::Options &opts, core::GlobalState &gs,
                                                   WorkerPool &workers, vector<ast::ParsedFile> &indexed) {
    if (kvstore == nullptr || !gs.wasModified() || gs.hadCriticalError()) {
        return nullptr;
    }

    auto writeBeginNotif = make_shared<absl::Notification>();
    // We're going to try to commit the changed global state to disk.
    // First, spawn off a new thread for this. kvstore only supports one thread writing to it, and we want to
    // flush our writes to disk asynchronously from the rest of sorbet as it can take awhile on a large project.
    auto thread = runInAThread("cacheGlobalStateAndFiles",
                               [&kvstore, &opts, &gs, writeBeginNotif, &workers, &indexed, tracer]() -> void {
                                   auto ownedKvstore = make_unique<OwnedKeyValueStore>(move(kvstore));
                                   auto wroteGlobalState = retainGlobalState(gs, opts, ownedKvstore);
                                   if (wroteGlobalState) {
                                       // Only write changes to disk if GlobalState changed since the last time.
                                       pipeline::cacheTreesAndFiles(gs, workers, indexed, ownedKvstore);
                                   }

                                   writeBeginNotif->Notify();
                                   // BELOW THIS POINT, WE CANNOT USE ANY REFERENCES TO OBJECTS IN THE
                                   // maybeCacheGlobalStateAndFiles STACK FRAME. Only use tracer and ownedKvstore.
                                   if (wroteGlobalState) {
                                       OwnedKeyValueStore::bestEffortCommit(*tracer, move(ownedKvstore));
                                       prodCounterInc("cache.committed");
                                   } else {
                                       prodCounterInc("cache.aborted");
                                       OwnedKeyValueStore::abort(move(ownedKvstore));
                                   }
                               });
    // Wait until the thread begins writing to kvstore before destroying the stack frame.
    writeBeginNotif->WaitForNotification();
    return thread;
}

} // namespace sorbet::realmain::cache