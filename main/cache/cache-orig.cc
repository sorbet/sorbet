#include "main/cache/cache.h"

using namespace std;

namespace sorbet::realmain::cache {
unique_ptr<OwnedKeyValueStore> maybeCreateKeyValueStore(const options::Options &opts) {
    return nullptr;
}

unique_ptr<OwnedKeyValueStore> ownIfUnchanged(const core::GlobalState &gs, unique_ptr<KeyValueStore> kvstore) {
    return nullptr;
}

unique_ptr<Joinable> maybeCacheGlobalStateAndFiles(unique_ptr<KeyValueStore> kvstore, shared_ptr<spdlog::logger> tracer,
                                                   const options::Options &opts, core::GlobalState &gs,
                                                   WorkerPool &workers, vector<ast::ParsedFile> &indexed) {
    return nullptr;
}

} // namespace sorbet::realmain::cache