#ifndef AUTOGEN_CACHE_H
#define AUTOGEN_CACHE_H

#include <string>
#include <vector>

#include "core/GlobalState.h"

namespace sorbet::autogen {
class AutogenCache {
    UnorderedMap<std::string, unsigned int> constantHashMap;

    AutogenCache(UnorderedMap<std::string, unsigned int> constantHashMap) : constantHashMap(constantHashMap) {};

    static AutogenCache unpack_for_files(std::string_view path, std::vector<std::string_view> changedFiles);

public:
    // This returns `true` when we have evidence that the set of changes in the changedFiles _definitely won't_ affect
    // the output of autogen. This means we can always be conservative: it's okay if this returns `false` in places
    // where it could return `true`, because that'll be correct but slower.
    static bool canSkipAutogen(core::GlobalState& gs, std::vector<std::string_view> changedFiles);

    AutogenCache() = default;
    void add(std::string path, unsigned int hash) {
        constantHashMap.emplace(path, hash);
    };

    std::string pack();
};
} // namespace sorbet::autogen
#endif // AUTOGEN_CACHE_H
