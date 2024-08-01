#include "common/kvstore/KeyValueStore.h"
#include "main/cache/cache.h"

using namespace std;

namespace sorbet::realmain::cache {
unique_ptr<OwnedKeyValueStore> maybeCreateKeyValueStore(shared_ptr<::spdlog::logger> logger,
                                                        const options::Options &opts) {
    return nullptr;
}

unique_ptr<OwnedKeyValueStore> ownIfUnchanged(const core::GlobalState &gs, unique_ptr<KeyValueStore> kvstore) {
    return nullptr;
}

void maybeCacheGlobalStateAndFiles(unique_ptr<KeyValueStore> kvstore, const options::Options &opts,
                                   core::GlobalState &gs, WorkerPool &workers, const vector<ast::ParsedFile> &indexed) {
    return;
}

} // namespace sorbet::realmain::cache
