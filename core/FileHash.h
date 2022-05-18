#ifndef RUBY_TYPER_FILE_HASH_H
#define RUBY_TYPER_FILE_HASH_H
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

struct LocalSymbolTableHashes {
    static constexpr int HASH_STATE_NOT_COMPUTED = 0;
    static constexpr int HASH_STATE_NOT_COMPUTED_COLLISION_AVOID = 1;
    static constexpr int HASH_STATE_INVALID = 2;
    static constexpr int HASH_STATE_INVALID_COLLISION_AVOID = 3;
    // A fingerprint for all the symbols contained in the file.
    uint32_t hierarchyHash = HASH_STATE_NOT_COMPUTED;
    // A fingerprint for the classes and modules contained in the file.
    uint32_t classModuleHash = HASH_STATE_NOT_COMPUTED;
    // A fingerprint for the type argument symbols contained in the file.
    uint32_t typeArgumentHash = HASH_STATE_NOT_COMPUTED;
    // A fingerprint for the type member symbols contained in the file.
    uint32_t typeMemberHash = HASH_STATE_NOT_COMPUTED;
    // A fingerprint for the fields contained in the file.
    // TODO(froydnj) would maybe be interesting to split this out into separate
    // field/static field hashes, or even finer subdivisions on static fields.
    uint32_t fieldHash = HASH_STATE_NOT_COMPUTED;
    // A fingerprint for the methods contained in the file.
    uint32_t methodHash = HASH_STATE_NOT_COMPUTED;
    std::vector<std::pair<NameHash, uint32_t>> methodHashes;

    static uint32_t patchHash(uint32_t hash) {
        if (hash == LocalSymbolTableHashes::HASH_STATE_NOT_COMPUTED) {
            hash = LocalSymbolTableHashes::HASH_STATE_NOT_COMPUTED_COLLISION_AVOID;
        } else if (hash == LocalSymbolTableHashes::HASH_STATE_INVALID) {
            hash = LocalSymbolTableHashes::HASH_STATE_INVALID_COLLISION_AVOID;
        }
        return hash;
    }

    static LocalSymbolTableHashes invalid() {
        LocalSymbolTableHashes ret;
        ret.hierarchyHash = HASH_STATE_INVALID;
        ret.classModuleHash = HASH_STATE_INVALID;
        ret.typeArgumentHash = HASH_STATE_INVALID;
        ret.typeMemberHash = HASH_STATE_INVALID;
        ret.fieldHash = HASH_STATE_INVALID;
        ret.methodHash = HASH_STATE_INVALID;
        return ret;
    }

    bool isInvalid() const {
        DEBUG_ONLY(
            if (hierarchyHash == HASH_STATE_INVALID) {
                ENFORCE(classModuleHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID);
                ENFORCE(typeArgumentHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID);
                ENFORCE(typeMemberHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID);
                ENFORCE(fieldHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID);
                ENFORCE(methodHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID);
            } else {
                ENFORCE(classModuleHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID);
                ENFORCE(typeArgumentHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID);
                ENFORCE(typeMemberHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID);
                ENFORCE(fieldHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID);
                ENFORCE(methodHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID);
            });
        return hierarchyHash == HASH_STATE_INVALID;
    }
};

// This structure represents all the uses of various constructs contained in a single file.
struct UsageHash {
    // A sorted, deduplicated list of the hashes of all the method names called
    // by this file.
    std::vector<core::NameHash> sends;
    // A sorted, deduplicated list of the hashes of all the names referenced in this
    // file that will wind up referencing `core::Symbol` structures.  This includes
    // the obvious constant names (`T::Hash` counts as two constant names), but also
    // the names of the classes/methods defined in the file as well as any
    // @instance/@@class variables.
    std::vector<core::NameHash> symbols;
};

struct FileHash {
    LocalSymbolTableHashes localSymbolTableHashes;
    UsageHash usages;

    FileHash() = default;
    FileHash(LocalSymbolTableHashes &&localSymbolTableHashes, UsageHash &&usages);
};

}; // namespace sorbet::core

#endif // RUBY_TYPER_FILE_HASH_H
