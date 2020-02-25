#ifndef SORBET_KEYVALUESTORE_H
#define SORBET_KEYVALUESTORE_H
#include "absl/synchronization/mutex.h"
#include "common/common.h"
#include <thread>
namespace sorbet {

class OwnedKeyValueStore;

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
    absl::Mutex readers_mtx;

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

    friend class OwnedKeyValueStore;
};

/**
 * A database with single writer and multiple readers.
 * Only the thread that created OwnedKeyValueStore is allowed to invoke `write`.
 * Creating OwnedKeyValueStore grabs a lock and allows to have consistent view over database.
 *
 * To write changes to disk, one must call `commitAndClose` or `disown` with `commitChanges` set to `true`.
 */
class OwnedKeyValueStore final {
    const std::thread::id writerId;
    std::unique_ptr<KeyValueStore> kvstore;
    struct TxnState;
    const std::unique_ptr<TxnState> txnState;

    void clear();
    void refreshMainTransaction();
    int commitOrAbortTransactions(bool commit);

public:
    OwnedKeyValueStore(std::unique_ptr<KeyValueStore> kvstore);
    ~OwnedKeyValueStore();

    /** returns nullptr if not found*/
    u1 *read(std::string_view key);
    std::string_view readString(std::string_view key);
    void writeString(std::string_view key, std::string_view value);
    /** can only be called from owning thread */
    void write(std::string_view key, const std::vector<u1> &value);

    /** Convert an owned key value store into an unowned key value store. If `commitChanges` is `true`, writes are
     * committed to disk. */
    static std::unique_ptr<KeyValueStore> disown(std::unique_ptr<OwnedKeyValueStore> ownedKvstore, bool commitChanges);

    /** Commits all changes to disk and closes the database. */
    static bool commitAndClose(std::unique_ptr<OwnedKeyValueStore> kvstore);
};

} // namespace sorbet

#endif // SORBET_KEYVALUESTORE_H
