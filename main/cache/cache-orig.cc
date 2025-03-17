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

std::string_view SessionCache::kvstorePath() const {
    return std::string_view(this->path);
}

std::unique_ptr<SessionCache> SessionCache::make(std::unique_ptr<const OwnedKeyValueStore> kvstore,
                                                 ::spdlog::logger &logger, const options::Options &opts) {
    return nullptr;
}

std::unique_ptr<KeyValueStore> SessionCache::open(std::shared_ptr<::spdlog::logger> logger) const {
    return nullptr;
}

} // namespace sorbet::realmain::cache
