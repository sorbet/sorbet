#include "common/kvstore/KeyValueStore.h"
#include "common/EarlyReturnWithCode.h"
#include "common/Timer.h"
#include "lmdb.h"

#include <utility>

using namespace std;
namespace sorbet {
constexpr string_view OLD_VERSION_KEY = "VERSION"sv;
constexpr string_view VERSION_KEY = "DB_FORMAT_VERSION"sv;
constexpr size_t MAX_DB_SIZE_BYTES =
    4L * 1024 * 1024 * 1024; // 4G. This is both maximum fs db size and max virtual memory usage.
struct KeyValueStore::DBState {
    MDB_env *env;
};

struct OwnedKeyValueStore::TxnState {
    MDB_dbi dbi;
    MDB_txn *txn;
    UnorderedMap<std::thread::id, MDB_txn *> readers;
};

namespace {
static void throw_mdb_error(string_view what, int err) {
    fmt::print(stderr, "mdb error: {}: {}\n", what, mdb_strerror(err));
    throw invalid_argument(string(what));
}

// Only one kvstore can be created per process -- the DBI is shared. Used to enforce that we never create
// multiple simultaneous kvstores.
// From the docs for mdb_dbi_open:
// > This function must not be called from multiple concurrent transactions in the same process. A transaction that
// > uses this function must finish (either commit or abort) before any other transaction in the process may use
// > this function.
// http://www.lmdb.tech/doc/group__mdb.html#gac08cad5b096925642ca359a6d6f0562a
// That function is called in OwnedKeyValueStore.
static atomic<bool> kvstoreInUse = false;

// Callback to `mdb_reader_list`. `msg` is the lock table contents in human-readable form, `ctx` is a `string*` passed
// in at the `mdb_reader_list` callsite. It copies `msg` into `ctx` to pass the lock table back to the callsite, and
// returns 0 to indicate success.
int mdbReaderListCallback(const char *msg, void *ctx) {
    string *output = (string *)ctx;
    *output = string(msg);
    return 0;
}

bool hasNoOutstandingReaders(MDB_env *env) {
    string ctxString;
    /*
     * LMDB only has two APIs to grok the reader list
     * (http://www.lmdb.tech/doc/group__mdb.html#ga8550000cd0501a44f57ee6dff0188744)
     *
     * - `mdb_reader_check`, which checks for stale transactions. Unfortunately a reader isn't stale if the owning
     * thread is still alive, and LMDB seems to clean up stale transactions automatically somehow prior to me calling
     * this function, so it doesn't work.
     * - `mdb_reader_list`, which produces a human-readable string containing the lock table contents (used in mdb_stat
     * CLI tool). It accepts a callback.
     *
     * I use the latter, as it's the only way to check if the lock table contains contents when we expect that it
     * doesn't.
     */
    const int err = mdb_reader_list(env, mdbReaderListCallback, &ctxString);
    if (err < 0) {
        throw_mdb_error("Failure checking for stale readers", err);
    }
    return ctxString.find("(no active readers)") != string::npos;
}
} // namespace

KeyValueStore::KeyValueStore(string version, string path, string flavor)
    : version(move(version)), path(move(path)), flavor(move(flavor)), dbState(make_unique<DBState>()) {
    ENFORCE(!this->version.empty());
    bool expected = false;
    if (!kvstoreInUse.compare_exchange_strong(expected, true)) {
        throw_mdb_error("Cannot create two kvstore instances simultaneously.", 0);
    }

    int rc = mdb_env_create(&dbState->env);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_env_set_mapsize(dbState->env, MAX_DB_SIZE_BYTES);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_env_set_maxdbs(dbState->env, 3);
    if (rc != 0) {
        goto fail;
    }
    // _disable_ thread local storage so that the writer thread can close/abort a reader transaction.
    // MDB_NOTLS instructs LMDB to store transactions into the `MDB_txn` objects rather than thread-local storage.
    // It allows read-only transactions to migrate across threads (letting the writer thread clean up reader
    // transactions), but users are expected to manage concurrent access. We already restrict concurrent access by
    // manually maintaining a map from thread to transaction.
    // Avoids MDB_READERS_FULL issues with concurrent Sorbet processes.
    rc = mdb_env_open(dbState->env, this->path.c_str(), MDB_NOTLS, 0664);
    if (rc == ENOENT) {
        fmt::print(stderr, "'{}' does not exist. When using --cache-dir, create the directory before hand.\n",
                   this->path);
        throw EarlyReturnWithCode(1);
    } else if (rc == EACCES) {
        fmt::print(stderr, "No read permissions for '{}'", this->path);
        throw EarlyReturnWithCode(1);
    } else if (rc != 0) {
        goto fail;
    }
    return;
fail:
    throw_mdb_error("failed to create database"sv, rc);
}
KeyValueStore::~KeyValueStore() noexcept(false) {
    mdb_env_close(dbState->env);
    bool expected = true;
    if (!kvstoreInUse.compare_exchange_strong(expected, false)) {
        throw_mdb_error("Cannot create two kvstore instances simultaneously.", 0);
    }
}

void KeyValueStore::enforceNoOutstandingReaders() const {
    ENFORCE(hasNoOutstandingReaders(dbState->env));
}

OwnedKeyValueStore::OwnedKeyValueStore(unique_ptr<KeyValueStore> kvstore)
    : writerId(this_thread::get_id()), kvstore(move(kvstore)), txnState(make_unique<TxnState>()) {
    // Writer thread may have changed; reset the primary transaction.
    refreshMainTransaction();
    {
        if (read(OLD_VERSION_KEY).data != nullptr) { // remove databases that use old(non-string) versioning scheme.
            clear();
        }
        auto dbVersion = readString(VERSION_KEY);
        if (dbVersion != this->kvstore->version) {
            clear();
            writeString(VERSION_KEY, this->kvstore->version);
        }
        return;
    }
}

void OwnedKeyValueStore::abort() const {
    // Note: txn being null indicates that the transaction has already ended, perhaps due to a commit.
    if (txnState->txn == nullptr) {
        return;
    }

    if (writerId != this_thread::get_id()) {
        throw_mdb_error("KeyValueStore can only write from thread that created it"sv, 0);
    }

    for (auto &txn : txnState->readers) {
        mdb_txn_abort(txn.second);
    }
    txnState->readers.clear();
    txnState->txn = nullptr;
    ENFORCE(kvstore != nullptr);
    mdb_close(kvstore->dbState->env, txnState->dbi);
}

int OwnedKeyValueStore::commit() {
    // Note: txn being null indicates that the transaction has already ended, perhaps due to a commit.
    if (txnState->txn == nullptr) {
        return 0;
    }

    if (writerId != this_thread::get_id()) {
        throw_mdb_error("KeyValueStore can only write from thread that created it"sv, 0);
    }

    int rc = 0;
    for (auto &txn : txnState->readers) {
        rc = mdb_txn_commit(txn.second) || rc;
    }
    txnState->readers.clear();
    txnState->txn = nullptr;
    ENFORCE(kvstore != nullptr);
    mdb_close(kvstore->dbState->env, txnState->dbi);
    return rc;
}

OwnedKeyValueStore::~OwnedKeyValueStore() {
    abort();
}

void OwnedKeyValueStore::writeInternal(std::string_view key, void *value, size_t len) {
    if (writerId != this_thread::get_id()) {
        throw_mdb_error("KeyValueStore can only write from thread that created it"sv, 0);
    }

    MDB_val kv;
    MDB_val dv;
    kv.mv_size = key.size();
    kv.mv_data = (void *)key.data();
    dv.mv_size = len;
    dv.mv_data = value;

    int rc = mdb_put(txnState->txn, txnState->dbi, &kv, &dv, 0);
    if (rc != 0) {
        throw_mdb_error("failed write into database"sv, rc);
    }
}

void OwnedKeyValueStore::write(string_view key, const vector<uint8_t> &value) {
    writeInternal(key, (void *)value.data(), value.size());
}

KeyValueStoreValue OwnedKeyValueStore::read(string_view key) const {
    MDB_txn *txn = nullptr;
    int rc = 0;
    {
        absl::ReaderMutexLock lk(&readers_mtx);
        auto fnd = txnState->readers.find(this_thread::get_id());
        if (fnd != txnState->readers.end()) {
            txn = fnd->second;
            ENFORCE(txn != nullptr);
        }
    }
    if (txn == nullptr) {
        absl::WriterMutexLock lk(&readers_mtx);
        auto &txn_store = txnState->readers[this_thread::get_id()];
        ENFORCE(txn_store == nullptr);
        rc = mdb_txn_begin(kvstore->dbState->env, nullptr, MDB_RDONLY, &txn_store);
        txn = txn_store;
    }
    if (rc != 0) {
        throw_mdb_error("failed to create read transaction"sv, rc);
    }

    MDB_val kv;
    kv.mv_size = key.size();
    kv.mv_data = (void *)key.data();
    MDB_val data;
    rc = mdb_get(txn, txnState->dbi, &kv, &data);
    if (rc != 0) {
        if (rc == MDB_NOTFOUND) {
            return {nullptr, 0};
        }
        throw_mdb_error("failed read from the database"sv, rc);
    }
    return {static_cast<uint8_t *>(data.mv_data), data.mv_size};
}

void OwnedKeyValueStore::clear() {
    if (writerId != this_thread::get_id()) {
        throw_mdb_error("KeyValueStore can only write from thread that created it"sv, 0);
    }

    int rc = mdb_drop(txnState->txn, txnState->dbi, 0);
    if (rc != 0) {
        goto fail;
    }
    rc = commit();
    if (rc != 0) {
        goto fail;
    }
    refreshMainTransaction();
    return;
fail:
    throw_mdb_error("failed to clear the database"sv, rc);
}

string_view OwnedKeyValueStore::readString(string_view key) const {
    auto rawData = read(key);
    if (rawData.data == nullptr) {
        return string_view();
    }
    string_view result((const char *)rawData.data, rawData.len);
    return result;
}

void OwnedKeyValueStore::writeString(string_view key, string_view value) {
    writeInternal(key, (void *)value.data(), value.size());
}

void OwnedKeyValueStore::refreshMainTransaction() {
    if (writerId != this_thread::get_id()) {
        throw_mdb_error("KeyValueStore can only write from thread that created it"sv, 0);
    }

    auto &dbState = *kvstore->dbState;
    auto rc = mdb_txn_begin(dbState.env, nullptr, 0, &txnState->txn);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_dbi_open(txnState->txn, kvstore->flavor.c_str(), MDB_CREATE, &txnState->dbi);
    if (rc != 0) {
        goto fail;
    }

    // Per the docs for mdb_dbi_open:
    //
    // The database handle will be private to the current transaction
    // until the transaction is successfully committed. If the
    // transaction is aborted the handle will be closed
    // automatically. After a successful commit the handle will reside
    // in the shared environment, and may be used by other
    // transactions.
    //
    // So we commit immediately to force the dbi into the shared space
    // so that readers can use it, and then re-open the transaction
    // for future writes.
    rc = mdb_txn_commit(txnState->txn);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_txn_begin(dbState.env, nullptr, 0, &txnState->txn);
    if (rc != 0) {
        goto fail;
    }
    {
        absl::WriterMutexLock lk(&readers_mtx);
        txnState->readers[writerId] = txnState->txn;
    }
    return;
fail:
    throw_mdb_error("failed to create transaction"sv, rc);
}

unique_ptr<KeyValueStore> OwnedKeyValueStore::abort(unique_ptr<const OwnedKeyValueStore> ownedKvstore) {
    if (ownedKvstore == nullptr) {
        return nullptr;
    }
    ownedKvstore->abort();
    return move(ownedKvstore->kvstore);
}

unique_ptr<KeyValueStore> OwnedKeyValueStore::bestEffortCommit(spdlog::logger &logger,
                                                               unique_ptr<OwnedKeyValueStore> ownedKvstore) {
    if (ownedKvstore == nullptr) {
        return nullptr;
    }
    Timer timeit("kvstore.bestEffortCommit");
    ownedKvstore->commit();
    return move(ownedKvstore->kvstore);
}

} // namespace sorbet
