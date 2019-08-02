#include "core/AutocorrectSuggestion.h"
#include "absl/strings/str_cat.h"

using namespace std;

namespace sorbet::core {

bool hasSeen(const UnorderedSet<Loc> &seen, Loc loc) {
    for (auto &seenLoc : seen) {
        if (seenLoc.file() != loc.file()) {
            continue;
        }
        // Check exactly equal for zero-width locs
        if (seenLoc == loc) {
            return true;
        }
        // Check for overlapping
        if (seenLoc.beginPos() >= loc.beginPos() && seenLoc.beginPos() < loc.endPos()) {
            return true;
        }
        if (seenLoc.endPos() >= loc.beginPos() && seenLoc.endPos() < loc.endPos()) {
            return true;
        }
    }
    return false;
}

UnorderedMap<FileRef, string> AutocorrectSuggestion::apply(vector<AutocorrectSuggestion> autocorrects,
                                                           UnorderedMap<FileRef, string> sources) {
    vector<pair<core::Loc, string>> edits;
    for (auto &autocorrect : autocorrects) {
        move(autocorrect.edits.begin(), autocorrect.edits.end(), back_inserter(edits));
    }

    // Sort the locs backwards
    auto compare = [](const pair<core::Loc, string> &left, const pair<core::Loc, string> &right) {
        if (left.first.file() != right.first.file()) {
            return left.first.file().id() > right.first.file().id();
        }

        auto a = left.first.beginPos();
        auto b = right.first.beginPos();
        if (a != b) {
            return a > b;
        }

        return false;
    };
    fast_sort(edits, compare);

    UnorderedSet<Loc> seen; // used to make sure nothing overlaps
    UnorderedMap<FileRef, string> ret;
    for (auto &edit : edits) {
        core::Loc loc = edit.first;
        std::string_view replacement = edit.second;
        if (!ret.count(loc.file())) {
            ret[loc.file()] = sources[loc.file()];
        }
        auto source = ret[loc.file()];
        auto start = loc.beginPos();
        auto end = loc.endPos();

        if (hasSeen(seen, loc)) {
            continue;
        }
        seen.emplace(loc);
        ret[loc.file()] = absl::StrCat(source.substr(0, start), replacement, source.substr(end, -1));
    }
    return ret;
}

} // namespace sorbet::core
