#ifndef AUTOGEN_CACHE_H
#define AUTOGEN_CACHE_H

#include <string>
#include <vector>

#include "core/GlobalState.h"

namespace sorbet::autogen {
class AutogenCache {
    UnorderedMap<std::string, unsigned int> _constantHashMap;


public:
    // This returns `true` when we have evidence that the set of changes in the changedFiles _definitely won't_ affect
    // the output of autogen. This means we can always be conservative: it's okay if this returns `false` in places
    // where it could return `true`, because that'll be correct but slower.
    static bool canSkipAutogen(core::GlobalState& gs, std::string_view cachePath, std::vector<std::string>& changedFiles);

    static AutogenCache unpackForFiles(std::string_view path, UnorderedSet<std::string>& changedFiles);
    static AutogenCache merge(std::vector<AutogenCache>& caches) {
        AutogenCache cache;

        for (auto &c : caches) {
            cache._constantHashMap.insert(c.constantHashMap().begin(), c.constantHashMap().end());
        }

        return cache;
    }

    AutogenCache(UnorderedMap<std::string, unsigned int> constantHashMap) : _constantHashMap(constantHashMap) {};
    AutogenCache() = default;

    void add(std::string path, unsigned int hash) {
        _constantHashMap.emplace(path, hash);
    };

    std::string pack();

    const UnorderedMap<std::string, unsigned int>& constantHashMap() const {
        return _constantHashMap;
    }
};
} // namespace sorbet::autogen
#endif // AUTOGEN_CACHE_H
