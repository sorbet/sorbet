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

unique_ptr<KeyValueStore> maybeCacheGlobalStateAndFiles(unique_ptr<KeyValueStore> kvstore, const options::Options &opts,
                                                        core::GlobalState &gs, WorkerPool &workers,
                                                        const vector<ast::ParsedFile> &indexed) {
    return kvstore;
}

SessionCache::~SessionCache() noexcept(false) {}

string_view SessionCache::kvstorePath() const {
    return string_view(this->path);
}

void SessionCache::reapOldCaches(const options::Options &opts) {}

unique_ptr<SessionCache> SessionCache::make(unique_ptr<const OwnedKeyValueStore> kvstore, ::spdlog::logger &logger,
                                            const options::Options &opts) {
    return nullptr;
}

unique_ptr<KeyValueStore> SessionCache::open(shared_ptr<::spdlog::logger> logger, const options::Options &opts) const {
    return nullptr;
}

} // namespace sorbet::realmain::cache
