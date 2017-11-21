#include "Loc.h"
#include "Context.h"

#include <algorithm>
#include <iterator>

namespace ruby_typer {
namespace core {

using namespace std;

Loc::Detail Loc::offset2Pos(core::FileRef source, u4 off, core::GlobalState &gs) {
    Loc::Detail pos;

    core::File &file = source.file(gs);
    Error::check(off <= file.source().to);
    auto it = std::lower_bound(file.line_breaks.begin(), file.line_breaks.end(), off);
    if (it == file.line_breaks.begin()) {
        pos.line = 1;
        pos.column = off + 1;
        return pos;
    }
    --it;
    pos.line = (it - file.line_breaks.begin()) + 2;
    pos.column = off - *it;
    return pos;
}

pair<Loc::Detail, Loc::Detail> Loc::position(core::GlobalState &gs) {
    Loc::Detail begin(offset2Pos(this->file, begin_pos, gs));
    Loc::Detail end(offset2Pos(this->file, end_pos, gs));
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
