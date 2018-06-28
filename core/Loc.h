#ifndef SORBET_AST_LOC_H
#define SORBET_AST_LOC_H

#include "Files.h"

namespace sorbet {
namespace core {
class GlobalState;

class Loc final {
public:
    FileRef file;
    u4 begin_pos, end_pos;

    static Loc none(FileRef file = FileRef()) {
        return Loc{file, (u4)-1, (u4)-1};
    }

    bool is_none() const {
        return begin_pos == (u4)-1 && end_pos == (u4)-1;
    }

    Loc join(Loc other);

    Loc() : file(), begin_pos(-1), end_pos(-1){};
    Loc(FileRef file, u4 begin, u4 end);

    Loc &operator=(const Loc &rhs) = default;
    Loc &operator=(Loc &&rhs) = default;
    Loc(const Loc &rhs) = default;
    Loc(Loc &&rhs) = default;

    struct Detail {
        u4 line, column;
    };

    bool contains(const Loc &other) const;
    std::pair<Detail, Detail> position(const core::GlobalState &gs) const;
    std::string toString(const core::GlobalState &gs, int tabs = 0) const;
    std::string filePosToString(const GlobalState &gs) const;

    bool operator==(const Loc &rhs) const;

    bool operator!=(const Loc &rhs) const;
    static u4 pos2Offset(core::FileRef source, Detail pos, const core::GlobalState &gs);
    static Detail offset2Pos(core::FileRef source, u4 off, const core::GlobalState &gs);
};
} // namespace core
} // namespace sorbet

#endif // SORBET_AST_LOC_H
