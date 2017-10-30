#ifndef SRUBY_AST_LOC_H
#define SRUBY_AST_LOC_H

#include "Files.h"

namespace ruby_typer {
namespace ast {
class GlobalState;

class Loc {
public:
    FileRef file;
    u4 begin_pos, end_pos;

    static Loc none(FileRef file) {
        return Loc{file, (u4)-1, (u4)-1};
    }

    bool is_none() {
        return begin_pos == (u4)-1 && end_pos == (u4)-1;
    }

    Loc() : file(0), begin_pos(-1), end_pos(-1){};
    Loc(FileRef file, u4 begin, u4 end) : file(file), begin_pos(begin), end_pos(end){};

    Loc &operator=(const Loc &rhs) = default;
    Loc &operator=(Loc &&rhs) = default;
    Loc(const Loc &rhs) = default;
    Loc(Loc &&rhs) = default;

    struct Detail {
        u4 line, column;
    };

    std::pair<Detail, Detail> position(ast::GlobalState &ctx);
};
} // namespace ast
} // namespace ruby_typer

#endif // SRUBY_AST_LOC_H
