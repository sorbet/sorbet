#include "common/UIntSet.h"

namespace sorbet {

// Integer divide by 32 and round up to determine how many u4s we need.
UIntSet::UIntSet(u4 size) : _members((size + 31) / 32, 0) {}

void UIntSet::add(u4 item) {
    u4 memberIndex = item >> 5;
    ENFORCE_NO_TIMER(memberIndex < _members.size());
    u4 mask = 1 << (item & 0x1F);
    _members[memberIndex] |= mask;
}

bool UIntSet::contains(u4 item) const {
    u4 memberIndex = item >> 5;
    ENFORCE_NO_TIMER(memberIndex < _members.size());
    u4 mask = 1 << (item & 0x1F);
    return (_members[memberIndex] & mask) > 0;
}

void UIntSet::remove(const UIntSet &set) {
    ENFORCE_NO_TIMER(_members.size() == set._members.size());
    for (int i = 0; i < _members.size(); i++) {
        _members[i] &= ~set._members[i];
    }
}

void UIntSet::add(const UIntSet &set) {
    ENFORCE_NO_TIMER(_members.size() == set._members.size());
    for (int i = 0; i < _members.size(); i++) {
        _members[i] |= set._members[i];
    }
}

bool UIntSet::empty() const {
    for (auto items : _members) {
        if (items > 0) {
            return false;
        }
    }
    return true;
}

void UIntSet::intersection(const UIntSet &set) {
    ENFORCE_NO_TIMER(_members.size() == set._members.size());
    for (int i = 0; i < _members.size(); i++) {
        _members[i] &= set._members[i];
    }
}

size_t UIntSet::size() const {
    size_t sz = 0;
    for (auto member : _members) {
        sz += __builtin_popcount(member);
    }
    return sz;
}

}; // namespace sorbet