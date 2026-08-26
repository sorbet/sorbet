#ifndef SORBET_COMMON_UINTSET_FOR_EACH_H
#define SORBET_COMMON_UINTSET_FOR_EACH_H

#include "common/UIntSet.h"

namespace sorbet {
template <typename F> void UIntSet::forEach(F each) const {
    uint32_t base = 0;
    for (auto entry : _members) {
        while (entry != 0) {
            uint32_t bit = __builtin_ctz(entry);
            each(base + bit);
            entry &= entry - 1; // clear lowest set bit
        }
        base += 32;
    }
}

} // namespace sorbet

#endif
