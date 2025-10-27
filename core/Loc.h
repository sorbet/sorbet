#ifndef SORBET_AST_LOC_H
#define SORBET_AST_LOC_H

#include "core/Files.h"
#include "core/LocOffsets.h"

namespace sorbet::core {
namespace serialize {
class SerializerImpl;
}
class GlobalState;
class Context;
class MutableContext;

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
    bool empty() const {
        ENFORCE_NO_TIMER(exists());
        return storage.offsets.empty();
    }

    Loc join(Loc other) const;

    // For a given Loc, returns a zero-length version that starts at the same location.
    Loc copyWithZeroLength() const {
        return {this->storage.fileRef, this->storage.offsets.copyWithZeroLength()};
    }
    // As above, but returns a zero-length version that starts at the end of the Loc.
    Loc copyEndWithZeroLength() const {
        return {this->storage.fileRef, this->storage.offsets.copyEndWithZeroLength()};
    }

    uint32_t beginPos() const {
        return storage.offsets.beginLoc;
    };

    uint32_t endPos() const {
        return storage.offsets.endLoc;
    }
    uint32_t length() const {
        ENFORCE_NO_TIMER(storage.offsets.exists());
        return storage.offsets.length();
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

    inline Loc(FileRef file, uint32_t begin, uint32_t end) : storage{{begin, end}, file} {
        ENFORCE_NO_TIMER(storage.offsets.exists());
    }

    inline Loc(FileRef file, LocOffsets offsets) : storage{offsets, file} {}

    Loc() : Loc(0, LocOffsets::none()){};

    Loc &operator=(const Loc &rhs) = default;
    Loc &operator=(Loc &&rhs) = default;
    Loc(const Loc &rhs) = default;
    Loc(Loc &&rhs) = default;

    struct Detail {
        // 1-indexed, like would be reported in a text editor (useful for error messages)
        uint32_t line, column;
    };

    bool contains(const Loc &other) const;
    std::pair<Detail, Detail> toDetails(const GlobalState &gs) const;
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string toString(const GlobalState &gs) const {
        return toStringWithTabs(gs);
    }
    std::string showRaw(const GlobalState &gs) const;
    std::string fileShortPosToString(const GlobalState &gs) const;
    std::string filePosToString(const GlobalState &gs, bool showFull = false) const;
    std::optional<std::string_view> source(const GlobalState &gs) const;

    bool operator==(const Loc &rhs) const;

    bool operator!=(const Loc &rhs) const;
    static std::optional<uint32_t> detail2Pos(const File &file, Detail detail);
    static Detail pos2Detail(const File &file, uint32_t off);
    static std::optional<Loc> fromDetails(const GlobalState &gs, FileRef fileRef, Detail begin, Detail end);

    // Create a new Loc by adjusting the beginPos and endPos of this Loc, like this:
    //
    //     Loc{file(), beginPos() + beginAdjust, endPos() + endAdjust}
    //
    // but takes care to check that the resulting Loc is a valid slice of the source() buffer,
    // taking care to avoid integer overflow / underflow.
    //
    // For example:
    //
    //     `loc.adjust(gs, -1, 0).exists() == false` if `loc.beginPos() == 0`
    //     `loc.adjust(gs, 0, 1).exists() == false` if `loc.endPos() == loc.file().data(gs).source().size()`
    //
    // etc.
    Loc adjust(const GlobalState &gs, int32_t beginAdjust, int32_t endAdjust) const;

    // Like `Loc::adjust`, but takes a start offset and length, instead of independently adjusting
    // the begin and end positions.
    Loc adjustLen(const GlobalState &gs, int32_t beginAdjust, int32_t len) const;

    // For a given Loc, returns
    //
    // - the Loc corresponding to the first non-whitespace character on this line, and
    // - how many characters of the start of this line are whitespace (the indentation level).
    //
    std::pair<Loc, uint32_t> findStartOfIndentation(const GlobalState &gs) const;

    // If the given loc spans multiple lines, return a new location which has been truncated to
    // one line (excluding the newline character which ends the first line).
    //
    // Otherwise, return the current loc unchanged.
    Loc truncateToFirstLine(const GlobalState &gs) const;
};
CheckSize(Loc, 12, 4);

template <typename H> H AbslHashValue(H h, const Loc &m) {
    return H::combine(std::move(h), m.storage.offsets.beginLoc, m.storage.offsets.endLoc, m.storage.fileRef.id());
}
} // namespace sorbet::core

#endif // SORBET_AST_LOC_H
