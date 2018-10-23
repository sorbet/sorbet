#ifndef SORBET_AUTOCORRECTSUGGESTION_H
#define SORBET_AUTOCORRECTSUGGESTION_H

#include "core/Loc.h"

namespace sorbet::core {

struct AutocorrectSuggestion {
    core::Loc loc;
    std::string replacement;

    AutocorrectSuggestion(Loc loc, std::string replacement) : loc(loc), replacement(replacement) {}
    static UnorderedMap<FileRef, std::string> apply(std::vector<AutocorrectSuggestion> autocorrects,
                                                    UnorderedMap<FileRef, std::string> sources);
};

} // namespace sorbet::core

#endif
