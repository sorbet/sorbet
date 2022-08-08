// has to go first as it violates our poisons
#include "rang.hpp"

#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/Loc.h"
#include <iterator>
#include <sstream>

namespace sorbet::core {

using namespace std;

LocOffsets LocOffsets::join(LocOffsets other) const {
    if (!this->exists()) {
        return other;
    }
    if (!other.exists()) {
        return *this;
    }
    return LocOffsets{min(this->beginPos(), other.beginPos()), max(this->endPos(), other.endPos())};
}

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

Loc::Detail Loc::offset2Pos(const File &file, uint32_t off) {
    Loc::Detail pos;

    ENFORCE(off <= file.source().size(), "file offset out of bounds in file: {} @ {} <= {}", string(file.path()),
            to_string(off), to_string(file.source().size()));
    auto it = absl::c_lower_bound(file.lineBreaks(), off);
    if (it == file.lineBreaks().begin()) {
        pos.line = 1;
        pos.column = off + 1;
        return pos;
    }
    --it;
    pos.line = (it - file.lineBreaks().begin()) + 1;
    pos.column = off - *it;
    return pos;
}

optional<uint32_t> Loc::pos2Offset(const File &file, Loc::Detail pos) {
    auto l = pos.line - 1;
    auto &lineBreaks = file.lineBreaks();
    if (!(0 <= l && l < lineBreaks.size())) {
        return nullopt;
    }
    auto lineOffset = lineBreaks[l];
    auto nextLineStart = l + 1 < lineBreaks.size() ? lineBreaks[l + 1] : file.source().size();
    auto lineLength = nextLineStart - lineOffset;
    if (pos.column > lineLength) {
        return nullopt;
    }
    return lineOffset + pos.column;
}

optional<Loc> Loc::fromDetails(const GlobalState &gs, FileRef fileRef, Loc::Detail begin, Loc::Detail end) {
    const auto &file = fileRef.data(gs);
    const auto beginOff = pos2Offset(file, begin);
    if (!beginOff.has_value()) {
        return nullopt;
    }
    const auto endOff = pos2Offset(file, end);
    if (!endOff.has_value()) {
        return nullopt;
    }
    return Loc(fileRef, beginOff.value(), endOff.value());
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

void addLocLine(stringstream &buf, int line, const File &file, int tabs, int posWidth) {
    printTabs(buf, tabs);
    buf << rang::fgB::black << leftPad(to_string(line + 1), posWidth) << " |" << rang::style::reset;
    ENFORCE(file.lineBreaks().size() > line + 1);
    auto endPos = file.lineBreaks()[line + 1];
    auto numToWrite = endPos - file.lineBreaks()[line] - 1;
    if (numToWrite <= 0) {
        return;
    }
    auto offset = file.lineBreaks()[line] + 1;
    if (offset < 0 || offset >= file.source().size()) {
        fatalLogger->error(R"(msg="Bad addLocLine offset" path="{}" line="{}" offset="{}")", absl::CEscape(file.path()),
                           line, offset);
        fatalLogger->error("source=\"{}\"", absl::CEscape(file.source()));
        ENFORCE(false);
    }
    if (offset + numToWrite > file.source().size()) {
        fatalLogger->error(R"(msg="Bad addLocLine write size" path="{}" line="{}" offset="{}" numToWrite="{}")",
                           absl::CEscape(file.path()), line, offset, numToWrite);
        fatalLogger->error("source=\"{}\"", absl::CEscape(file.source()));
        ENFORCE(false);
    }
    buf.write(file.source().data() + offset, numToWrite);
}
} // namespace

string Loc::toStringWithTabs(const GlobalState &gs, int tabs) const {
    stringstream buf;
    const File &file = this->file().data(gs);
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
        addLocLine(buf, lineIt, file, tabs, posWidth);
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
        addLocLine(buf, lineIt, file, tabs, posWidth);
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

string LocOffsets::showRaw(const Context ctx) const {
    return ctx.locAt(*this).showRaw(ctx);
}
string LocOffsets::showRaw(const MutableContext ctx) const {
    return ctx.locAt(*this).showRaw(ctx);
}
string LocOffsets::showRaw(const GlobalState &gs, const FileRef file) const {
    return Loc(file, *this).showRaw(gs);
}
string LocOffsets::showRaw() const {
    return fmt::format("LocOffsets {{beginPos={}, endPos={}}}", beginPos(), endPos());
}

bool LocOffsets::operator==(const LocOffsets &rhs) const {
    return this->beginLoc == rhs.beginLoc && this->endLoc == rhs.endLoc;
}

bool LocOffsets::operator!=(const LocOffsets &rhs) const {
    return !(rhs == *this);
}

string Loc::showRaw(const GlobalState &gs) const {
    string_view path;
    if (file().exists()) {
        path = file().data(gs).path();
    } else {
        path = "???"sv;
    }

    string censored;
    if (gs.censorForSnapshotTests) {
        censored = File::censorFilePathForSnapshotTests(path);
        if (absl::StartsWith(path, File::URL_PREFIX)) {
            return fmt::format("Loc {{file={} start=removed end=removed}}", censored);
        }
        path = censored;
    }

    if (!exists()) {
        return fmt::format("Loc {{file={} start=??? end=???}}", path);
    }

    auto [start, end] = this->position(gs);
    return fmt::format("Loc {{file={} start={}:{} end={}:{}}}", path, start.line, start.column, end.line, end.column);
}

string Loc::filePosToString(const GlobalState &gs, bool showFull) const {
    stringstream buf;
    if (!file().exists()) {
        buf << "???";
    } else {
        auto path = gs.getPrintablePath(file().data(gs).path());
        if (gs.censorForSnapshotTests) {
            buf << File::censorFilePathForSnapshotTests(path);
        } else {
            buf << path;
        }

        if (exists()) {
            auto pos = position(gs);
            if (path.find("https://") == 0) {
                // For github permalinks
                buf << "#L";
            } else {
                buf << ":";
            }
            auto censor = gs.censorForSnapshotTests && file().data(gs).isPayload();
            buf << (censor ? "CENSORED" : to_string(pos.first.line));
            if (showFull) {
                buf << ":";
                buf << (censor ? "CENSORED" : to_string(pos.first.column));
                buf << "-";
                buf << (censor ? "CENSORED" : to_string(pos.second.line));
                buf << ":";
                buf << (censor ? "CENSORED" : to_string(pos.second.column));
            } else {
                // pos.second.line; is intentionally not printed so that iterm2 can open file name:line_number as links
            }
        }
    }
    return buf.str();
}

optional<string_view> Loc::source(const GlobalState &gs) const {
    if (!exists()) {
        return nullopt;
    } else {
        auto source = this->file().data(gs).source();
        return source.substr(beginPos(), endPos() - beginPos());
    }
}

bool Loc::contains(const Loc &other) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(other.exists());
    return file() == other.file() && other.beginPos() >= beginPos() && other.endPos() <= endPos();
}

bool Loc::operator==(const Loc &rhs) const {
    return storage.offsets == rhs.storage.offsets && storage.fileRef == rhs.storage.fileRef;
}

bool Loc::operator!=(const Loc &rhs) const {
    return !(rhs == *this);
}

Loc Loc::adjust(const GlobalState &gs, int32_t beginAdjust, int32_t endAdjust) const {
    if (!this->exists()) {
        return Loc::none(this->file());
    }

    // Upcast these to signed, 64-bit ints, so we don't have to worry about underflow / overflow.
    int64_t begin = this->beginPos();
    int64_t end = this->endPos();

    uint64_t fileLength = this->file().data(gs).source().size();

    if (begin + beginAdjust < 0) {
        return Loc::none(this->file());
    }

    if (begin + beginAdjust > end + endAdjust) {
        return Loc::none(this->file());
    }

    if (end + endAdjust > fileLength) {
        return Loc::none(this->file());
    }

    uint32_t newBegin = begin + beginAdjust;
    uint32_t newEnd = end + endAdjust;

    return Loc{this->file(), newBegin, newEnd};
}

Loc Loc::adjustLen(const GlobalState &gs, int32_t beginAdjust, int32_t len) const {
    if (!this->exists()) {
        return Loc::none(this->file());
    }

    // Upcast these to signed, 64-bit ints, so we don't have to worry about underflow / overflow.
    int64_t begin = this->beginPos();

    uint64_t fileLength = this->file().data(gs).source().size();

    if (begin + beginAdjust < 0) {
        return Loc::none(this->file());
    }

    if (begin + beginAdjust + len > fileLength) {
        return Loc::none(this->file());
    }

    uint32_t newBegin = begin + beginAdjust;
    uint32_t newEnd = newBegin + len;

    return Loc{this->file(), newBegin, newEnd};
}

pair<Loc, uint32_t> Loc::findStartOfLine(const GlobalState &gs) const {
    auto startDetail = this->position(gs).first;
    auto maybeLineStart = Loc::pos2Offset(this->file().data(gs), {startDetail.line, 1});
    ENFORCE(maybeLineStart.has_value());
    auto lineStart = maybeLineStart.value();
    std::string_view lineView = this->file().data(gs).source().substr(lineStart);

    size_t padding = lineView.find_first_not_of(" \t");
    if (padding == string::npos) {
        // if this line didn't have a whitespace prefix, then don't add any padding to it, so startOffset = lineStart.
        padding = 0;
    }
    uint32_t startOffset = lineStart + padding;
    return make_pair(Loc(this->file(), startOffset, startOffset), padding);
}

} // namespace sorbet::core
