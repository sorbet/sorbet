#ifndef RUBY_TYPER_FILE_HASH_H
#define RUBY_TYPER_FILE_HASH_H
#include "common/common.h"
namespace sorbet::core {
class NameRef;
class GlobalState;
class ShortNameHash {
public:
    /** Sorts an array of ShortNameHashes and removes duplicates. */
    static void sortAndDedupe(std::vector<core::ShortNameHash> &hashes);

    ShortNameHash(const GlobalState &gs, NameRef nm);
    inline bool isDefined() const {
        return _hashValue != 0;
    }
    ShortNameHash(const ShortNameHash &nm) noexcept = default;
    ShortNameHash() noexcept : _hashValue(0){};
    inline bool operator==(const ShortNameHash &rhs) const noexcept {
        ENFORCE(isDefined());
        ENFORCE(rhs.isDefined());
        return _hashValue == rhs._hashValue;
    }

    inline bool operator!=(const ShortNameHash &rhs) const noexcept {
        return !(rhs == *this);
    }

    inline bool operator<(const ShortNameHash &rhs) const noexcept {
        return this->_hashValue < rhs._hashValue;
    }

    uint32_t _hashValue;
};

template <typename H> H AbslHashValue(H h, const ShortNameHash &m) {
    return H::combine(std::move(h), m._hashValue);
}

struct SymbolHash {
    // The hash of the symbol's name. Note that symbols with the same name owned by different
    // symbols map to the same ShortNameHash. This is fine, because our strategy for deciding which
    // downstream files to retypecheck is "any file that mentions any method with this name,"
    // regardless of which method symbol(s) that call might dispatch to.
    ShortNameHash nameHash;
    // The combined hash of all method symbols with the given nameHash. If this changes, it tells us
    // that at least one method symbol with the given name changed in some way, including type
    // information.
    uint32_t symbolHash;

    SymbolHash() noexcept = default;
    SymbolHash(ShortNameHash nameHash, uint32_t symbolHash) noexcept : nameHash(nameHash), symbolHash(symbolHash) {}

    inline bool operator<(const SymbolHash &h) const noexcept {
        return this->nameHash < h.nameHash || (!(h.nameHash < this->nameHash) && this->symbolHash < h.symbolHash);
    }
};

// When a file is edited, we run index and resolve it using an local (empty) GlobalState.
// We then hash the symbols defined in that local GlobalState, and use the result to quickly decide
// whether "something" changed, or whether nothing changed (and thus we can take the fast path).
//
// The something/nothing decision is tracked by hierarchyHash.
//
// The other things in this structure are either used for metrics purposes only, or for special
// cases for e.g. when there are "no" changes, except changes to method types.
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

    // Essentially a map from ShortNameHash -> uint32_t, where keys are names of methods and values are
    // Symbol hashes for all methods defined in the file with that name (on any owner).
    //
    // Stored as a vector instead of a map to optimize for set_difference and compact storage
    // representation.
    //
    // While the hierarchyHash stores only the methodShapeHash of the method symbol (which ignores
    // things like types for the fast/slow path decision), this stores the complete method symbol
    // hash, so that if anything including types change for a method we know what their names are.
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
    std::vector<core::ShortNameHash> sends;
    // A sorted, deduplicated list of the hashes of all the names referenced in this
    // file that will wind up referencing `core::Symbol` structures.  This includes
    // the obvious constant names (`T::Hash` counts as two constant names), but also
    // the names of the classes/methods defined in the file as well as any
    // @instance/@@class variables.
    std::vector<core::ShortNameHash> symbols;
};

struct FileHash {
    LocalSymbolTableHashes localSymbolTableHashes;
    UsageHash usages;

    FileHash() = default;
    FileHash(LocalSymbolTableHashes &&localSymbolTableHashes, UsageHash &&usages);
};

}; // namespace sorbet::core

#endif // RUBY_TYPER_FILE_HASH_H
