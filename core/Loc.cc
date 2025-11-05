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

bool LocOffsets::contains(const LocOffsets &other) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(other.exists());
    return other.beginPos() >= beginPos() && other.endPos() <= endPos();
}

Loc Loc::join(Loc other) const {
    if (!this->exists()) {
        return other;
    }
    if (!other.exists()) {
        return *this;
    }
    ENFORCE_NO_TIMER(this->file() == other.file(), "joining locations from different files");
    return Loc(this->file(), min(this->beginPos(), other.beginPos()), max(this->endPos(), other.endPos()));
}

Loc::Detail Loc::pos2Detail(const File &file, uint32_t off) {
    Loc::Detail detail;

    if (off > file.source().size()) {
        fatalLogger->error(R"(msg="Bad offset2Pos off" path="{}" off="{}"")", absl::CEscape(file.path()), off);
        fatalLogger->error("source=\"{}\"", absl::CEscape(file.source()));
        ENFORCE_NO_TIMER(false);
    }
    auto lineBreaks = file.lineBreaks();
    // lower_bound is what the C++ STL calls the "least upper bound," i.e., a pointer to the first
    // element which is greater than or equal to you in a sorted list
    auto it = absl::c_lower_bound(lineBreaks, off);
    if (it == lineBreaks.begin()) {
        detail.line = 1;
        detail.column = off + 1;
        return detail;
    }
    detail.line = std::distance(lineBreaks.begin(), it) + 1;
    --it;
    detail.column = off - *it;
    return detail;
}

optional<uint32_t> Loc::detail2Pos(const File &file, Loc::Detail detail) {
    auto line1idx = detail.line;
    auto lineBreaks = file.lineBreaks();
    if (!(1 <= line1idx && line1idx <= lineBreaks.size() + 1)) {
        return nullopt;
    }
    auto line0idx = line1idx - 1;

    auto lineStart = line0idx == 0 ? 0 : lineBreaks[line0idx - 1] + 1;
    auto lineEnd = line0idx < lineBreaks.size() ? lineBreaks[line0idx] : file.source().size();
    auto lineLength = lineEnd - lineStart;
    auto column1idx = detail.column;
    if (column1idx > lineLength + 1) {
        return nullopt;
    }

    auto column0idx = column1idx - 1;
    return lineStart + column0idx;
}

optional<Loc> Loc::fromDetails(const GlobalState &gs, FileRef fileRef, Loc::Detail begin, Loc::Detail end) {
    const auto &file = fileRef.data(gs);
    const auto beginOff = detail2Pos(file, begin);
    if (!beginOff.has_value()) {
        return nullopt;
    }
    const auto endOff = detail2Pos(file, end);
    if (!endOff.has_value()) {
        return nullopt;
    }
    return Loc(fileRef, beginOff.value(), endOff.value());
}

pair<Loc::Detail, Loc::Detail> Loc::toDetails(const GlobalState &gs) const {
    Loc::Detail begin(pos2Detail(this->file().data(gs), beginPos()));
    Loc::Detail end(pos2Detail(this->file().data(gs), endPos()));
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
static_assert((WINDOW_SIZE & 1) == 0, "WINDOW_SIZE should be divisible by 2");

void addLocLine(stringstream &buf, int line, const File &file, int tabs, int lineNumPadding,
                bool censorForSnapshotTests) {
    printTabs(buf, tabs);
    buf << rang::fgB::black;
    if (censorForSnapshotTests) {
        buf << leftPad("NN", lineNumPadding);
    } else {
        buf << leftPad(to_string(line + 1), lineNumPadding);
    }
    buf << " |" << rang::style::reset;
    if (!(0 <= line || line < file.lineBreaks().size())) {
        fatalLogger->error(R"(msg="Bad addLocLine line" path="{}" line="{}"")", absl::CEscape(file.path()), line);
        fatalLogger->error("source=\"{}\"", absl::CEscape(file.source()));
        ENFORCE_NO_TIMER(false);
    }
    auto endPos = file.lineBreaks()[line];
    auto offset = line == 0 ? 0 : file.lineBreaks()[line - 1] + 1;
    if (endPos <= offset) {
        return;
    }
    auto numToWrite = endPos - offset;
    if (offset >= file.source().size()) {
        fatalLogger->error(R"(msg="Bad addLocLine offset" path="{}" line="{}" offset="{}")", absl::CEscape(file.path()),
                           line, offset);
        fatalLogger->error("source=\"{}\"", absl::CEscape(file.source()));
        ENFORCE_NO_TIMER(false);
    }
    if (offset + numToWrite > file.source().size()) {
        fatalLogger->error(R"(msg="Bad addLocLine write size" path="{}" line="{}" offset="{}" numToWrite="{}")",
                           absl::CEscape(file.path()), line, offset, numToWrite);
        fatalLogger->error("source=\"{}\"", absl::CEscape(file.source()));
        ENFORCE_NO_TIMER(false);
    }
    buf.write(file.source().data() + offset, numToWrite);
}

void printLocDetails(stringstream &to, pair<Loc::Detail, Loc::Detail> details, bool showFull, bool censor) {
    if (censor) {
        if (showFull) {
            to << "CENSORED:CENSORED-CENSORED:CENSORED";
        } else {
            to << "CENSORED";
        }
    } else {
        if (showFull) {
            to << to_string(details.first.line) << ':' << to_string(details.first.column) << '-';
            to << to_string(details.second.line) << ':' << to_string(details.second.column);
        } else {
            to << to_string(details.first.line);
            // Second line is intentionally not printed so that iTerm2 can open file name:line_number as links
        }
    }
}
} // namespace

