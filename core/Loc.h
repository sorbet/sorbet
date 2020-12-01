#ifndef SORBET_AST_LOC_H
#define SORBET_AST_LOC_H

#include "Files.h"

namespace sorbet::core {
namespace serialize {
class SerializerImpl;
}
class GlobalState;
class Context;
class MutableContext;

constexpr int INVALID_POS_LOC = 0xffffff;
struct LocOffsets {
    u4 beginLoc;
    u4 endLoc;
    u4 beginPos() const {
        return beginLoc;
    };

    u4 endPos() const {
        return endLoc;
    }
    bool exists() const {
        return endLoc != INVALID_POS_LOC && beginLoc != INVALID_POS_LOC;
    }
    static LocOffsets none() {
        return LocOffsets{INVALID_POS_LOC, INVALID_POS_LOC};
    }
    LocOffsets join(LocOffsets other) const;
    // For a given Loc, returns a zero-length version that starts at the same location.
    LocOffsets copyWithZeroLength() const {
        return LocOffsets{beginPos(), beginPos()};
    }

    std::string showRaw(const Context ctx) const;
    std::string showRaw(const MutableContext ctx) const;
};
CheckSize(LocOffsets, 8, 4);

class Loc final {
    struct {
        LocOffsets offsets;
        core::FileRef fileRef;
    } storage;
    template <typename H> friend H AbslHashValue(H h, const Loc &m);
    friend class sorbet::core::serialize::SerializerImpl;

    void setFile(core::FileRef file) {
        storage.fileRef = file;
    }

public:
    static Loc none(FileRef file = FileRef()) {
        return Loc{file, LocOffsets::none()};
    }

    bool exists() const {
        return storage.fileRef != 0 && storage.offsets.exists();
    }

    Loc join(Loc other) const;

    u4 beginPos() const {
        return storage.offsets.beginLoc;
    };

    u4 endPos() const {
        return storage.offsets.endLoc;
    }
    const LocOffsets &offsets() const {
        return storage.offsets;
    }

    FileRef file() const {
        return storage.fileRef;
    }

    bool isTombStoned(const GlobalState &gs) const {
        auto f = file();
        if (!f.exists()) {
            return false;
        } else {
            return file().data(gs).sourceType == File::Type::TombStone;
        }
    }

    inline Loc(FileRef file, u4 begin, u4 end) : storage{{begin, end}, file} {
        ENFORCE(begin <= INVALID_POS_LOC);
        ENFORCE(end <= INVALID_POS_LOC);
        ENFORCE(begin <= end);
    }

    inline Loc(FileRef file, LocOffsets offsets) : Loc(file, offsets.beginPos(), offsets.endPos()){};

    Loc() : Loc(0, LocOffsets::none()){};

    Loc &operator=(const Loc &rhs) = default;
    Loc &operator=(Loc &&rhs) = default;
    Loc(const Loc &rhs) = default;
    Loc(Loc &&rhs) = default;

    struct Detail {
        u4 line, column;
    };

    bool contains(const Loc &other) const;
    std::pair<Detail, Detail> position(const GlobalState &gs) const;
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string toString(const GlobalState &gs) const {
        return toStringWithTabs(gs);
    }
    std::string showRaw(const GlobalState &gs) const;
    std::string filePosToString(const GlobalState &gs, bool showFull = false) const;
    std::string source(const GlobalState &gs) const;

    bool operator==(const Loc &rhs) const;

    bool operator!=(const Loc &rhs) const;
    static std::optional<u4> pos2Offset(const File &file, Detail pos);
    static Detail offset2Pos(const File &file, u4 off);
    static std::optional<Loc> fromDetails(const GlobalState &gs, FileRef fileRef, Detail begin, Detail end);

    // For a given Loc, returns
    //
    // - the Loc corresponding to the first non-whitespace character on this line, and
    // - how many characters of the start of this line are whitespace.
    //
    std::pair<Loc, u4> findStartOfLine(const GlobalState &gs) const;
};
CheckSize(Loc, 12, 4);

template <typename H> H AbslHashValue(H h, const Loc &m) {
    return H::combine(std::move(h), m.storage.offsets.beginLoc, m.storage.offsets.endLoc, m.storage.fileRef.id());
}
} // namespace sorbet::core

#endif // SORBET_AST_LOC_H
