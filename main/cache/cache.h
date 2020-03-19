#ifndef RUBY_TYPER_CACHE_CACHE_H
#define RUBY_TYPER_CACHE_CACHE_H

#include <memory>

namespace sorbet {
class KeyValueStore;
namespace ast {
struct ParsedFile;
}
namespace core {
class GlobalState;
}
namespace realmain::options {
struct Options;
}
} // namespace sorbet

namespace sorbet::realmain::cache {
// If cacheDir is specified, creates a KeyValueStore. Otherwise, returns nullptr.
std::unique_ptr<KeyValueStore> maybeCreateKeyValueStore(const options::Options &opts);

// If kvstore is not null, caches global state and the given files to disk if they have changed. Can silently fail to
// cache
void maybeCacheGlobalStateAndFiles(std::unique_ptr<KeyValueStore> &kvstore, const options::Options &opts,
                                   core::GlobalState &gs, std::vector<ast::ParsedFile> &indexed);
} // namespace sorbet::realmain::cache

#endif
