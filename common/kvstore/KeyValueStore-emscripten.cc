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

KeyValueStore::KeyValueStore(string version, string path, string flavor)
    : version(move(version)), path(move(path)), flavor(move(flavor)), dbState(make_unique<DBState>()) {
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

void OwnedKeyValueStore::write(string_view key, const vector<u1> &value) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

u1 *OwnedKeyValueStore::read(string_view key) const {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

void OwnedKeyValueStore::clear() {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}

string_view OwnedKeyValueStore::readString(string_view key) const {
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