string Loc::toStringWithTabs(const GlobalState &gs, int tabs) const {
    stringstream buf;
    const File &file = this->file().data(gs);
    auto censorForSnapshotTests = gs.censorForSnapshotTests && file.isPayload();
    auto details = this->toDetails(gs);
    int lineNumPadding = details.second.line < 100 ? 2 : details.second.line < 10000 ? 4 : 8;

    const auto firstLine = details.first.line - 1;
    auto lineIt = firstLine;
    bool first = true;
    while (lineIt != details.second.line && lineIt - firstLine < WINDOW_HALF_SIZE) {
        if (!first) {
            buf << '\n';
        }
        first = false;
        addLocLine(buf, lineIt, file, tabs, lineNumPadding, censorForSnapshotTests);
        lineIt++;
    }
    if (lineIt != details.second.line && lineIt < details.second.line - WINDOW_HALF_SIZE) {
        buf << '\n';
        printTabs(buf, tabs);
        string space(lineNumPadding, ' ');
        buf << space << rang::fgB::black << " |" << rang::style::reset << "...";
        lineIt = details.second.line - WINDOW_HALF_SIZE;
    }
    while (lineIt != details.second.line) {
        buf << '\n';
        addLocLine(buf, lineIt, file, tabs, lineNumPadding, censorForSnapshotTests);
        lineIt++;
    }

    if (details.second.line == details.first.line) {
        // add squigly
        buf << '\n';
        printTabs(buf, tabs);
        for (int i = 0; i <= lineNumPadding; i++) {
            buf << ' ';
        }
        int p;

        for (p = 0; p < details.first.column; p++) {
            buf << ' ';
        }
        buf << rang::fg::cyan;
        if (details.second.column > details.first.column) {
            for (; p < details.second.column; p++) {
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

    auto [start, end] = this->toDetails(gs);
    return fmt::format("Loc {{file={} start={}:{} end={}:{}}}", path, start.line, start.column, end.line, end.column);
}

string Loc::fileShortPosToString(const GlobalState &gs) const {
    stringstream buf;
    if (!file().exists()) {
        buf << "???";
    } else if (exists()) {
        auto details = toDetails(gs);
        auto showFull = true;
        auto censor = gs.censorForSnapshotTests && file().data(gs).isPayload();
        printLocDetails(buf, details, showFull, censor);
    }
    return buf.str();
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
            auto details = toDetails(gs);
            if (absl::StartsWith(path, "https://")) {
                // For github permalinks
                buf << "#L";
            } else {
                buf << ':';
            }

            auto censor = gs.censorForSnapshotTests && file().data(gs).isPayload();
            printLocDetails(buf, details, showFull, censor);
        }
    }
    return buf.str();
}

optional<string_view> Loc::source(const GlobalState &gs) const {
    if (!exists()) {
        return nullopt;
    } else {
        auto source = this->file().data(gs).source();
        return source.substr(beginPos(), length());
    }
}

bool Loc::contains(const Loc &other) const {
    ENFORCE_NO_TIMER(this->exists());
    ENFORCE_NO_TIMER(other.exists());
    return file() == other.file() && offsets().contains(other.offsets());
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

pair<Loc, uint32_t> Loc::findStartOfIndentation(const GlobalState &gs) const {
    auto startDetail = this->toDetails(gs).first;
    auto maybeLineStart = Loc::detail2Pos(this->file().data(gs), {startDetail.line, 1});
    ENFORCE_NO_TIMER(maybeLineStart.has_value());
    auto lineStart = maybeLineStart.value();
    string_view lineView = this->file().data(gs).source().substr(lineStart);

    size_t padding = lineView.find_first_not_of(" \t");
    if (padding == string::npos) {
        // if this line didn't have a whitespace prefix, then don't add any padding to it, so startOffset =
        // lineStart.
        padding = 0;
    }
    uint32_t startOffset = lineStart + padding;
    return make_pair(Loc(this->file(), startOffset, startOffset), padding);
}

Loc Loc::truncateToFirstLine(const GlobalState &gs) const {
    auto [beginDetail, endDetail] = this->toDetails(gs);
    if (beginDetail.line == endDetail.line) {
        return *this;
    }

    const auto &lineBreaks = this->file().data(gs).lineBreaks();
    // Detail::line is 1-indexed. lineBreaks stores the ending newline of the 0-indexed line
    auto firstNewline = lineBreaks[beginDetail.line - 1];
    return Loc(this->file(), this->beginPos(), firstNewline);
}

} // namespace sorbet::core
