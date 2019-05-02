#include "common/kvstore/KeyValueStore.h"

#include <utility>

using namespace std;
namespace sorbet {
constexpr string_view OLD_VERSION_KEY = "VERSION"sv;
constexpr string_view VERSION_KEY = "DB_FORMAT_VERSION"sv;
constexpr size_t MAX_DB_SIZE_BYTES =
    1L * 1024 * 1024 * 1024; // 1G. This is both maximum fs db size and max virtual memory usage.

static void throw_mdb_error(string_view what, int err) {
    fmt::print(stderr, "mdb error: {}: {}\n", what, mdb_strerror(err));
    throw invalid_argument(string(what));
}

KeyValueStore::KeyValueStore(string version, string path, string flavor)
    : path(move(path)), flavor(move(flavor)), writerId(this_thread::get_id()) {
    int rc;
    rc = mdb_env_create(&env);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_env_set_mapsize(env, MAX_DB_SIZE_BYTES);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_env_set_maxdbs(env, 3);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_env_open(env, this->path.c_str(), 0, 0664);
    if (rc != 0) {
        goto fail;
    }
    refreshMainTransaction();
    {
        if (read(OLD_VERSION_KEY)) { // remove databases that use old(non-string) versioning scheme.
            clear();
        }
        auto dbVersion = readString(VERSION_KEY);
        if (dbVersion != version) {
            clear();
            writeString(VERSION_KEY, version);
        }
        return;
    }
fail:
    throw_mdb_error("failed to create database"sv, rc);
}
KeyValueStore::~KeyValueStore() noexcept(false) {
    if (commited) {
        return;
    }

    mdb_txn_abort(txn);
    mdb_close(env, dbi);
    mdb_env_close(env);
}

void KeyValueStore::write(string_view key, const vector<u1> &value) {
    if (writerId != this_thread::get_id()) {
        throw invalid_argument("KeyValueStore can only write from thread that created it");
    }
    MDB_val kv;
    MDB_val dv;
    kv.mv_size = key.size();
    kv.mv_data = (void *)key.data();
    dv.mv_size = value.size();
    dv.mv_data = (void *)value.data();

    int rc;
    rc = mdb_put(txn, dbi, &kv, &dv, 0);
    if (rc != 0) {
        throw invalid_argument("failed write into database");
    }
}

u1 *KeyValueStore::read(string_view key) {
    MDB_txn *txn = nullptr;
    int rc = 0;
    {
        absl::ReaderMutexLock lk(&readers_mtx);
        auto fnd = readers.find(this_thread::get_id());
        if (fnd != readers.end()) {
            txn = fnd->second;
            ENFORCE(txn != nullptr);
        }
    }
    if (txn == nullptr) {
        absl::WriterMutexLock lk(&readers_mtx);
        auto &txn_store = readers[this_thread::get_id()];
        ENFORCE(txn_store == nullptr);
        rc = mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn_store);
        txn = txn_store;
    }
    if (rc != 0) {
        throw_mdb_error("failed to create read transaction"sv, rc);
    }

    MDB_val kv;
    kv.mv_size = key.size();
    kv.mv_data = (void *)key.data();
    MDB_val data;
    rc = mdb_get(txn, dbi, &kv, &data);
    if (rc != 0) {
        if (rc == MDB_NOTFOUND) {
            return nullptr;
        }
        throw_mdb_error("failed read from the database"sv, rc);
    }
    return (u1 *)data.mv_data;
}

void KeyValueStore::clear() {
    if (writerId != this_thread::get_id()) {
        throw invalid_argument("KeyValueStore can only write from thread that created it");
    }
    int rc = mdb_drop(txn, dbi, 0);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_txn_commit(txn);
    if (rc != 0) {
        goto fail;
    }
    refreshMainTransaction();
    return;
fail:
    throw_mdb_error("failed to clear the database"sv, rc);
}

string_view KeyValueStore::readString(string_view key) {
    auto rawData = read(key);
    if (!rawData) {
        return string_view();
    }
    size_t sz;
    memcpy(&sz, rawData, sizeof(sz));
    string_view result(((const char *)rawData) + sizeof(sz), sz);
    return result;
}

void KeyValueStore::writeString(string_view key, string_view value) {
    vector<u1> rawData(value.size() + sizeof(size_t));
    size_t sz = value.size();
    memcpy(rawData.data(), &sz, sizeof(sz));
    memcpy(rawData.data() + sizeof(sz), value.data(), sz);
    write(key, move(rawData));
}

void KeyValueStore::refreshMainTransaction() {
    if (writerId != this_thread::get_id()) {
        throw invalid_argument("KeyValueStore can only write from thread that created it");
    }
    auto rc = mdb_txn_begin(env, nullptr, 0, &txn);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_dbi_open(txn, flavor.c_str(), MDB_CREATE, &dbi);
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
    rc = mdb_txn_commit(txn);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_txn_begin(env, nullptr, 0, &txn);
    if (rc != 0) {
        goto fail;
    }
    {
        absl::WriterMutexLock lk(&readers_mtx);
        readers[writerId] = txn;
    }
    return;
fail:
    throw_mdb_error("failed to create transaction"sv, rc);
}

bool KeyValueStore::commit(unique_ptr<KeyValueStore> k) {
    int rc;
    k->commited = true;
    rc = mdb_txn_commit(k->txn);

    if (rc != 0) {
        return false;
    }
    mdb_close(k->env, k->dbi);
    mdb_env_close(k->env);
    return true;
}

} // namespace sorbet
