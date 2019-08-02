#ifndef SORBET_AUTOCORRECTSUGGESTION_H
#define SORBET_AUTOCORRECTSUGGESTION_H

#include "core/Loc.h"

namespace sorbet::core {

struct AutocorrectSuggestion {
    std::string title;
    std::vector<std::pair<core::Loc, std::string>> edits;

    AutocorrectSuggestion(std::string title, std::vector<std::pair<core::Loc, std::string>> edits)
        : title(title), edits(edits) {}
    static UnorderedMap<FileRef, std::string> apply(std::vector<AutocorrectSuggestion> autocorrects,
                                                    UnorderedMap<FileRef, std::string> sources);
};

} // namespace sorbet::core

#endif
