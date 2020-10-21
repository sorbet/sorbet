#ifndef SORBET_COMMON_UINTSET_H
#define SORBET_COMMON_UINTSET_H

#include "common/common.h"

namespace sorbet {
class UIntSet final {
    // Most uses require < 128 members.
    InlinedVector<u4, 4> _members;

public:
    // Creates a new set with the given capacity as the maximum acceptable item ID.
    UIntSet(u4 capacity);

    // Add number to set.
    void add(u4 item);

    // Remove number from set.
    void remove(u4 item);

    // Returns true if the set contains the given item.
    bool contains(u4 item) const;

    // Defined in UIntSetForEach.h so that it's only included where it is called.
    template <typename F> void forEach(F each) const;

    // Add items in `set` to this set. Sets must have the same size.
    void add(const UIntSet &set);

    // Remove items in `set` from this set. Sets must have the same size.
    void remove(const UIntSet &set);

    // Mutates the set to contain the intersection of this set and the passed-in set.
    void intersect(const UIntSet &set);

    // Returns true if the set is empty.
    bool empty() const;

    // Returns the number of elements in the set.
    size_t size() const;
};

} // namespace sorbet

#endif
