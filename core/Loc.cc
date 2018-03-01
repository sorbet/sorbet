#include "Loc.h"
#include "Context.h"
#include "rang.hpp"

#include <algorithm>
#include <iterator>

namespace ruby_typer {
namespace core {

using namespace std;

Loc Loc::join(Loc other) {
    if (this->is_none()) {
        return other;
    }
    if (other.is_none()) {
        return *this;
    }
    ENFORCE(this->file == other.file, "joining locations from different files");

    return Loc{this->file, min(this->begin_pos, other.begin_pos), max(this->end_pos, other.end_pos)};
}

Loc::Detail Loc::offset2Pos(core::FileRef source, u4 off, const core::GlobalState &gs) {
    Loc::Detail pos;

    const core::File &file = source.data(gs);
    ENFORCE(off <= file.source().size(), "file offset out of bounds");
    if (off >= file.source().size()) {
        // parser generate positions out of file \facepalm.
        off = file.source().size() - 1;
    }
    auto it = std::lower_bound(file.line_breaks.begin(), file.line_breaks.end(), off);
    if (it == file.line_breaks.begin()) {
        pos.line = 1;
        pos.column = off + 1;
        return pos;
    }
    --it;
    pos.line = (it - file.line_breaks.begin()) + 1;
    pos.column = off - *it;
    return pos;
}

pair<Loc::Detail, Loc::Detail> Loc::position(const core::GlobalState &gs) const {
    Loc::Detail begin(offset2Pos(this->file, begin_pos, gs));
    Loc::Detail end(offset2Pos(this->file, end_pos, gs));
    return make_pair(begin, end);
}

void printTabs(stringstream &to, int count) {
    int i = 0;
    while (i < count) {
        to << "  ";
        i++;
    }
}

string leftPad(string s, int l) {
    if (s.size() < l) {
        string prefix(l - s.size(), ' ');
        s = prefix + s;
    } else {
        s = s.substr(0, l);
    }
    return s;
}

string Loc::toString(const core::GlobalState &gs, int tabs) const {
    stringstream buf;
    const File &filed = this->file.data(gs);
    auto pos = this->position(gs);
    int posWidth = pos.second.line < 100 ? 2 : pos.second.line < 10000 ? 4 : 8;

    auto lineIt = pos.first.line - 1;
    while (lineIt != pos.second.line) {
        printTabs(buf, tabs);
        buf << rang::fgB::black << leftPad(to_string(lineIt + 1), posWidth) << " |" << rang::style::reset;
        auto endPos = filed.line_breaks[lineIt + 1];
        if (filed.source()[endPos] == '\n') {
            endPos -= 1;
        }
        buf.write(filed.source().data() + filed.line_breaks[lineIt] + 1, endPos - filed.line_breaks[lineIt]);
        buf << '\n';
        lineIt++;
    }

    if (pos.second.line == pos.first.line) {
        // add squigly
        printTabs(buf, tabs);
        for (int i = 0; i <= posWidth; i++) {
            buf << ' ';
        }
        int p;

        for (p = 0; p < pos.first.column; p++) {
            buf << ' ';
        }
        buf << rang::fg::green;
        for (; p < pos.second.column; p++) {
            buf << '^';
        }
        buf << rang::fg::reset;
    }
    return buf.str();
}

string Loc::filePosToString(const GlobalState &gs) const {
    stringstream buf;
    if (is_none()) {
        buf << "???";
    } else {
        auto pos = position(gs);
        auto path = file.data(gs).path();
        if (path.find("https://") == 0) {
            // For github permalinks
            buf << path << "#L";
        } else {
            buf << path << ":";
        }
        buf << pos.first.line;
        // pos.second.line; is intentionally not printed so that iterm2 can open file name:line_number as links
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
