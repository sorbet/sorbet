#ifndef SORBET_HASHING_H
#define SORBET_HASHING_H

#include "core/Names.h"

namespace sorbet {
namespace core {
static constexpr unsigned int HASH_MULT = 65599; // sdbm
static constexpr unsigned int HASH_MULT2 = 31;   // for names

inline unsigned int mix(unsigned int acc, unsigned int nw) {
    return nw + (acc << 6) + (acc << 16) - acc; // HASH_MULT in faster version
}

inline unsigned int _hash_mix_unique(unsigned int hash1, NameKind nk, unsigned int hash2, unsigned int hash3) {
    return mix(mix(hash2, hash1), hash3) * HASH_MULT2 + _NameKind2Id_UNIQUE(nk);
}

inline unsigned int _hash_mix_constant(NameKind nk, unsigned int id) {
    return id * HASH_MULT2 + _NameKind2Id_CONSTANT(nk);
}

inline unsigned int _hash(std::string_view utf8) {
    // TODO: replace with http://www.sanmayce.com/Fastest_Hash/, see https://www.strchr.com/hash_functions
    // and https://github.com/rurban/smhasher
    auto end = utf8.end();
    unsigned int res = 0;
    auto it = utf8.begin();
#pragma clang loop unroll(enable)
    for (; it != end; it++) {
        res = mix(res, *it - '!'); // "!" is the first printable letter in ASCII.
        // This will help Latin1 but may harm utf8 multibyte
    }
    return res * HASH_MULT2 + _NameKind2Id_UTF8(UTF8);
}
} // namespace core
} // namespace sorbet
#endif // SORBET_HASHING_H
