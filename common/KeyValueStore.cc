#include "KeyValueStore.h"
using namespace std;
namespace ruby_typer {
const string VERSION_KEY = "VERSION";
const size_t MAX_DB_SIZE_BYTES =
    4L * 1024 * 1024 * 1024; // 4G. This is both maximum fs db size and max virtual memory usage.
KeyValueStore::KeyValueStore(int version, string path) : path(path), writerId(this_thread::get_id()) {
    int rc;
    rc = mdb_env_create(&env);
    if (rc != 0)
        goto fail;
    rc = mdb_env_set_mapsize(env, MAX_DB_SIZE_BYTES);
    if (rc != 0)
        goto fail;
    rc = mdb_env_open(env, this->path.c_str(), 0, 0664);
    if (rc != 0)
        goto fail;
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    if (rc != 0)
        goto fail;
    rc = mdb_open(txn, NULL, MDB_CREATE, &dbi);
    if (rc != 0)
        goto fail;
    readers[writerId] = txn;
    {
        auto dbVersionPtr = read(VERSION_KEY);
        int dbVersion;
        if (dbVersionPtr) {
            memcpy(&dbVersion, dbVersionPtr, sizeof(dbVersion));
        }
        if (!dbVersionPtr || dbVersion != version) {
            if (dbVersionPtr) {
                clear();
            }
            vector<u1> versionVec(sizeof(int) / sizeof(u1));
            memcpy(versionVec.data(), &version, sizeof(version));
            write(VERSION_KEY, move(versionVec));
        }
        return;
    }
fail:
    throw invalid_argument("failed to create database");
}
KeyValueStore::~KeyValueStore() noexcept(false) {
    int rc;
    rc = mdb_txn_commit(txn);
    if (rc != 0)
        goto fail;
    mdb_close(env, dbi);
    mdb_env_close(env);
    return;

fail:
    throw invalid_argument("failed to close the database");
}

void KeyValueStore::write(const absl::string_view key, vector<u1> value) {
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

u1 *KeyValueStore::read(const absl::string_view key) {
    MDB_txn *txn;
    int rc = 0;
    {
        unique_lock<mutex> lk(readersLock);
        auto &txn_store = readers[this_thread::get_id()];
        if (!txn_store) {
            rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn_store);
        }
        txn = txn_store;
    }
    if (rc != 0) {
        throw invalid_argument("failed to create read transaction");
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
        throw invalid_argument("failed read from the database");
    }
    return (u1 *)data.mv_data;
}

void KeyValueStore::clear() {
    if (writerId != this_thread::get_id()) {
        throw invalid_argument("KeyValueStore can only write from thread that created it");
    }
    int rc = mdb_drop(txn, dbi, 0);
    if (rc != 0) {
        throw invalid_argument("failed to clear the database");
    }
}

} // namespace ruby_typer