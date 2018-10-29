#ifndef RUBY_TYPER_CRYPTO_HASHING_H
#define RUBY_TYPER_CRYPTO_HASHING_H

#include "common/common.h"
extern "C" {
#ifndef EMSCRIPTEN
#include "blake2.h"
#else
#include "ref/blake2.h"
#endif
};

namespace sorbet::crypto_hashing {
inline std::array<u1, 64> hash64(std::string_view data) {
    static_assert(BLAKE2B_OUTBYTES == 64);
    std::array<u1, 64> res;

#ifndef EMSCRIPTEN
    int err = blake2b(&res[0], data.begin(), nullptr, std::size(res), data.size(), 0);
#else
    // it has different order of arguments \facepalm
    int err = blake2b(&res[0], std::size(res), data.begin(), data.size(), nullptr, 0);
#endif
    ENFORCE(err == 0);
    return res;
};
} // namespace sorbet::crypto_hashing
#endif // RUBY_TYPER_CRYPTO_HASHING_H
