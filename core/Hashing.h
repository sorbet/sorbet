#ifndef SORBET_HASHING_H
#define SORBET_HASHING_H

#include "core/Names.h"
#include "xxhash.h"

namespace sorbet::core {
static constexpr unsigned int HASH_MULT = 65599; // sdbm
static constexpr unsigned int HASH_MULT2 = 31;   // for names

// TODO: Replace with streaming variant of XXHash.
inline unsigned int mix(unsigned int acc, unsigned int nw) {
    return nw + (acc << 6) + (acc << 16) - acc; // HASH_MULT in faster version
}

inline unsigned int _hash(std::string_view utf8) {
    return XXH32(utf8.data(), utf8.size(), /* seed */ 0);
}
} // namespace sorbet::core
#endif // SORBET_HASHING_H
