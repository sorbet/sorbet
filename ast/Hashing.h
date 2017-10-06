//
// Created by Dmitry Petrashko on 10/6/17.
//

#ifndef SRUBY_HASHING_H
#define SRUBY_HASHING_H

#include "Names.h"

namespace sruby {
namespace ast {
static constexpr unsigned int HASH_MULT = 65599; // sdbm
static constexpr unsigned int HASH_MULT2 = 31;   // for names

inline unsigned int mix(unsigned int acc, unsigned int nw) {
    return nw + (acc << 6) + (acc << 16) - acc; // HASH_MULT in faster version
}

inline unsigned int _hash_mix_unique(unsigned int hash1, NameKind nk, unsigned int hash2, unsigned int hash3) {
    return mix(mix(hash2, hash1), hash3) * HASH_MULT2 + _NameKind2Id_UNIQUE(nk);
}

inline unsigned int _hash(UTF8Desc utf8) {
    // TODO: replace with http://www.sanmayce.com/Fastest_Hash/, see https://www.strchr.com/hash_functions
    // and https://github.com/rurban/smhasher
    auto *end = utf8.from + utf8.to;
    unsigned int res = 0;
    auto it = utf8.from;
#pragma clang loop unroll(enable)
    for (; it != end; it++) {
        res = mix(res, *it - '!'); // "!" is the first printable letter in ASCII.
        // This will help Latin1 but may harm utf8 multibyte
    }
    return res * HASH_MULT2 + _NameKind2Id_UTF8(UTF8);
}
} // namespace ast
} // namespace sruby
#endif // SRUBY_HASHING_H
