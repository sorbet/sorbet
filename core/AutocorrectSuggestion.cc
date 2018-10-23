#include "core/AutocorrectSuggestion.h"
#include "absl/strings/str_cat.h"

using namespace std;

namespace sorbet::core {

bool hasSeen(const UnorderedSet<Loc> &seen, Loc loc) {
    for (auto &seenLoc : seen) {
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
    // Sort the locs backwards
    auto compare = [](const AutocorrectSuggestion &left, const AutocorrectSuggestion &right) {
        if (left.loc.file() != right.loc.file()) {
            return left.loc.file().id() > right.loc.file().id();
        }

        auto a = left.loc.beginPos();
        auto b = right.loc.beginPos();
        if (a != b) {
            return a > b;
        }

        return false;
    };
    fast_sort(autocorrects, compare);

    UnorderedSet<Loc> seen; // used to make sure nothing overlaps
    UnorderedMap<FileRef, string> ret;
    for (auto &autocorrect : autocorrects) {
        auto &loc = autocorrect.loc;
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
        ret[loc.file()] = absl::StrCat(source.substr(0, start), autocorrect.replacement, source.substr(end, -1));
    }
    return ret;
}

} // namespace sorbet::core
