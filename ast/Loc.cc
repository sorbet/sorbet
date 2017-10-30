#include "Loc.h"
#include "Context.h"

#include <algorithm>
#include <iterator>

namespace ruby_typer {
namespace ast {

using namespace std;

Loc::Detail offset2Pos(ast::UTF8Desc source, u4 off) {
    Loc::Detail pos;

    Error::check(off < source.to);

    pos.line = 1 + count(source.from, source.from + off, '\n');

    auto end = make_reverse_iterator(source.from + off);
    auto begin = make_reverse_iterator(source.from);
    auto nl = find(end, begin, '\n');

    if (nl == begin) {
        pos.column = off;
    } else {
        pos.column = nl - end;
    }

    return pos;
}

pair<Loc::Detail, Loc::Detail> Loc::position(ast::GlobalState &ctx) {
    ast::File &file = this->file.file(ctx);
    return make_pair(offset2Pos(file.source(), begin_pos), offset2Pos(file.source(), end_pos));
}

} // namespace ast
} // namespace ruby_typer
