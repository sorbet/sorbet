#ifndef RUBY_TYPER_FILE_HASH_H
#define RUBY_TYPER_FILE_HASH_H
#include "common/common.h"
#include "core/ArityHash.h"
namespace sorbet::core {
class NameRef;
class GlobalState;
class FoundDefinitionRef;
namespace serialize {
class SerializerImpl;
}
class WithoutUniqueNameHash {
public:
    static void sortAndDedupe(std::vector<core::WithoutUniqueNameHash> &hashes);

    WithoutUniqueNameHash(const GlobalState &gs, NameRef nm);
    inline bool isDefined() const {
        return _hashValue != 0;
    }
    WithoutUniqueNameHash(const WithoutUniqueNameHash &nm) noexcept = default;
    WithoutUniqueNameHash() noexcept : _hashValue(0){};
    inline bool operator==(const WithoutUniqueNameHash &rhs) const noexcept {
        ENFORCE(isDefined());
        ENFORCE(rhs.isDefined());
        return _hashValue == rhs._hashValue;
    }

    inline bool operator!=(const WithoutUniqueNameHash &rhs) const noexcept {
        return !(rhs == *this);
    }

    inline bool operator<(const WithoutUniqueNameHash &rhs) const noexcept {
        return this->_hashValue < rhs._hashValue;
    }

    uint32_t _hashValue;
};

template <typename H> H AbslHashValue(H h, const WithoutUniqueNameHash &m) {
    return H::combine(std::move(h), m._hashValue);
}

class FullNameHash {
public:
    static void sortAndDedupe(std::vector<core::FullNameHash> &hashes);

    FullNameHash(const GlobalState &gs, NameRef nm);
    inline bool isDefined() const {
        return _hashValue != 0;
    }
    FullNameHash(const FullNameHash &nm) noexcept = default;
    FullNameHash() noexcept : _hashValue(0){};
    inline bool operator==(const FullNameHash &rhs) const noexcept {
        ENFORCE(isDefined());
        ENFORCE(rhs.isDefined());
        return _hashValue == rhs._hashValue;
    }

    inline bool operator!=(const FullNameHash &rhs) const noexcept {
        return !(rhs == *this);
    }

    inline bool operator<(const FullNameHash &rhs) const noexcept {
        return this->_hashValue < rhs._hashValue;
    }

    uint32_t _hashValue;
};

template <typename H> H AbslHashValue(H h, const FullNameHash &m) {
    return H::combine(std::move(h), m._hashValue);
}

struct SymbolHash {
    // The hash of the symbol's name. Note that symbols with the same name owned by different
    // symbols map to the same WithoutUniqueNameHash. This is fine, because our strategy for deciding which
    // downstream files to retypecheck is "any file that mentions any symbol with this name,"
    // regardless of which symbol(s) that name might refer to in that position.
    WithoutUniqueNameHash nameHash;
    // The combined hash of all symbols with the given nameHash. If this changes, it tells us that at
    // least one symbol with the given name changed in some way, ignoring nothing about the symbol.
    uint32_t symbolHash;

    SymbolHash() noexcept = default;
    SymbolHash(WithoutUniqueNameHash nameHash, uint32_t symbolHash) noexcept
        : nameHash(nameHash), symbolHash(symbolHash) {}

    inline bool operator<(const SymbolHash &h) const noexcept {
        return this->nameHash < h.nameHash || (!(h.nameHash < this->nameHash) && this->symbolHash < h.symbolHash);
    }
};

// 28 is the same as the size of an ID in FoundDefinitionRef::_storage
//
// `ownerIsSymbol` means that the ownerIdx stores a ClassOrModuleRef ID because the
// FoundDefinition::kind was Kind::Symbol.
// `!ownerIsSymbol` means that ownerIdx stores an index into the definedClasses list maintained by
// Namer::defineSymbols.
#define FOUND_HASH_OWNER_INFO()             \
private:                                    \
    friend class serialize::SerializerImpl; \
    uint32_t ownerIdx : 28;                 \
    bool ownerIsSymbol : 1;                 \
                                            \
public:                                     \
    FoundDefinitionRef owner() const;

struct FoundStaticFieldHash {
    // The owner of this static field (index into namer's definedClasses vector)
    FOUND_HASH_OWNER_INFO();
    // Hash of this static field's name
    const FullNameHash nameHash;

