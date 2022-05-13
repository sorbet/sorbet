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
    NameHash(const NameHash &nm) noexcept = default;
    NameHash() noexcept : _hashValue(0){};
    inline bool operator==(const NameHash &rhs) const noexcept {
        ENFORCE(isDefined());
        ENFORCE(rhs.isDefined());
        return _hashValue == rhs._hashValue;
    }

    inline bool operator!=(const NameHash &rhs) const noexcept {
        return !(rhs == *this);
    }

    inline bool operator<(const NameHash &rhs) const noexcept {
        return this->_hashValue < rhs._hashValue;
    }

    uint32_t _hashValue;
};

template <typename H> H AbslHashValue(H h, const NameHash &m) {
    return H::combine(std::move(h), m._hashValue);
}

struct DefinitionHash {
    static constexpr int HASH_STATE_NOT_COMPUTED = 0;
    static constexpr int HASH_STATE_NOT_COMPUTED_COLLISION_AVOID = 1;
    static constexpr int HASH_STATE_INVALID = 2;
    static constexpr int HASH_STATE_INVALID_COLLISION_AVOID = 3;
    uint32_t hierarchyHash = HASH_STATE_NOT_COMPUTED;
    uint32_t classModuleHash = HASH_STATE_NOT_COMPUTED;
    uint32_t typeArgumentHash = HASH_STATE_NOT_COMPUTED;
    uint32_t typeMemberHash = HASH_STATE_NOT_COMPUTED;
    // TODO(froydnj) would maybe be interesting to split this out into separate
    // field/static field hashes, or even finer subdivisions on static fields.
    uint32_t fieldHash = HASH_STATE_NOT_COMPUTED;
    uint32_t methodHash = HASH_STATE_NOT_COMPUTED;
    std::vector<std::pair<NameHash, uint32_t>> methodHashes;

    static DefinitionHash invalid() {
        DefinitionHash ret;
        ret.hierarchyHash = HASH_STATE_INVALID;
        ret.classModuleHash = HASH_STATE_INVALID;
        ret.typeArgumentHash = HASH_STATE_INVALID;
        ret.typeMemberHash = HASH_STATE_INVALID;
        ret.fieldHash = HASH_STATE_INVALID;
        ret.methodHash = HASH_STATE_INVALID;
        return ret;
    }
};

struct UsageHash {
    std::vector<core::NameHash> sends;
    std::vector<core::NameHash> symbols;
};

struct FileHash {
    DefinitionHash definitions;
    UsageHash usages;

    FileHash() = default;
    FileHash(DefinitionHash &&definitions, UsageHash &&usages);
};

}; // namespace sorbet::core

#endif // RUBY_TYPER_NAME_HASH_H
