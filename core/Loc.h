#ifndef SORBET_AST_LOC_H
#define SORBET_AST_LOC_H

#include "Files.h"

namespace sorbet {
namespace core {
namespace serialize {
class SerializerImpl;
}
class GlobalState;

class Loc final {
    struct {
        u4 high; // 3 upper bytes are beginLoc, 1 lower byte is upper byte of endLoc
        u4 low;  // 2 upper bytes are lower bytes of endLoc, 2 lower bytes are fileRef
    } storage;
    friend std::hash<sorbet::core::Loc>;
    friend class sorbet::core::serialize::SerializerImpl;

    static constexpr int STORAGE_HIGH_NONE = 0xffffffff;
    static constexpr int STORAGE_LOW_FILE_MASK = 0x0000ffff;
    static constexpr int INVALID_POS = 0x00ffffff;

    void setFile(core::FileRef file) {
        storage.low = (storage.low & ~STORAGE_LOW_FILE_MASK) | file.id();
    }

public:
    static Loc none(FileRef file = FileRef()) {
        return Loc{file, INVALID_POS, INVALID_POS};
    }

    bool exists() const {
        return storage.high != STORAGE_HIGH_NONE && ((storage.low & STORAGE_LOW_FILE_MASK) != 0);
    }

    Loc join(Loc other) const;

    u4 beginPos() const {
        return storage.high >> 8;
    };

    u4 endPos() const {
        return ((storage.high & 255) << 16) + (storage.low >> 16);
    }

    FileRef file() const {
        return FileRef(storage.low & STORAGE_LOW_FILE_MASK);
    }

    inline Loc(FileRef file, u4 begin, u4 end) : storage{(begin << 8) | (end >> 16), (end << 16) + file.id()} {
        ENFORCE(begin <= INVALID_POS);
        ENFORCE(end <= INVALID_POS);
        ENFORCE(begin <= end);
    }

    Loc() : Loc(FileRef(), INVALID_POS, INVALID_POS){};

    Loc &operator=(const Loc &rhs) = default;
    Loc &operator=(Loc &&rhs) = default;
    Loc(const Loc &rhs) = default;
    Loc(Loc &&rhs) = default;

    struct Detail {
        u4 line, column;
    };

    bool contains(const Loc &other) const;
    std::pair<Detail, Detail> position(const GlobalState &gs) const;
    std::string toString(const GlobalState &gs, int tabs = 0) const;
    std::string filePosToString(const GlobalState &gs) const;
    std::string source(const GlobalState &gs) const;

    bool operator==(const Loc &rhs) const;

    bool operator!=(const Loc &rhs) const;
    static u4 pos2Offset(FileRef source, Detail pos, const GlobalState &gs);
    static Detail offset2Pos(FileRef source, u4 off, const GlobalState &gs);
};
CheckSize(Loc, 8, 4);
} // namespace core
} // namespace sorbet

template <> struct std::hash<sorbet::core::Loc> {
    std::size_t operator()(const sorbet::core::Loc loc) const {
        return 5 * loc.storage.high + 7 * loc.storage.low;
    }
};

#endif // SORBET_AST_LOC_H
