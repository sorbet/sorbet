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

/**
 * Returns 'true' if the given GlobalState was originally created from the current contents of
 * kvstore (e.g., kvstore has not since been modified).
 **/
bool kvstoreUnchangedSinceGsCreation(const core::GlobalState &gs, const std::unique_ptr<OwnedKeyValueStore> &kvstore);

// Returns an owned key value store if kvstore is unchanged since gs was created, or false otherwise.
std::unique_ptr<OwnedKeyValueStore> ownIfUnchanged(const core::GlobalState &gs, std::unique_ptr<KeyValueStore> kvstore);

/** Writes the GlobalState to kvstore, but only if it was modified. Returns 'true' if a write happens. */
bool retainGlobalState(core::GlobalState &gs, const realmain::options::Options &options,
                       const std::unique_ptr<OwnedKeyValueStore> &kvstore);

// If kvstore is not null, caches global state and the given files to disk if they have changed. Can silently fail to
// cache
std::unique_ptr<KeyValueStore> maybeCacheGlobalStateAndFiles(std::unique_ptr<KeyValueStore> kvstore,
                                                             const options::Options &opts, core::GlobalState &gs,
                                                             WorkerPool &workers,
                                                             const std::vector<ast::ParsedFile> &indexed);
} // namespace sorbet::realmain::cache

#endif