    FoundStaticFieldHash(uint32_t ownerIdx, bool ownerIsSymbol, FullNameHash nameHash)
        : ownerIdx(ownerIdx), ownerIsSymbol(ownerIsSymbol), nameHash(nameHash) {
        sanityCheck();
    }

    void sanityCheck() const;

    // Debug string
    std::string toString() const;
};
CheckSize(FoundStaticFieldHash, 8, 4);

using FoundStaticFieldHashes = std::vector<FoundStaticFieldHash>;

struct FoundTypeMemberHash {
    // The owner of this type member (index into namer's definedClasses vector)
    FOUND_HASH_OWNER_INFO();

    // If this is true, then the actual owner is the singleton class of the class pointed to by `idx`
    bool isTypeTemplate : 1;

    // Hash of this type member's name
    const FullNameHash nameHash;

    FoundTypeMemberHash(uint32_t ownerIdx, bool ownerIsSymbol, bool isTypeTemplate, FullNameHash nameHash)
        : ownerIdx(ownerIdx), ownerIsSymbol(ownerIsSymbol), isTypeTemplate(isTypeTemplate), nameHash(nameHash) {
        sanityCheck();
    }

    void sanityCheck() const;

    // Debug string
    std::string toString() const;
};
CheckSize(FoundTypeMemberHash, 8, 4);

using FoundTypeMemberHashes = std::vector<FoundTypeMemberHash>;

// A fingerprint of a FoundDefinition suitable for storing on a File.
// Allows a current Namer run to reference information about a previous Namer run.
struct FoundMethodHash {
    // The owner of this method.
    FOUND_HASH_OWNER_INFO();

    // Whether the method was defined as an instance method or a singleton class method
    bool useSingletonClass : 1;

    // Hash of this method's name
    const FullNameHash nameHash;

    // A hash of the method's arity
    ArityHash arityHash;

    FoundMethodHash(uint32_t ownerIdx, bool ownerIsSymbol, bool useSingletonClass, FullNameHash nameHash,
                    ArityHash arityHash)
        : ownerIdx(ownerIdx), ownerIsSymbol(ownerIsSymbol), useSingletonClass(useSingletonClass), nameHash(nameHash),
          arityHash(arityHash) {
        sanityCheck();
    };

    void sanityCheck() const;

    // Debug string
    std::string toString() const;
};
CheckSize(FoundMethodHash, 12, 4);

using FoundMethodHashes = std::vector<FoundMethodHash>;

struct FoundFieldHash {
    // The owner of this field.
    FOUND_HASH_OWNER_INFO();
    // Whether the field was defined on instances of the class or on the singleton class.
    bool onSingletonClass : 1;
    // Whether the field was a class or instance variable.
    // TODO(froydnj) we should just subsume class variables into the more
    // general static fields, since that's how we represent them internally.
    bool isInstanceVariable : 1;
    // Whether the definition of the field comes from inside a method.
    bool fromWithinMethod : 1;

    // Hash of this field's name.
    const FullNameHash nameHash;

    FoundFieldHash(uint32_t ownerIdx, bool ownerIsSymbol, bool onSingletonClass, bool isInstanceVariable,
                   bool fromWithinMethod, FullNameHash nameHash)
        : ownerIdx(ownerIdx), ownerIsSymbol(ownerIsSymbol), onSingletonClass(onSingletonClass),
          isInstanceVariable(isInstanceVariable), fromWithinMethod(fromWithinMethod), nameHash(nameHash) {
        sanityCheck();
    }

