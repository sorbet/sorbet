#include "common/kvstore/KeyValueStore.h"

#include <utility>

using namespace std;
namespace sorbet {
struct KeyValueStore::DBState {};
struct OwnedKeyValueStore::TxnState {};

[[noreturn]] static void throw_mdb_error(string_view what, int err) {
    fmt::print(stderr, "mdb error: {}: {}\n", what, "");
    throw invalid_argument(string(what));
}

KeyValueStore::KeyValueStore(shared_ptr<spdlog::logger> logger, string version, string path, string flavor,
                             size_t maxSize)
    : version(move(version)), path(move(path)), flavor(move(flavor)), dbState(make_unique<DBState>()), logger(logger) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}
KeyValueStore::~KeyValueStore() noexcept(false) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

OwnedKeyValueStore::OwnedKeyValueStore(unique_ptr<KeyValueStore> kvstore) : kvstore(move(kvstore)) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

OwnedKeyValueStore::~OwnedKeyValueStore() {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

int OwnedKeyValueStore::commit() {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

void OwnedKeyValueStore::abort() const {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

void OwnedKeyValueStore::writeInternal(string_view key, void *data, size_t len) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

void OwnedKeyValueStore::write(string_view key, const vector<uint8_t> &value) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

KeyValueStoreValue OwnedKeyValueStore::read(string_view key) const {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

void OwnedKeyValueStore::checkVersions() {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

optional<string_view> OwnedKeyValueStore::readString(string_view key) const {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

void OwnedKeyValueStore::writeString(string_view key, string_view value) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

void OwnedKeyValueStore::refreshMainTransaction() {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

unique_ptr<KeyValueStore> OwnedKeyValueStore::bestEffortCommit(spdlog::logger &logger,
                                                               unique_ptr<OwnedKeyValueStore> k) {
    return nullptr;
}

unique_ptr<KeyValueStore> OwnedKeyValueStore::abort(unique_ptr<const OwnedKeyValueStore> ownedKvstore) {
    return nullptr;
}

} // namespace sorbet
