#include "main/cache/cache.h"

using namespace std;

namespace sorbet::realmain::cache {
unique_ptr<OwnedKeyValueStore> maybeCreateKeyValueStore(const options::Options &opts) {
    return nullptr;
}

unique_ptr<OwnedKeyValueStore> ownIfUnchanged(const core::GlobalState &gs, unique_ptr<KeyValueStore> kvstore) {
    return nullptr;
}

void maybeCacheGlobalStateAndFiles(unique_ptr<KeyValueStore> kvstore, const options::Options &opts,
                                   core::GlobalState &gs, vector<ast::ParsedFile> &indexed) {
    return;
}

} // namespace sorbet::realmain::cache