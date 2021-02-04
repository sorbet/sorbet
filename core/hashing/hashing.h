#ifndef SORBET_HASHING_HASHING_H
#define SORBET_HASHING_HASHING_H

#include "xxhash.h"
#include <string_view>

namespace sorbet::core {

class Hasher {
    XXH32_state_t *state;

public:
    // Faster way to hash a string in isolation.
    static unsigned int hashString(std::string_view utf8);

    Hasher();
    Hasher(Hasher &&);
    ~Hasher();

    // Disallow copy constructors, as Hasher manually manages the memory of XXH32_state_t.
    Hasher &operator=(const Hasher &) = delete;
    Hasher(const Hasher &) = delete;

    // Hashes a string.
    void mixString(std::string_view utf8);

    // Hashes a 32-bit unsigned integer.
    void mixUint(uint32_t num);

    // Hashes a 64-bit unsigned integer.
    void mixU8(uint64_t num);

    // Returns the resulting hash (digest). Can be called multiple times.
    unsigned int digest() const;

    // Resets the hasher. Allows the hasher to be re-used.
    // Useful when hashing many things in a loop.
    void reset();
};
} // namespace sorbet::core
#endif // SORBET_HASHING_HASHING_H
