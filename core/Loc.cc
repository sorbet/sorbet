#include "core/Loc.h"
#include "core/Context.h"
#include "rang.hpp"

#include "absl/algorithm/container.h"
#include <algorithm>
#include <iterator>

namespace sorbet::core {

using namespace std;

Loc Loc::join(Loc other) const {
    if (!this->exists()) {
        return other;
    }
    if (!other.exists()) {
        return *this;
    }
    ENFORCE(this->file() == other.file(), "joining locations from different files");
    return Loc(this->file(), min(this->beginPos(), other.beginPos()), max(this->endPos(), other.endPos()));
}

Loc::Detail Loc::offset2Pos(const File &file, u4 off) {
    Loc::Detail pos;

    ENFORCE(off <= file.source().size(), "file offset out of bounds");
    if (off >= file.source().size()) {
        // parser generate positions out of file \facepalm.
        off = file.source().size() - 1;
    }
    auto it = absl::c_lower_bound(file.line_breaks(), off);
    if (it == file.line_breaks().begin()) {
        pos.line = 1;
        pos.column = off + 1;
        return pos;
    }
    --it;
    pos.line = (it - file.line_breaks().begin()) + 1;
    pos.column = off - *it;
    return pos;
}

u4 Loc::pos2Offset(const File &file, Loc::Detail pos) {
    auto l = pos.line - 1;
    auto lineOffset = file.line_breaks()[l];
    return lineOffset + pos.column;
}

Loc Loc::fromDetails(const GlobalState &gs, FileRef fileRef, Loc::Detail begin, Loc::Detail end) {
    const auto &file = fileRef.data(gs);
    return Loc(fileRef, pos2Offset(file, begin), pos2Offset(file, end));
}

pair<Loc::Detail, Loc::Detail> Loc::position(const GlobalState &gs) const {
    Loc::Detail begin(offset2Pos(this->file().data(gs), beginPos()));
    Loc::Detail end(offset2Pos(this->file().data(gs), endPos()));
    return make_pair(begin, end);
}
namespace {
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

constexpr unsigned int WINDOW_SIZE = 10; // how many lines of source to print
constexpr unsigned int WINDOW_HALF_SIZE = WINDOW_SIZE / 2;
static_assert((WINDOW_SIZE & 1) == 0, "WINDOW_SIZE should be divisable by 2");

void addLocLine(stringstream &buf, int line, const File &filed, int tabs, int posWidth) {
    printTabs(buf, tabs);
    buf << rang::fgB::black << leftPad(to_string(line + 1), posWidth) << " |" << rang::style::reset;
    auto endPos = filed.line_breaks()[line + 1];
    if (filed.source()[endPos] == '\n') {
        endPos -= 1;
    }
    buf.write(filed.source().data() + filed.line_breaks()[line] + 1, endPos - filed.line_breaks()[line]);
}
} // namespace

string Loc::toString(const GlobalState &gs, int tabs) const {
    stringstream buf;
    const File &filed = this->file().data(gs);
    auto pos = this->position(gs);
    int posWidth = pos.second.line < 100 ? 2 : pos.second.line < 10000 ? 4 : 8;

    const auto firstLine = pos.first.line - 1;
    auto lineIt = firstLine;
    bool first = true;
    while (lineIt != pos.second.line && lineIt - firstLine < WINDOW_HALF_SIZE) {
        if (!first) {
            buf << '\n';
        }
        first = false;
        addLocLine(buf, lineIt, filed, tabs, posWidth);
        lineIt++;
    }
    if (lineIt != pos.second.line && lineIt < pos.second.line - WINDOW_HALF_SIZE) {
        buf << '\n';
        printTabs(buf, tabs);
        string space(posWidth, ' ');
        buf << space << rang::fgB::black << " |" << rang::style::reset << "...";
        lineIt = pos.second.line - WINDOW_HALF_SIZE;
    }
    while (lineIt != pos.second.line) {
        buf << '\n';
        addLocLine(buf, lineIt, filed, tabs, posWidth);
        lineIt++;
    }

    if (pos.second.line == pos.first.line) {
        // add squigly
        buf << '\n';
        printTabs(buf, tabs);
        for (int i = 0; i <= posWidth; i++) {
            buf << ' ';
        }
        int p;

        for (p = 0; p < pos.first.column; p++) {
            buf << ' ';
        }
        buf << rang::fg::cyan;
        if (pos.second.column - pos.first.column > 0) {
            for (; p < pos.second.column; p++) {
                buf << '^';
            }
        } else {
            // If zero-width loc, at least show something.
            buf << '^';
        }
        buf << rang::fg::reset;
    }
    return buf.str();
}

string Loc::showRaw(const GlobalState &gs) const {
    auto path = file().data(gs).path();
    auto [start, end] = this->position(gs);
    return fmt::format("Loc {{file={} start={}:{} end={}:{}}}", path, start.line, start.column, end.line, end.column);
}

string Loc::filePosToString(const GlobalState &gs) const {
    stringstream buf;
    if (!exists()) {
        buf << "???";
    } else {
        auto pos = position(gs);
        auto path = file().data(gs).path();
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

string Loc::source(const GlobalState &gs) const {
    auto source = this->file().data(gs).source();
    return string(source.substr(beginPos(), endPos() - beginPos()));
}

bool Loc::contains(const Loc &other) const {
    return file() == other.file() && other.beginPos() >= beginPos() && other.endPos() <= endPos();
}

bool Loc::operator==(const Loc &rhs) const {
    return storage.endLoc == rhs.storage.endLoc && storage.beginLoc == rhs.storage.beginLoc &&
           storage.fileRef == rhs.storage.fileRef;
}

bool Loc::operator!=(const Loc &rhs) const {
    return !(rhs == *this);
}

} // namespace sorbet::core
