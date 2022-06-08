#ifndef SORBET_COMMON_UINTSET_H
#define SORBET_COMMON_UINTSET_H

#include "common/common.h"

namespace sorbet {
class UIntSet final {
    // Most uses require < 128 members.
    InlinedVector<uint32_t, 4> _members;

public:
    // Creates a new set with the given capacity as the maximum acceptable item ID.
    UIntSet(uint32_t capacity);

    // Removes all elements from the set.
    void clear();

    // Add number to set.
    void add(uint32_t item);

    // Remove number from set.
    void remove(uint32_t item);

    // Returns true if the set contains the given item.
    bool contains(uint32_t item) const;

    // Defined in UIntSetForEach.h so that it's only included where it is called.
    template <typename F> void forEach(F each) const;

    // Add items in `set` to this set. Sets must have the same size.
    void add(const UIntSet &set);

    // Add items in the given sets to this set. Sets must have the same size.
    void add(const UIntSet &a, const UIntSet &b);

    // Remove items in `set` from this set. Sets must have the same size.
    void remove(const UIntSet &set);

    // Mutates the set to contain the intersection of this set and the passed-in set.
    void intersect(const UIntSet &set);

    // Mutates the set to contain the union of the given sets.
    void overwriteWithUnion(const UIntSet &a, const UIntSet &b);

    // Returns true if the set is empty.
    bool empty() const;

    // Returns the number of elements in the set.
    size_t size() const;
};

} // namespace sorbet

#endif
