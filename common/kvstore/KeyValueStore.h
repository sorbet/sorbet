#ifndef SORBET_KEYVALUESTORE_H
#define SORBET_KEYVALUESTORE_H
#include "absl/synchronization/mutex.h"
#include "common/common.h"
#include <thread>
namespace sorbet {

class OwnedKeyValueStore;

struct KeyValueStoreValue {
    uint8_t *data;
    size_t len;
};

/**
 * A database with single writer and multiple readers. Must be owned by a particular thread (via `OwnedKeyValueStore`)
 * before it can be used.
 */
class KeyValueStore final {
    const std::string version;
    const std::string path;
    const std::string flavor;
    struct DBState;
    const std::unique_ptr<DBState> dbState;

public:
    /**
     * A KeyValueStore lives at a given `path` on disk, which must be
     * a pre-existing, writeable, directory.
     *
     * `version` contains a string naming the format of the data
     * contained therein. If the format or semantics of the serialized
     * form changes, `version` should change. Only one `version` will
     * be saved in the cache database at a time; If the existing
     * database has data from a different `version`, it will be
     * cleared on creation.
     *
     * Multiple `flavor`s, however, may coexist within the
     * database. `flavor` should encode the set of configuration or
     * other options that may affect the cached data. Two
     * `KeyValueStore`s opened with different `flavor`s will not share
     * any entries, but each will see their own set of values.
     */
    KeyValueStore(std::string version, std::string path, std::string flavor);
    ~KeyValueStore() noexcept(false);

    // Used in tests to assert that OwnedKeyValueStore cleans up reader transactions.
    void enforceNoOutstandingReaders() const;

    friend class OwnedKeyValueStore;
};

// Usage:
// unique_ptr<OwnedKeyValueStore> kvstore = ...;
// ...
// {
//   unique_ptr<SuspendedKVStore> suspended = SuspendedKVStore::suspend(move(kvstore));
//   // Make other kv store
//   kvstore = SuspendedKVStore::resume(move(suspended)); // Put it back
// }
class SuspendedKVStore {
    std::unique_ptr<OwnedKeyValueStore> okvs;

public:
    // Commits and closes the OwnedKeyValueStore, so that we can temporarily own a different one.
    // This also sets kvstoreInUse to false
    static std::unique_ptr<SuspendedKVStore> suspend(std::unique_ptr<OwnedKeyValueStore> kv);

    static std::unique_ptr<OwnedKeyValueStore> resume(std::unique_ptr<SuspendedKVStore> kv);
};

/**
 * A database with single writer and multiple readers.
 * Only the thread that created OwnedKeyValueStore is allowed to invoke `write`.
 * Creating OwnedKeyValueStore grabs a lock and allows to have consistent view over database.
 *
 * To write changes to disk, one must call `commit`.
 */
class OwnedKeyValueStore final {
    const std::thread::id writerId;
    // Mutable so that private method abort() can be const.
    mutable std::unique_ptr<KeyValueStore> kvstore;
    struct TxnState;
    const std::unique_ptr<TxnState> txnState;
    mutable absl::Mutex readers_mtx;

    void writeInternal(std::string_view key, void *value, size_t len);
    void clear();
    void refreshMainTransaction();
    int commit();
    void abort() const;

public:
    OwnedKeyValueStore(std::unique_ptr<KeyValueStore> kvstore);
    ~OwnedKeyValueStore();

    /** returns nullptr if not found*/
    KeyValueStoreValue read(std::string_view key) const;
    /** Reads a string from the key value store. Lifetime of string is tied to the lifetime of the database. Returns an
     * empty string_view if the key does not exist. */
    std::string_view readString(std::string_view key) const;
    void writeString(std::string_view key, std::string_view value);
    /** can only be called from owning thread */
    void write(std::string_view key, const std::vector<uint8_t> &value);

    /** Aborts all changes without writing them to disk. Returns an unowned kvstore that can be re-owned if more writes
     * are desired. If not explicitly called, OwnedKeyValueStore will implicitly abort everything in the destructor.
     * Must be called by the owning thread. */
    static std::unique_ptr<KeyValueStore> abort(std::unique_ptr<const OwnedKeyValueStore> ownedKvstore);

    /** Attempts to commit all changes to disk. Can fail to commit changes silently. Returns an unowned kvstore that can
     * be re-owned if more writes are desired.  Must be called by the owning thread. */
    static std::unique_ptr<KeyValueStore> bestEffortCommit(spdlog::logger &logger,
                                                           std::unique_ptr<OwnedKeyValueStore> ownedKvstore);
};

} // namespace sorbet

#endif // SORBET_KEYVALUESTORE_H
