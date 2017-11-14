#include "Loc.h"
#include "Context.h"

#include <algorithm>
#include <iterator>

namespace ruby_typer {
namespace core {

using namespace std;

Loc::Detail offset2Pos(core::UTF8Desc source, u4 off) {
    Loc::Detail pos;

    Error::check(off <= source.to);

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

pair<Loc::Detail, Loc::Detail> Loc::position(core::GlobalState &gs) {
    core::File &file = this->file.file(gs);
    core::UTF8Desc source(file.source());
    Loc::Detail begin(offset2Pos(source, begin_pos));
    Loc::Detail end(offset2Pos(source, end_pos));
    return make_pair(begin, end);
}

string Loc::toString(core::GlobalState &gs) {
    stringstream buf;
    UTF8Desc source = this->file.file(gs).source();
    auto pos = this->position(gs);
    auto endstart = make_reverse_iterator(source.from + this->begin_pos);
    auto beginstart = make_reverse_iterator(source.from);
    auto start = find(endstart, beginstart, '\n');

    auto end = find(source.from + this->end_pos, source.from + source.to, '\n');

    auto offset1 = beginstart - start;
    auto offset2 = end - source.from;
    string outline(source.from + (offset1), source.from + (offset2));
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

bool Loc::operator==(const Loc &rhs) const {
    return file == rhs.file && begin_pos == rhs.begin_pos && end_pos == rhs.end_pos;
}

bool Loc::operator!=(const Loc &rhs) const {
    return !(rhs == *this);
}

} // namespace core
} // namespace ruby_typer
