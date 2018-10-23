#include "common/kvstore/KeyValueStore.h"

#include <utility>
using namespace std;
namespace sorbet {
constexpr string_view OLD_VERSION_KEY = "VERSION"sv;
constexpr string_view VERSION_KEY = "DB_FORMAT_VERSION"sv;
constexpr size_t MAX_DB_SIZE_BYTES =
    4L * 1024 * 1024 * 1024; // 4G. This is both maximum fs db size and max virtual memory usage.
KeyValueStore::KeyValueStore(string version, string path) : path(move(path)), writerId(this_thread::get_id()) {
    int rc;
    rc = mdb_env_create(&env);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_env_set_mapsize(env, MAX_DB_SIZE_BYTES);
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
    throw invalid_argument("failed to create database");
}
KeyValueStore::~KeyValueStore() noexcept(false) {
    if (commited) {
        return;
    }

    mdb_txn_abort(txn);
    mdb_close(env, dbi);
    mdb_env_close(env);
    return;
}

void KeyValueStore::write(string_view key, vector<u1> value) {
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
    MDB_txn *txn;
    int rc = 0;
    {
        unique_lock<mutex> lk(readersLock);
        auto &txn_store = readers[this_thread::get_id()];
        if (!txn_store) {
            rc = mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn_store);
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
        goto fail;
    }
    rc = mdb_txn_commit(txn);
    if (rc != 0) {
        goto fail;
    }
    refreshMainTransaction();
    return;
fail:
    throw invalid_argument("failed to clear the database");
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

void KeyValueStore::writeString(string_view key, string value) {
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
    rc = mdb_open(txn, nullptr, MDB_CREATE, &dbi);
    if (rc != 0) {
        goto fail;
    }
    readers[writerId] = txn;
    return;
fail:
    throw invalid_argument("failed to create transaction");
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
