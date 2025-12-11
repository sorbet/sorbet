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

    std::optional<std::string> deDupKey;

    AutocorrectSuggestion(std::string title, std::vector<Edit> edits, bool isDidYouMean = false,
                          std::optional<std::string> deDupKey = std::nullopt)
        : title(title), edits(edits), isDidYouMean(isDidYouMean), deDupKey(deDupKey) {}

    // Reads all the files to be edited, and then accumulates all the edits that need to be applied
    // to those files into a resulting string with all edits applied. Does not write those back out
    // to disk.
    static UnorderedMap<FileRef, std::string> apply(const GlobalState &gs, FileSystem &fs,
                                                    const std::vector<AutocorrectSuggestion> &autocorrects);
};

} // namespace sorbet::core

#endif
