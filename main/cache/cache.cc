#include "main/cache/cache.h"
#include "common/kvstore/KeyValueStore.h"
#include "main/options/options.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "version/version.h"

using namespace std;

namespace sorbet::realmain::cache {
unique_ptr<KeyValueStore> maybeCreateKeyValueStore(const options::Options &opts) {
    if (opts.cacheDir.empty()) {
        return nullptr;
    }
    return make_unique<KeyValueStore>(sorbet_full_version_string, opts.cacheDir,
                                      opts.skipRewriterPasses ? "nodsl" : "default");
}

void maybeCacheGlobalStateAndFiles(unique_ptr<KeyValueStore> &kvstore, const options::Options &opts,
                                   core::GlobalState &gs, vector<ast::ParsedFile> &indexed) {
    if (kvstore == nullptr) {
        return;
    }
    auto ownedKvstore = make_unique<OwnedKeyValueStore>(move(kvstore));
    // TODO: Move these methods into this file.
    auto wroteGlobalState = payload::retainGlobalState(gs, opts, ownedKvstore);
    if (wroteGlobalState) {
        // Only write changes to disk if GlobalState changed since the last time.
        pipeline::cacheTreesAndFiles(gs, indexed, ownedKvstore);
        kvstore = OwnedKeyValueStore::bestEffortCommit(gs.tracer(), move(ownedKvstore));
    } else {
        kvstore = OwnedKeyValueStore::abort(move(ownedKvstore));
    }
}

} // namespace sorbet::realmain::cache