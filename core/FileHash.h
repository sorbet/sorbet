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

struct SymbolHash {
    // The hash of the symbol's name. Note that symbols with the same name owned by different
    // symbols map to the same NameHash. This is fine, because our strategy for deciding which
    // downstream files to retypecheck is "any file that mentions any method with this name,"
    // regardless of which method symbol(s) that call might dispatch to.
    NameHash nameHash;
    // The combined hash of all method symbols with the given nameHash. If this changes, it tells us
    // that at least one method symbol with the given name changed in some way, including type
    // information.
    uint32_t symbolHash;

    SymbolHash() noexcept = default;
    SymbolHash(NameHash nameHash, uint32_t symbolHash) noexcept : nameHash(nameHash), symbolHash(symbolHash) {}

    inline bool operator<(const SymbolHash &h) const noexcept {
        return this->nameHash < h.nameHash || (!(h.nameHash < this->nameHash) && this->symbolHash < h.symbolHash);
    }
};

struct LocalSymbolTableHashes {
    // Default value of hierarchyHash (and other hashes) tracked by this class.
    static constexpr int HASH_STATE_NOT_COMPUTED = 0;
    // Since something could naturally hash to `HASH_STATE_NOT_COMPUTED`, we force these hash
    // results to collide with all things that hashed to `1`.
    static constexpr int HASH_STATE_NOT_COMPUTED_COLLISION_AVOID = 1;
    // Indicates that Sorbet completely failed to parse the file (there were parse errors and the
    // parse result was completely empty), therefore the LocalSymbolTableHashes are meaningless.
    //
    // While this state is not strictly necessary, it is useful for tracking metrics (want to know
    // how many times a user actually changed definitions vs how many times they changed because of
    // a shortcoming in Sorbet's parser implementation.)
    static constexpr int HASH_STATE_INVALID_PARSE = 2;
    // Since something could naturally hash to `HASH_STATE_INVALID_PARSE`, we force these hash
    // results to collide with all things that hashed to `3`.
    static constexpr int HASH_STATE_INVALID_PARSE_COLLISION_AVOID = 3;
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

    // Essentially a map from NameHash -> uint32_t, where keys are names of methods and values are
    // Symbol hashes for all methods defined in the file with that name (on any owner).
    //
    // Stored as a vector instead of a map to optimize for set_difference and compact storage
    // representation.
    std::vector<SymbolHash> methodHashes;

    static uint32_t patchHash(uint32_t hash) {
        if (hash == LocalSymbolTableHashes::HASH_STATE_NOT_COMPUTED) {
            hash = LocalSymbolTableHashes::HASH_STATE_NOT_COMPUTED_COLLISION_AVOID;
        } else if (hash == LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE) {
            hash = LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE_COLLISION_AVOID;
        }
        return hash;
    }

    static LocalSymbolTableHashes invalidParse() {
        LocalSymbolTableHashes ret;
        ret.hierarchyHash = HASH_STATE_INVALID_PARSE;
        ret.classModuleHash = HASH_STATE_INVALID_PARSE;
        ret.typeArgumentHash = HASH_STATE_INVALID_PARSE;
        ret.typeMemberHash = HASH_STATE_INVALID_PARSE;
        ret.fieldHash = HASH_STATE_INVALID_PARSE;
        ret.methodHash = HASH_STATE_INVALID_PARSE;
        return ret;
    }

    bool isInvalidParse() const {
        DEBUG_ONLY(
            if (hierarchyHash == HASH_STATE_INVALID_PARSE) {
                ENFORCE(classModuleHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(typeArgumentHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(typeMemberHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(fieldHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(methodHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
            } else {
                ENFORCE(classModuleHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(typeArgumentHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(typeMemberHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(fieldHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(methodHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
            });
        return hierarchyHash == HASH_STATE_INVALID_PARSE;
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