    void sanityCheck() const;

    // Debug string
    std::string toString() const;
};
CheckSize(FoundFieldHash, 8, 4);

using FoundFieldHashes = std::vector<FoundFieldHash>;

struct FoundDefHashes {
    FoundStaticFieldHashes staticFieldHashes;
    FoundTypeMemberHashes typeMemberHashes;
    FoundMethodHashes methodHashes;
    FoundFieldHashes fieldHashes;
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
    uint32_t fieldHash = HASH_STATE_NOT_COMPUTED;
    // A fingerprint for the non-class alias static fields contained in the file.
    uint32_t staticFieldHash = HASH_STATE_NOT_COMPUTED;
    // A fingerprint for the class alias static fields contained in the file.
    uint32_t classAliasHash = HASH_STATE_NOT_COMPUTED;
    // A fingerprint for the methods contained in the file.
    uint32_t methodHash = HASH_STATE_NOT_COMPUTED;

    // SymbolHash is a pair of WithoutUniqueNameHash and uint32_t, which makes this vector
    // essentially a map from WithoutUniqueNameHash -> uint32_t, where keys are names of symbols and
    // values are Symbol hashes for all symbols defined in the file with that name (on any owner).
    // Since WithoutUniqueNameHash is sensitive to constant name kinds, `class Foo` and `def Foo`
    // are different names.
    //
    // It's stored as a vector instead of a map to optimize for set_difference and compact storage
    // representation.
    //
    // The hierarchyHash stores only enough information to know whether to take the fast path or not.
    // These symbol hashes store enough to know whether _anything_ changed, which lets us use a set
    // difference to know the name hashes of any symbols that changed in any way, so that files
    // mentioning them can be retypechecked.
    std::vector<SymbolHash> retypecheckableSymbolHashes;

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
        ret.staticFieldHash = HASH_STATE_INVALID_PARSE;
        ret.classAliasHash = HASH_STATE_INVALID_PARSE;
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
                ENFORCE(staticFieldHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(classAliasHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(methodHash == core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
            } else {
                ENFORCE(classModuleHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(typeArgumentHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(typeMemberHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(fieldHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(staticFieldHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(classAliasHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
                ENFORCE(methodHash != core::LocalSymbolTableHashes::HASH_STATE_INVALID_PARSE);
            });
        return hierarchyHash == HASH_STATE_INVALID_PARSE;
    }
};

// This structure represents every time a name was used in a place where it could be referencing the
// name of a (Sorbet) symbol. For example, this program:
//
//     self.foo()
//     @bar
//     Qux
//     :example
//
// references some (possibly non-existent) symbols with the names `foo`, `@bar`, and `Qux` (but
// _not_ `:example`, because that's a Ruby `Symbol` literal not a Sorbet symbol).
//
// These hashes are used for quickly lowering the upper bound on the set of files that might need to
// be retypechecked when there is a fast path edit or when there is a find-all-references request.
//
// (Useful for _over_ approximating the set of files that might be affected.)
struct UsageHash {
    std::vector<core::WithoutUniqueNameHash> nameHashes;
};

// This is stored on the core::File object directly, which is then cached.
//
// It's important that nothing in this structure (including transitively) contains information
// that's specific to a particular GlobalState. This is why, e.g., all core::NameRefs are hashed
// instead of storing their IDs. FileHash contains FoundDefinitionRef IDs transitively, but those
// IDs auto-increment from 0 based on the order of definitions in the file, regardless of the
// contents of a GlobalState.
struct FileHash {
    LocalSymbolTableHashes localSymbolTableHashes;
    UsageHash usages;
    FoundDefHashes foundHashes;

    FileHash() = default;
    FileHash(LocalSymbolTableHashes &&localSymbolTableHashes, UsageHash &&usages, FoundDefHashes &&foundHashes);
};

}; // namespace sorbet::core

#endif // RUBY_TYPER_FILE_HASH_H
