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
    static bool canSkipAutogen(core::GlobalState &gs, const std::string &cachePath,
                               const std::vector<std::string> &changedFiles);

    static AutogenCache unpackForFiles(std::string_view path, const UnorderedSet<std::string> &changedFiles);

    AutogenCache() = default;
    AutogenCache(AutogenCache &&) = default;
    AutogenCache(const AutogenCache &) = delete;
    AutogenCache &operator=(const AutogenCache &) = delete;

    void add(std::string path, unsigned int hash) {
        _constantHashMap.emplace(path, hash);
    };

    std::string pack() const;

    const UnorderedMap<std::string, unsigned int> &constantHashMap() const {
        return _constantHashMap;
    }
};
} // namespace sorbet::autogen
#endif // AUTOGEN_CACHE_H
