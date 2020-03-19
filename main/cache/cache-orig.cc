#include "main/cache/cache.h"

using namespace std;

namespace sorbet::realmain::cache {
unique_ptr<KeyValueStore> maybeCreateKeyValueStore(const options::Options &opts) {
    return nullptr;
}

void maybeCacheGlobalStateAndFiles(unique_ptr<KeyValueStore> &kvstore, const options::Options &opts,
                                   core::GlobalState &gs, vector<ast::ParsedFile> &indexed) {
    return;
}

} // namespace sorbet::realmain::cache