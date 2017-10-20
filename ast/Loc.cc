#include "Loc.h"
#include "Context.h"

#include <algorithm>
#include <iterator>

namespace ruby_typer {
namespace ast {

using namespace std;

void offset2Pos(ast::UTF8Desc source, u4 off, u4 &line, u4 &col) {
    Error::check(off < source.to);

    line = 1 + count(source.from, source.from + off, '\n');

    auto end = make_reverse_iterator(source.from + off);
    auto begin = make_reverse_iterator(source.from);
    auto nl = find(end, begin, '\n');

    if (nl == begin) {
        col = off;
    } else {
        col = nl - end;
    }
}

void Loc::position(ast::ContextBase &ctx, u4 &begin_line, u4 &begin_col, u4 &end_line, u4 &end_col) {
    ast::File &file = this->file.file(ctx);
    offset2Pos(file.source(), begin_pos, begin_line, begin_col);
    offset2Pos(file.source(), end_pos, end_line, end_col);
}

} // namespace ast
} // namespace ruby_typer
