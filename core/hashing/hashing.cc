// Must go before xxhash include. Makes a noticeable performance improvement (~10%).
#define XXH_INLINE_ALL

#include "core/hashing/hashing.h"
#include "xxhash.h"

namespace sorbet::core {
unsigned int _hash(std::string_view utf8) {
    return XXH32(utf8.data(), utf8.size(), /* seed */ 0);
}

} // namespace sorbet::core
