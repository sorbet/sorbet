#ifndef RUBY_TYPER_CACHE_CACHE_H
#define RUBY_TYPER_CACHE_CACHE_H

#include <memory>
#include <string>

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
std::unique_ptr<KeyValueStore> maybeCacheGlobalStateAndFiles(std::unique_ptr<KeyValueStore> kvstore,
                                                             const options::Options &opts, core::GlobalState &gs,
                                                             WorkerPool &workers,
                                                             const std::vector<ast::ParsedFile> &indexed);

// For situations where it's necessary to have unique ownership of the cache for the duration of the session, the
// `SessionCache` is a good option. It makes a copy of an existing `KeyValueStore`, and provides a mechanism for
// reopening that unique copy. This ensures that the name table present in the kvstore will not change unless it's by
// something with access to the `SessionCache`, making it easier to coexist with other processes that might want to use
// and mutate the cache directory specified by `--cache-dir`.
class SessionCache {
    // The path to the session-unique copy of the cache that was created during initialization.
    std::string path;

    SessionCache() = delete;
    explicit SessionCache(std::string path);

public:
    static const std::string_view SESSION_DIR_PREFIX;
    static const std::string_view OLD_SESSION_DIR_PREFIX;

    // Removes the session cache.
    ~SessionCache() noexcept(false);

    // The path to the unique copy.
    std::string_view kvstorePath() const;

    // Look for old session caches in the cache directory, and remove them if their Sorbet process is no longer active.
    static void reapOldCaches(const options::Options &opts);

    // Close out the kvstore, and create a copy that's stored at `opts.cacheDir + '/' + randomUuid`. If the creation
    // failed, or the kvstore was a `nullptr`, this returns a null pointer.
    static std::unique_ptr<SessionCache> make(std::unique_ptr<const OwnedKeyValueStore> kvstore,
                                              ::spdlog::logger &logger, const options::Options &opts);

    // Open the session cache for use.
    std::unique_ptr<KeyValueStore> open(std::shared_ptr<::spdlog::logger> logger, const options::Options &opts) const;
};

} // namespace sorbet::realmain::cache

#endif
