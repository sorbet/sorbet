#ifndef RUBY_TYPER_KEYVALUESTORE_H
#define RUBY_TYPER_KEYVALUESTORE_H
#include "common.h"
#include "lmdb.h"
#include <mutex>
#include <thread>
#include <unordered_map>
namespace ruby_typer {

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
    const std::thread::id writerId;
    std::unordered_map<std::thread::id, MDB_txn *> readers;
    std::mutex readersLock;

    void clear();

public:
    KeyValueStore(int version, std::string path);
    /** returns nullptr if not found*/
    u1 *read(const absl::string_view key);
    /** can only be called from main thread */
    void write(const absl::string_view key, std::vector<u1> value);
    ~KeyValueStore() noexcept(false);
};
} // namespace ruby_typer

#endif // RUBY_TYPER_KEYVALUESTORE_H
