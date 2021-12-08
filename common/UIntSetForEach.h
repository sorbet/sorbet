#ifndef SORBET_COMMON_UINTSET_FOR_EACH_H
#define SORBET_COMMON_UINTSET_FOR_EACH_H

#include "common/UIntSet.h"

namespace sorbet {
template <typename F> void UIntSet::forEach(F each) const {
    uint32_t id = 0;
    for (auto entry : _members) {
        uint32_t startIdForNextU4 = id + 32;
        while (entry != 0) {
            uint32_t startIdForNextU1 = id + 8;
            // Process 1 byte at a time so we can check 8 places at once.
            uint32_t byte = entry & 0xFF;
            while (byte != 0) {
                if ((byte & 0x1) == 1) {
                    each(id);
                }
                byte >>= 1;
                id++;
            }
            entry >>= 8;
            id = startIdForNextU1;
        }
        id = startIdForNextU4;
    }
}

} // namespace sorbet

#endif
