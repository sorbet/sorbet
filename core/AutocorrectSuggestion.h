#ifndef SORBET_AUTOCORRECTSUGGESTION_H
#define SORBET_AUTOCORRECTSUGGESTION_H

#include "core/Loc.h"
#include <map>

namespace sorbet::core {

struct AutocorrectSuggestion {
    core::Loc loc;
    std::string replacement;

    AutocorrectSuggestion(Loc loc, std::string replacement) : loc(loc), replacement(replacement) {}
    static std::map<FileRef, std::string> apply(std::vector<AutocorrectSuggestion> autocorrects,
                                                std::map<FileRef, std::string> sources);
};

} // namespace sorbet::core

#endif
