#ifndef RUBY_TYPER_NAME_HASH_H
#define RUBY_TYPER_NAME_HASH_H
#include "common/common.h"
namespace sorbet::core {
class NameRef;
class GlobalState;
class NameHash {
public:
    /** Sorts an array of NameHashes and removes duplicates. */
    static void sortAndDedupe(std::vector<core::NameHash> &hashes);

    NameHash(const GlobalState &gs, NameRef nm);
    inline bool isDefined() const {
        return _hashValue != 0;
    }
    NameHash(const NameHash &nm) = default;
    NameHash() : _hashValue(0){};
    inline bool operator==(const NameHash &rhs) const {
        ENFORCE(isDefined());
        ENFORCE(rhs.isDefined());
        return _hashValue == rhs._hashValue;
    }

    inline bool operator!=(const NameHash &rhs) const {
        return !(rhs == *this);
    }

    inline bool operator<(const NameHash &rhs) const {
        return this->_hashValue < rhs._hashValue;
    }

    uint32_t _hashValue;
};

template <typename H> H AbslHashValue(H h, const NameHash &m) {
    return H::combine(std::move(h), m._hashValue);
}

struct GlobalStateHash {
    static constexpr int HASH_STATE_NOT_COMPUTED = 0;
    static constexpr int HASH_STATE_NOT_COMPUTED_COLLISION_AVOID = 1;
    static constexpr int HASH_STATE_INVALID = 2;
    static constexpr int HASH_STATE_INVALID_COLLISION_AVOID = 3;
    uint32_t hierarchyHash = HASH_STATE_NOT_COMPUTED;
    std::vector<std::pair<NameHash, uint32_t>> methodHashes;
};

struct UsageHash {
    std::vector<core::NameHash> sends;
    std::vector<core::NameHash> symbols;
};

struct FileHash {
    GlobalStateHash definitions;
    UsageHash usages;

    FileHash() = default;
    FileHash(GlobalStateHash &&definitions, UsageHash &&usages);
};

}; // namespace sorbet::core

#endif // RUBY_TYPER_NAME_HASH_H
