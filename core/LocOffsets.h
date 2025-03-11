#ifndef SORBET_CORE_LOCOFFSETS_H
#define SORBET_CORE_LOCOFFSETS_H

#include "common/common.h"
#include "core/FileRef.h"

namespace sorbet::core {
class GlobalState;
class Context;
class MutableContext;

struct LocOffsets {
    using POS = uint32_t;

    constexpr static POS INVALID_POS_LOC = std::numeric_limits<POS>::max();

    POS beginLoc = INVALID_POS_LOC;
    POS endLoc = INVALID_POS_LOC;

    POS beginPos() const {
        return beginLoc;
    };

    POS endPos() const {
        return endLoc;
    }

    bool exists() const {
        return endLoc != INVALID_POS_LOC && beginLoc != INVALID_POS_LOC;
    }

    bool empty() const {
        ENFORCE_NO_TIMER(exists());
        return beginLoc == endLoc;
    }

    LocOffsets() : beginLoc{INVALID_POS_LOC}, endLoc{INVALID_POS_LOC} {}

    LocOffsets(uint32_t beginLoc, uint32_t endLoc) : beginLoc{beginLoc}, endLoc{endLoc} {
        ENFORCE_NO_TIMER(this->exists());
        ENFORCE_NO_TIMER(beginLoc <= endLoc);
    }

    static LocOffsets none() {
        return LocOffsets();
    }

    LocOffsets join(LocOffsets other) const;

    // For a given LocOffsets, returns a zero-length version that starts at the same location.
    LocOffsets copyWithZeroLength() const {
        return LocOffsets{beginPos(), beginPos()};
    }

    // As above, but returns a zero-length version that starts at the end of the location.
    LocOffsets copyEndWithZeroLength() const {
        return LocOffsets{endPos(), endPos()};
    }

    bool contains(const LocOffsets &other) const;

    std::string showRaw(const Context ctx) const;
    std::string showRaw(const MutableContext ctx) const;
    std::string showRaw(const GlobalState &gs, const FileRef file) const;
    std::string showRaw() const;

    bool operator==(const LocOffsets &rhs) const;

    bool operator!=(const LocOffsets &rhs) const;
};
CheckSize(LocOffsets, 8, 4);

template <typename H> H AbslHashValue(H h, const LocOffsets &m) {
    return H::combine(std::move(h), m.beginLoc, m.endLoc);
}

} // namespace sorbet::core

#endif // SORBET_CORE_LOCOFFSETS_H
