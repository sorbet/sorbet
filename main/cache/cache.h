#ifndef RUBY_TYPER_CACHE_CACHE_H
#define RUBY_TYPER_CACHE_CACHE_H

#include <memory>

namespace spdlog {
class logger;
}

namespace sorbet {
class KeyValueStore;
class OwnedKeyValueStore;
class WorkerPool;
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
std::unique_ptr<OwnedKeyValueStore> maybeCreateKeyValueStore(std::shared_ptr<::spdlog::logger> logger,
                                                             const options::Options &opts);

// Returns an owned key value store if kvstore is unchanged since gs was created, or false otherwise.
std::unique_ptr<OwnedKeyValueStore> ownIfUnchanged(const core::GlobalState &gs, std::unique_ptr<KeyValueStore> kvstore);

// If kvstore is not null, caches global state and the given files to disk if they have changed. Can silently fail to
// cache
void maybeCacheGlobalStateAndFiles(std::unique_ptr<KeyValueStore> kvstore, const options::Options &opts,
                                   core::GlobalState &gs, WorkerPool &workers,
                                   const std::vector<ast::ParsedFile> &indexed);
} // namespace sorbet::realmain::cache

#endif
