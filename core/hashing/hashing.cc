#define XXH_INLINE_ALL

#include "core/hashing/hashing.h"

namespace sorbet::core {
Hasher::Hasher() : state(XXH32_createState()) {
    reset();
}

Hasher::Hasher(Hasher &&other) : state(other.state) {
    other.state = nullptr;
}

Hasher::~Hasher() {
    if (state != nullptr) {
        XXH32_freeState(state);
    }
}

void Hasher::reset() {
    XXH32_reset(state, /* seed */ 0);
}

unsigned int Hasher::hashString(std::string_view utf8) {
    return XXH32(utf8.data(), utf8.size(), /* seed */ 0);
}

void Hasher::mixString(std::string_view utf8) {
    XXH32_update(state, utf8.data(), utf8.size());
}

void Hasher::mixUint(unsigned int num) {
    XXH32_update(state, &num, sizeof(num));
}

void Hasher::mixU8(uint64_t num) {
    XXH32_update(state, &num, sizeof(num));
}

unsigned int Hasher::digest() const {
    return XXH32_digest(state);
}

} // namespace sorbet::core
