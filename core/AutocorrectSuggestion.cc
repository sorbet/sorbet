#include "AutocorrectSuggestion.h"
#include <unordered_set>

using namespace std;

namespace sorbet {
namespace core {

bool hasSeen(const unordered_set<Loc> &seen, Loc loc) {
    for (auto &seenLoc : seen) {
        if (seenLoc.beginPos >= loc.beginPos && seenLoc.beginPos < loc.endPos) {
            return true;
        }
        if (seenLoc.endPos >= loc.beginPos && seenLoc.endPos < loc.endPos) {
            return true;
        }
    }
    return false;
}

map<FileRef, string> AutocorrectSuggestion::apply(vector<AutocorrectSuggestion> autocorrects,
                                                  map<FileRef, string> sources) {
    // Sort the locs backwards
    auto compare = [](AutocorrectSuggestion &left, AutocorrectSuggestion &right) {
        if (left.loc.file != right.loc.file) {
            return left.loc.file.id() > right.loc.file.id();
        }

        auto a = left.loc.beginPos;
        auto b = right.loc.beginPos;
        if (a != b) {
            return a > b;
        }

        return false;
    };
    sort(autocorrects.begin(), autocorrects.end(), compare);

    unordered_set<Loc> seen; // used to make sure nothing overlaps
    map<FileRef, string> ret;
    for (auto &autocorrect : autocorrects) {
        auto &loc = autocorrect.loc;
        if (!ret.count(loc.file)) {
            ret[loc.file] = sources[loc.file];
        }
        auto source = ret[loc.file];
        auto start = loc.beginPos;
        auto end = loc.endPos;

        if (hasSeen(seen, loc)) {
            continue;
        }
        seen.emplace(loc);
        ret[loc.file] = string(source.substr(0, start)) + autocorrect.replacement + string(source.substr(end, -1));
    }
    return ret;
}

} // namespace core
} // namespace sorbet
