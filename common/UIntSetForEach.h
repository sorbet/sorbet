#ifndef SORBET_COMMON_UINTSET_FOR_EACH_H
#define SORBET_COMMON_UINTSET_FOR_EACH_H

#include "common/UIntSet.h"

namespace sorbet {
template <typename F> void UIntSet::forEach(F each) const {
    u4 id = 0;
    for (auto entry : _members) {
        u4 startIdForNextU4 = id + 32;
        while (entry != 0) {
            // Shift until entry is 0.
            if ((entry & 0x1) == 1) {
                each(id);
            }
            entry >>= 1;
            id++;
        }
        id = startIdForNextU4;
    }
}

} // namespace sorbet

#endif