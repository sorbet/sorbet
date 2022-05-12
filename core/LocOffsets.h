#ifndef SORBET_CORE_LOCOFFSETS_H
#define SORBET_CORE_LOCOFFSETS_H

#include "common/common.h"
#include "core/FileRef.h"

namespace sorbet::core {
class GlobalState;
class Context;
class MutableContext;

constexpr int INVALID_POS_LOC = 0xfffffff;
struct LocOffsets {
    uint32_t beginLoc = INVALID_POS_LOC;
    uint32_t endLoc = INVALID_POS_LOC;
    uint32_t beginPos() const {
        return beginLoc;
    };

    uint32_t endPos() const {
        return endLoc;
    }
    bool exists() const {
        return endLoc != INVALID_POS_LOC && beginLoc != INVALID_POS_LOC;
    }
    bool empty() const {
        ENFORCE_NO_TIMER(exists());
        return beginLoc == endLoc;
    }
    static LocOffsets none() {
        return LocOffsets{INVALID_POS_LOC, INVALID_POS_LOC};
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

    std::string showRaw(const Context ctx) const;
    std::string showRaw(const MutableContext ctx) const;
    std::string showRaw(const GlobalState &gs, const FileRef file) const;
    std::string showRaw() const;

    bool operator==(const LocOffsets &rhs) const;

    bool operator!=(const LocOffsets &rhs) const;
};
CheckSize(LocOffsets, 8, 4);

} // namespace sorbet::core

#endif // SORBET_CORE_LOCOFFSETS_H
