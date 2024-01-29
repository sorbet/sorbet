#include "common/kvstore/KeyValueStore.h"
#include "common/EarlyReturnWithCode.h"
#include "common/strings/formatting.h"
#include "common/timers/Timer.h"
#include "lmdb.h"
#include "spdlog/spdlog.h"

// TODO(jez) We do not use lmdb when building for emscripten, so we're guaranteed that the pinned
// LLVM we compile Sorbet with has a libc++ version high enough to use `<filesystem>` (from C++17)
#include <filesystem>

#include <utility>

using namespace std;
namespace sorbet {
constexpr string_view VERSION_KEY = "DB_FORMAT_VERSION"sv;
struct KeyValueStore::DBState {
    MDB_env *env;
};

struct OwnedKeyValueStore::TxnState {
    MDB_dbi dbi;
    MDB_txn *txn;
    UnorderedMap<std::thread::id, MDB_txn *> readers;
};

namespace {

[[noreturn]] void throw_mdb_error(string_view what, int err, string_view path) {
    fmt::print(stderr, "sorbet mdb error: what=\"{}\" mdb_error=\"{}\" cache_path=\"{}\"\n", what, mdb_strerror(err),
               path);
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

bool hasNoOutstandingReaders(MDB_env *env, string_view path) {
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
        throw_mdb_error("Failure checking for stale readers", err, path);
    }
    return ctxString.find("(no active readers)") != string::npos;
}
} // namespace

KeyValueStore::KeyValueStore(shared_ptr<spdlog::logger> logger, string version, string path, string flavor,
                             size_t maxSize)
    : version(move(version)), path(move(path)), flavor(move(flavor)), dbState(make_unique<DBState>()),
      logger(move(logger)) {
    ENFORCE(!this->version.empty());
    bool expected = false;
    if (!kvstoreInUse.compare_exchange_strong(expected, true)) {
        throw_mdb_error("Cannot create two kvstore instances simultaneously.", 0, path);
    }

    int rc = mdb_env_create(&dbState->env);
    if (rc != 0) {
        goto fail;
    }
    rc = mdb_env_set_mapsize(dbState->env, maxSize);
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
        try {
            filesystem::create_directories(this->path);
        } catch (filesystem::filesystem_error &e) {
            fmt::print(stderr,
                       "'{}' does not exist and could not be created. "
                       "When using --cache-dir, create the directory before hand.\n",
                       this->path);
            throw EarlyReturnWithCode(1);
        }
        rc = mdb_env_open(dbState->env, this->path.c_str(), MDB_NOTLS, 0664);
        if (rc != 0) {
            goto fail;
        }
    } else if (rc == ENOTDIR) {
        fmt::print(stderr, "'{}' is not a directory, and so cannot store the Sorbet disk cache.\n", this->path);
        throw EarlyReturnWithCode(1);
    } else if (rc == EACCES) {
        fmt::print(stderr, "No read permissions for '{}'\n", this->path);
        throw EarlyReturnWithCode(1);
    } else if (rc != 0) {
        goto fail;
    }
    return;
fail:
    throw_mdb_error("failed to create database"sv, rc, path);
}
KeyValueStore::~KeyValueStore() noexcept(false) {
    mdb_env_close(dbState->env);
    bool expected = true;
    if (!kvstoreInUse.compare_exchange_strong(expected, false)) {
        throw_mdb_error("Cannot create two kvstore instances simultaneously.", 0, path);
    }
}

void KeyValueStore::enforceNoOutstandingReaders() const {
    ENFORCE(hasNoOutstandingReaders(dbState->env, path));
}

OwnedKeyValueStore::OwnedKeyValueStore(unique_ptr<KeyValueStore> kvstore)
    : writerId(this_thread::get_id()), kvstore(move(kvstore)), txnState(make_unique<TxnState>()) {
    // Writer thread may have changed; reset the primary transaction.
    refreshMainTransaction();
    checkVersions();

    auto dbVersion = readString(VERSION_KEY);
    if (!dbVersion.has_value()) {
        this->kvstore->logger->trace("Writing version into cache: {}", this->kvstore->version);
        writeString(VERSION_KEY, this->kvstore->version);
    } else {
        ENFORCE(dbVersion == this->kvstore->version, "checkVersions should have guaranteed this");
    }
}

