#ifndef SORBET_CACHELOCK_H
#define SORBET_CACHELOCK_H

#include <optional>
#include <string>
#include <string_view>

namespace sorbet {

// Owns a lock on a cache directory's access lock file. Destruction releases the lock but leaves the file in place.
class CacheLock final {
    int fd;

    explicit CacheLock(int fd);

public:
    // Kept in place across opens and database replacement. Only private session-cache cleanup removes it.
    static constexpr std::string_view ACCESS_LOCK_FILE = "cache-access.lock";

    CacheLock(const CacheLock &) = delete;
    CacheLock &operator=(const CacheLock &) = delete;
    CacheLock(CacheLock &&other) noexcept;
    CacheLock &operator=(CacheLock &&other) noexcept;
    ~CacheLock();

    // The cache directory must exist. Returns nullopt on failure and sets errno; nonblocking contention sets
    // EWOULDBLOCK. A successful result holds the lock until destroyed or reset.
    static std::optional<CacheLock> acquireShared(const std::string &path, bool blocking = true);
    static std::optional<CacheLock> acquireExclusive(const std::string &path, bool blocking = true);
};

} // namespace sorbet

#endif
