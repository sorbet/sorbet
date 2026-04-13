#ifndef SORBET_AUTOCORRECTSUGGESTION_H
#define SORBET_AUTOCORRECTSUGGESTION_H

#include "common/FileSystem.h"

#include "core/Loc.h"

namespace sorbet::core {

struct AutocorrectSuggestion {
    struct Edit {
        core::Loc loc;
        std::string replacement;
    };

    const std::string title;
    std::vector<Edit> edits;

    bool isDidYouMean;

    bool shouldSkipWhenAggregated;

    AutocorrectSuggestion(std::string title, std::vector<Edit> edits, bool isDidYouMean = false,
                          bool shouldSkipWhenAggregated = false)
        : title(title), edits(edits), isDidYouMean(isDidYouMean), shouldSkipWhenAggregated(shouldSkipWhenAggregated) {}

    // Reads all the files to be edited, and then accumulates all the edits that need to be applied
    // to those files into a resulting string with all edits applied. Does not write those back out
    // to disk.
    static UnorderedMap<FileRef, std::string> apply(const GlobalState &gs, FileSystem &fs,
                                                    const std::vector<AutocorrectSuggestion> &autocorrects);
    // Given a list of edits, sorts them by loc and replacement string, and then merges edits that insert at the same
    // location into one edit.
    // Ex. [{locA, "b"}, {locA, "a"}] will be combined to [{locA, "ab"}]
    static void mergeAdjacentEdits(std::vector<core::AutocorrectSuggestion::Edit> &edits);
};

} // namespace sorbet::core

#endif
