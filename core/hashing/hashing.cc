// Must go before xxhash include. Makes a noticeable performance improvement (~10%).
#define XXH_INLINE_ALL

#include "core/hashing/hashing.h"
#include "xxhash.h"

namespace sorbet::core {
unsigned int _hash(std::string_view utf8) {
    return XXH32(utf8.data(), utf8.size(), /* seed */ 0);
}

unsigned int _hash(uint64_t num1) {
    return XXH32(&num1, sizeof(num1), /* seed */ 0);
}

unsigned int _hash(uint32_t num1, uint32_t num2, uint32_t num3) {
    uint32_t data[3] = {num1, num2, num3};
    return XXH32(data, sizeof(uint32_t) * 3, /* seed */ 0);
}

} // namespace sorbet::core
