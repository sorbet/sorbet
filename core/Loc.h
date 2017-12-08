#ifndef SRUBY_AST_LOC_H
#define SRUBY_AST_LOC_H

#include "Files.h"

namespace ruby_typer {
namespace core {
class GlobalState;

class Loc final {
public:
    FileRef file;
    u4 begin_pos, end_pos;

    static Loc none(FileRef file) {
        return Loc{file, (u4)-1, (u4)-1};
    }

    bool is_none() {
        return begin_pos == (u4)-1 && end_pos == (u4)-1;
    }

    Loc join(Loc other);

    Loc() : file(0), begin_pos(-1), end_pos(-1){};
    Loc(FileRef file, u4 begin, u4 end) : file(file), begin_pos(begin), end_pos(end){};

    Loc &operator=(const Loc &rhs) = default;
    Loc &operator=(Loc &&rhs) = default;
    Loc(const Loc &rhs) = default;
    Loc(Loc &&rhs) = default;

    struct Detail {
        u4 line, column;
    };

    std::pair<Detail, Detail> position(core::GlobalState &gs);
    std::string toString(core::GlobalState &gs, int tabs = 0);

    bool operator==(const Loc &rhs) const;

    bool operator!=(const Loc &rhs) const;
    static Detail offset2Pos(core::FileRef source, u4 off, core::GlobalState &gs);
};
} // namespace core
} // namespace ruby_typer

#endif // SRUBY_AST_LOC_H
