#ifndef SORBET_AUTOCORRECTSUGGESTION_H
#define SORBET_AUTOCORRECTSUGGESTION_H

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
    static UnorderedMap<FileRef, std::string> apply(std::vector<AutocorrectSuggestion> autocorrects,
                                                    UnorderedMap<FileRef, std::string> sources);
};

} // namespace sorbet::core

#endif
