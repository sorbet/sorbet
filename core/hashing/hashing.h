#ifndef SORBET_HASHING_H
#define SORBET_HASHING_H

#include <string_view>

namespace sorbet::core {
static constexpr unsigned int HASH_MULT = 65599; // sdbm
static constexpr unsigned int HASH_MULT2 = 31;   // for names

inline unsigned int mix(unsigned int acc, unsigned int nw) {
    return nw + (acc << 6) + (acc << 16) - acc; // HASH_MULT in faster version
}

unsigned int _hash(std::string_view utf8);

unsigned int _hash(uint64_t num1);

unsigned int _hash(uint32_t num1, uint32_t num2, uint32_t num3);

} // namespace sorbet::core
#endif // SORBET_HASHING_H
