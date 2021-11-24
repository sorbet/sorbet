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

    AutocorrectSuggestion(std::string title, std::vector<Edit> edits) : title(title), edits(edits) {}

    // Apply a single `AutocorrectSuggestion` to a string, yielding the autocorrected source
    std::string apply(const std::string_view source);

    // Reads all the files to be edited, and then accumulates all the edits that need to be applied
    // to those files into a resulting string with all edits applied. Does not write those back out
    // to disk.
    static UnorderedMap<FileRef, std::string> apply(const GlobalState &gs, FileSystem &fs,
                                                    const std::vector<AutocorrectSuggestion> &autocorrects);
};

} // namespace sorbet::core

#endif
