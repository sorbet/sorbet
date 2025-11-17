#include "core/AutocorrectSuggestion.h"
#include "absl/strings/str_cat.h"
#include "common/sort/sort.h"

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

void AutocorrectSuggestion::mergeAdjacentEdits(std::vector<core::AutocorrectSuggestion::Edit> &edits) {
    fast_sort(edits, [](const auto &lhs, const auto &rhs) {
        if (lhs.loc == rhs.loc) {
            return lhs.replacement < rhs.replacement;
        }
        return lhs.loc.beginPos() < rhs.loc.beginPos();
    });
    auto i = 0;
    while (edits.size() > 0 && i < edits.size() - 1) {
        if (edits[i].loc.beginPos() == edits[i + 1].loc.beginPos() && edits[i].loc.empty() &&
            edits[i + 1].loc.empty()) {
            // If we're inserting 2 edits at the same location, combine them into a single edit.
            // TODO(neil): maybe this logic should be moved/added to AutocorrectSuggestion::apply, to handle other
            // cases where 2 autocorrects add at the same loc?
            edits[i].replacement += edits[i + 1].replacement;
            edits.erase(edits.begin() + i + 1);
        } else {
            i++;
        }
    }
}

UnorderedMap<FileRef, string> AutocorrectSuggestion::apply(const GlobalState &gs, FileSystem &fs,
                                                           const vector<AutocorrectSuggestion> &autocorrects) {
    UnorderedMap<FileRef, string> sources;
    for (auto &autocorrect : autocorrects) {
        for (auto &edit : autocorrect.edits) {
            auto file = edit.loc.file();
            if (!sources.count(file)) {
                sources[file] = fs.readFile(string(file.data(gs).path()));
            }
        }
    }

    vector<AutocorrectSuggestion::Edit> edits;
    for (auto &autocorrect : autocorrects) {
        move(autocorrect.edits.begin(), autocorrect.edits.end(), back_inserter(edits));
    }

    // Sort the locs backwards
    auto compare = [](const AutocorrectSuggestion::Edit &left, const AutocorrectSuggestion::Edit &right) {
        if (left.loc.file() != right.loc.file()) {
            return left.loc.file() > right.loc.file();
        }

        auto a = left.loc.beginPos();
        auto b = right.loc.beginPos();
        if (a != b) {
            return a > b;
        }

        return false;
    };
    fast_sort(edits, compare);

    UnorderedSet<Loc> seen; // used to make sure nothing overlaps
    UnorderedMap<FileRef, string> ret;
    for (auto &edit : edits) {
        core::Loc loc = edit.loc;
        ENFORCE(loc.exists(), "Can't apply autocorrect when Loc doesn't exist");
        if (!loc.exists()) {
            // Recover gracefully even if ENFORCE fails.
            continue;
        }

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
        ret[loc.file()] = absl::StrCat(source.substr(0, start), edit.replacement, source.substr(end, -1));
    }
    return ret;
}

} // namespace sorbet::core