void OwnedKeyValueStore::abort() const {
    // Note: txn being null indicates that the transaction has already ended, perhaps due to a commit.
    if (txnState->txn == nullptr) {
        return;
    }

    if (writerId != this_thread::get_id()) {
        throw_mdb_error("KeyValueStore can only write from thread that created it"sv, 0, kvstorePath());
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

    kvstore->logger->trace("OwnedKeyValueStore::commit");

    if (writerId != this_thread::get_id()) {
        throw_mdb_error("KeyValueStore can only write from thread that created it"sv, 0, kvstorePath());
    }

    int rc = 0;
    for (auto &txn : txnState->readers) {
        auto rci = mdb_txn_commit(txn.second);
        if (rc != 0) {
            // Take the first non-zero rc
            rc = rci;
        }
    }
    txnState->readers.clear();
    txnState->txn = nullptr;
    ENFORCE(kvstore != nullptr);
    mdb_close(kvstore->dbState->env, txnState->dbi);
    return rc;
}

std::string_view OwnedKeyValueStore::kvstorePath() const {
    ENFORCE(kvstore != nullptr);
    // This is used in error handling code, so we want to degrade gracefully if the ENFORCE is broken.
    if (kvstore == nullptr) {
        return "";
    } else {
        return kvstore->path;
    }
}

OwnedKeyValueStore::~OwnedKeyValueStore() {
    abort();
}

void OwnedKeyValueStore::writeInternal(std::string_view key, void *value, size_t len) {
    if (writerId != this_thread::get_id()) {
        throw_mdb_error("KeyValueStore can only write from thread that created it"sv, 0, kvstorePath());
    }

    MDB_val kv;
    MDB_val dv;
    kv.mv_size = key.size();
    kv.mv_data = (void *)key.data();
    dv.mv_size = len;
    dv.mv_data = value;

    int rc = mdb_put(txnState->txn, txnState->dbi, &kv, &dv, 0);
    if (rc != 0) {
        throw_mdb_error("failed write into database"sv, rc, kvstorePath());
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
        throw_mdb_error("failed to create read transaction"sv, rc, kvstorePath());
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
        throw_mdb_error("failed read from the database"sv, rc, kvstorePath());
    }
    return {static_cast<uint8_t *>(data.mv_data), data.mv_size};
}

namespace {

vector<string> listFlavors(spdlog::logger &logger, MDB_txn *txn, string_view path) {
    // Open the unnamed database, which lists the names of all the other databases
    MDB_dbi unnamedDBI;
    int rc = mdb_dbi_open(txn, nullptr, 0, &unnamedDBI);
    if (rc != 0) {
        throw_mdb_error("failed to clear the database"sv, rc, path);
    }

    vector<string> flavors;
    MDB_cursor *cursor;
    rc = mdb_cursor_open(txn, unnamedDBI, &cursor);
    if (rc != 0) {
        throw_mdb_error("failed to clear the database"sv, rc, path);
    }

    MDB_val kv;
    MDB_val dv;
    rc = mdb_cursor_get(cursor, &kv, &dv, MDB_FIRST);
    while (rc != MDB_NOTFOUND) {
        flavors.emplace_back(string((char *)kv.mv_data, kv.mv_size));
        logger.trace("OwnedKeyValueStore::checkVersions found flavor: {}", flavors.back());
        rc = mdb_cursor_get(cursor, &kv, &dv, MDB_NEXT);
    }
    if (rc != 0 && rc != MDB_NOTFOUND) {
        throw_mdb_error("failed to clear the database"sv, rc, path);
    }

    mdb_cursor_close(cursor);

    return flavors;
}

} // namespace

void OwnedKeyValueStore::checkVersions() {
    if (writerId != this_thread::get_id()) {
        throw_mdb_error("KeyValueStore can only write from thread that created it"sv, 0, kvstorePath());
    }

    kvstore->logger->trace("OwnedKeyValueStore::checkVersions");

    int rc;
    {
        auto flavors = listFlavors(*kvstore->logger, txnState->txn, kvstorePath());

        for (const auto &flavor : flavors) {
            rc = mdb_dbi_open(txnState->txn, flavor.c_str(), 0, &txnState->dbi);
            if (rc != 0) {
                throw_mdb_error("failed to clear the database"sv, rc, kvstorePath());
            }

            auto dbVersion = readString(VERSION_KEY);
            if (!dbVersion.has_value() || dbVersion != this->kvstore->version) {
                int del = 1;
                kvstore->logger->trace("OwnedKeyValueStore::checkVersions dropping flavor '{}' because {} != {}",
                                       flavor, dbVersion.value_or("nullopt"), this->kvstore->version);
                rc = mdb_drop(txnState->txn, txnState->dbi, del);
                if (rc != 0) {
                    throw_mdb_error("failed to clear the database"sv, rc, kvstorePath());
                }
            }
        }

        rc = commit();
        if (rc != 0) {
            throw_mdb_error("failed to clear the database"sv, rc, kvstorePath());
        }
    }

    refreshMainTransaction();
}

namespace {
size_t allUsedBytes(MDB_stat &stat) {
    return stat.ms_psize * (stat.ms_branch_pages + stat.ms_leaf_pages + stat.ms_overflow_pages);
}

} // namespace

// I got the inspiration for this implementation from this answer:
//
//     https://stackoverflow.com/a/40527056
//
// Unfortunately it seems that there is not a convenient helper function for this built into the LMDB API.
size_t OwnedKeyValueStore::cacheSize() const {
    if (writerId != this_thread::get_id()) {
        // This is mostly for simplicity. This is technically a read-only transaction, and so we
        // could support doing it with txnState->readers[this_thread::get_id()] but that seems like
        // overkill given that we don't need to do so at the moment.
        throw_mdb_error("Can only call cacheSize from main thread"sv, 0, kvstorePath());
    }

    kvstore->logger->trace("OwnedKeyValueStore::cacheSize");
    int rc;
    MDB_stat stat;

    size_t totalBytes = 0;

    rc = mdb_env_stat(kvstore->dbState->env, &stat);
    if (rc != 0) {
        throw_mdb_error("failed to stat the main environment", rc, kvstorePath());
    }

    totalBytes += allUsedBytes(stat);

    // Open the unnamed database, which lists the names of all the other databases
    auto flavors = listFlavors(*kvstore->logger, txnState->txn, kvstorePath());

    for (const auto &flavor : flavors) {
        MDB_dbi flavorDBI;
        rc = mdb_dbi_open(txnState->txn, flavor.c_str(), 0, &flavorDBI);
        if (rc != 0) {
            auto msg = fmt::format("failed to open cache flavor {}", flavor);
            throw_mdb_error(msg, rc, kvstorePath());
        }

        rc = mdb_stat(txnState->txn, flavorDBI, &stat);
        if (rc != 0) {
            auto msg = fmt::format("failed to stat the cache flavor {}", flavor);
            throw_mdb_error(msg, rc, kvstorePath());
        }

        totalBytes += allUsedBytes(stat);
    }

    return totalBytes;
}

optional<string_view> OwnedKeyValueStore::readString(string_view key) const {
    auto rawData = read(key);
    if (rawData.data == nullptr) {
        return nullopt;
    }
    string_view result((const char *)rawData.data, rawData.len);
    return result;
}

void OwnedKeyValueStore::writeString(string_view key, string_view value) {
    writeInternal(key, (void *)value.data(), value.size());
}

void OwnedKeyValueStore::refreshMainTransaction() {
    kvstore->logger->trace("OwnedKeyValueStore::refreshMainTransaction");

    if (writerId != this_thread::get_id()) {
        throw_mdb_error("KeyValueStore can only write from thread that created it"sv, 0, kvstorePath());
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
    throw_mdb_error("failed to create transaction"sv, rc, kvstorePath());
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
    Timer timeit(logger, "kvstore.bestEffortCommit");
    ownedKvstore->commit();
    return move(ownedKvstore->kvstore);
}

} // namespace sorbet
