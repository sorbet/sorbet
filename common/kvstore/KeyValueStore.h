#ifndef SORBET_KEYVALUESTORE_H
#define SORBET_KEYVALUESTORE_H
#include "absl/synchronization/mutex.h"
#include "common/common.h"
#include "lmdb.h"
#include <thread>
namespace sorbet {

/**
 * A database with single writer and multiple readers.
 * Only the thread that created KeyValueStore is allowed to invoke Write.
 * Creating KeyValueStore grabs a lock and allows to have consistent view over database.
 */
class KeyValueStore {
    MDB_env *env;
    MDB_dbi dbi;
    MDB_txn *txn;
    const std::string path;
    const std::string flavor;
    const std::thread::id writerId;
    UnorderedMap<std::thread::id, MDB_txn *> readers;
    absl::Mutex readers_mtx;
    bool commited = false;

    void clear();
    void refreshMainTransaction();

public:
    KeyValueStore(std::string version, std::string path, std::string flavor);
    /** returns nullptr if not found*/
    u1 *read(std::string_view key);
    std::string_view readString(std::string_view key);
    void writeString(std::string_view key, std::string_view value);
    /** can only be called from main thread */
    void write(std::string_view key, const std::vector<u1> &value);
    ~KeyValueStore() noexcept(false);
    static bool commit(std::unique_ptr<KeyValueStore>);
};
} // namespace sorbet

#endif // SORBET_KEYVALUESTORE_H
