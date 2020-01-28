#include "common/kvstore/KeyValueStore.h"

#include <utility>

using namespace std;
namespace sorbet {
struct KeyValueStore::DBState {
};

[[noreturn]] static void throw_mdb_error(string_view what, int err) {
    fmt::print(stderr, "mdb error: {}: {}\n", what, "");
    throw invalid_argument(string(what));
}

KeyValueStore::KeyValueStore(string version, string path, string flavor)
    : path(move(path)), flavor(move(flavor)), writerId(this_thread::get_id()), dbState(make_unique<DBState>()) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
}
KeyValueStore::~KeyValueStore() noexcept(false) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
   }

void KeyValueStore::write(string_view key, const vector<u1> &value) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
   }

u1 *KeyValueStore::read(string_view key) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
    }

void KeyValueStore::clear() {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
    }

string_view KeyValueStore::readString(string_view key) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
    }

void KeyValueStore::writeString(string_view key, string_view value) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
    }

void KeyValueStore::refreshMainTransaction() {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
    }

bool KeyValueStore::commit(unique_ptr<KeyValueStore> k) {
    throw_mdb_error("creating databases isn't supported on emscripten"sv, 0);
   }

} // namespace sorbet
