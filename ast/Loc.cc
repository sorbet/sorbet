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

std::string Loc::toString(ast::GlobalState &ctx) {
    stringstream buf;
    UTF8Desc source = this->file.file(ctx).source();
    auto pos = this->position(ctx);
    const char *i = source.from;
    int lineCount = 0;
    while (lineCount < pos.first.line - 1) {
        i = (const char *)memchr(i + 1, '\n', source.to - (i - source.from));
        lineCount++;
    }
    const char *j = i;
    while (lineCount < pos.second.line) {
        const char *nj = (const char *)memchr(j + 1, '\n', source.to - (j - source.from));
        if (nj != nullptr) {
            j = nj;
        } else {
            break;
        }
        lineCount++;
    }
    string outline(i + 1, j - i - 1);
    buf << outline;
    if (pos.second.line == pos.first.line) {
        // add squigly
        buf << endl;
        int p;
        for (p = 0; p < pos.first.column; p++) {
            buf << " ";
        }
        for (; p < pos.second.column; p++) {
            buf << "^";
        }
    }
    return buf.str();
}

} // namespace ast
} // namespace ruby_typer
