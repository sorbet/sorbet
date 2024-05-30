#ifndef SORBET_AUTOCORRECTSUGGESTION_H
#define SORBET_AUTOCORRECTSUGGESTION_H

#include "common/FileSystem.h"

#include "core/Loc.h"

namespace sorbet::core {

struct AutocorrectSuggestion {
    struct Edit {
        core::Loc loc;
        std::string replacement;

        Edit() = default;
        Edit(core::Loc loc, std::string replacement) : loc(loc), replacement(move(replacement)) {}
    };

    const std::string title;
    std::vector<Edit> edits;

    bool isDidYouMean;

    AutocorrectSuggestion(std::string title, std::vector<Edit> edits, bool isDidYouMean = false)
        : title(title), edits(edits), isDidYouMean(isDidYouMean) {}

    // Apply a single `AutocorrectSuggestion` that contains either zero or one edits to a string, yielding the
    // autocorrected source.  This is useful for testing that an autocorrect does what we want.
    const std::string applySingleEditForTesting(const std::string_view source) const;

    // Reads all the files to be edited, and then accumulates all the edits that need to be applied
    // to those files into a resulting string with all edits applied. Does not write those back out
    // to disk.
    static UnorderedMap<FileRef, std::string> apply(const GlobalState &gs, FileSystem &fs,
                                                    const std::vector<AutocorrectSuggestion> &autocorrects);
};

} // namespace sorbet::core

#endif
