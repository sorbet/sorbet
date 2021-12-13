#ifndef RUBY_TYPER_NAMEREF_H
#define RUBY_TYPER_NAMEREF_H
#include "common/common.h"
#include "core/DebugOnlyCheck.h"
namespace sorbet::core {
class GlobalState;
class NameSubstitution;
struct UniqueName;
struct ConstantName;
struct UTF8Name;

// If you add NameKinds, make sure NameRef::KIND_BITS is kept in sync!
enum class NameKind : uint8_t {
    UTF8 = 1,
    UNIQUE = 2,
    CONSTANT = 3,
};

CheckSize(NameKind, 1, 1);

struct NameDataDebugCheck {
    const GlobalState &gs;
    const unsigned int nameCountAtCreation;

    NameDataDebugCheck(const GlobalState &gs);
    void check() const;
};

/** This is to `NameRef &` what SymbolData is to `SymbolRef &`. Read docs on SymbolData */
class UniqueNameData : private DebugOnlyCheck<NameDataDebugCheck> {
    const UniqueName &name;

public:
    UniqueNameData(const UniqueName &ref, const GlobalState &gs);
    const UniqueName *operator->() const;
};

class ConstantNameData : private DebugOnlyCheck<NameDataDebugCheck> {
    const ConstantName &name;

public:
    ConstantNameData(const ConstantName &ref, const GlobalState &gs);
    const ConstantName *operator->() const;
};

class UTF8NameData : private DebugOnlyCheck<NameDataDebugCheck> {
    const UTF8Name &name;

public:
    UTF8NameData(const UTF8Name &ref, const GlobalState &gs);
    const UTF8Name *operator->() const;
};

constexpr size_t sizeof__UniqueName = sizeof(UniqueName *);
constexpr size_t alignof__UniqueName = alignof(UniqueName *);
CheckSize(UniqueNameData, sizeof__UniqueName, alignof__UniqueName);

struct NameRefDebugCheck {
    int globalStateId;

    constexpr NameRefDebugCheck() : globalStateId(-1) {}

    NameRefDebugCheck(const GlobalState &gs, NameKind kind, uint32_t id);

    void check(const GlobalState &gs, NameKind kind, uint32_t id) const;
    void check(const NameSubstitution &subst) const;
};

constexpr std::string_view PACKAGE_SUFFIX = "_Package";
constexpr std::string_view PACKAGE_PRIVATE_SUFFIX = "_Package_Private";

class NameRef final : private DebugOnlyCheck<NameRefDebugCheck> {
private:
    // NameKind takes up this many bits in _id.
    static constexpr uint32_t KIND_BITS = 2;
    static constexpr uint32_t ID_BITS = 32 - KIND_BITS;
    static constexpr uint32_t KIND_MASK = (1 << KIND_BITS) - 1;
    static constexpr uint32_t MAX_ID = (1 << ID_BITS) - 1;

    uint32_t _id;

    uint32_t unsafeTableIndex() const {
        return _id >> KIND_BITS;
    }

public:
    // The value `0` implies that there is no NameKind tag present (NameKind begin at 1) and is a special-value used to
    // indicate a non-existant NameRef.
    constexpr NameRef() : _id(0){};

    // WellKnown is a tag to statically indicate that the caller is deliberately
    // constructing a well-known name, whose ID is stable across all
    // GlobalStates. This should never be used outside of the name constructors
    // generated by tools/generate_names.cc
    struct WellKnown {};

    constexpr NameRef(WellKnown, NameKind kind, uint32_t id) : _id{(id << KIND_BITS) | static_cast<uint32_t>(kind)} {
        // If this fails, the symbol table is too big :(
        ENFORCE_NO_TIMER(id <= MAX_ID);
    }

    NameRef(const GlobalState &gs, NameKind kind, uint32_t id);

    NameRef(const GlobalState &gs, NameRef ref);

    NameRef(const NameRef &nm) = default;

    NameRef(NameRef &&nm) = default;

    static NameRef fromRaw(const GlobalState &gs, uint32_t raw) {
        return NameRef(gs, NameRef::fromRawUnchecked(raw));
    }

    static NameRef fromRawUnchecked(uint32_t raw) {
        auto ref = NameRef();
        ref._id = raw;
        return ref;
    }

    NameRef &operator=(const NameRef &rhs) = default;

    bool operator==(const NameRef &rhs) const {
        return _id == rhs._id;
    }

    bool operator!=(const NameRef &rhs) const {
        return !(rhs == *this);
    }

    constexpr uint32_t rawId() const {
        return _id;
    }

    uint32_t utf8Index() const {
        ENFORCE_NO_TIMER(kind() == NameKind::UTF8);
        return unsafeTableIndex();
    }

    uint32_t uniqueIndex() const {
        ENFORCE_NO_TIMER(kind() == NameKind::UNIQUE);
        return unsafeTableIndex();
    }

    uint32_t constantIndex() const {
        ENFORCE_NO_TIMER(kind() == NameKind::CONSTANT);
        return unsafeTableIndex();
    }

    constexpr NameKind kind() const {
        auto nameKind = static_cast<NameKind>(_id & KIND_MASK);
        ENFORCE_NO_TIMER(static_cast<uint8_t>(nameKind) > 0);
        return nameKind;
    }

    /* Names are one of three different kinds: unique, utf8, or constant. */

    const UniqueNameData dataUnique(const GlobalState &gs) const;
    const UTF8NameData dataUtf8(const GlobalState &gs) const;
    const ConstantNameData dataCnst(const GlobalState &gs) const;

    // Returns the `0` NameRef, used to indicate non-existence of a name
    static constexpr NameRef noName() {
        return NameRef();
    }

    inline bool exists() const {
        return _id != 0;
    }

    NameRef addEq(GlobalState &gs) const;
    NameRef addQuestion(GlobalState &gs) const;

    NameRef addAt(GlobalState &gs) const;
    NameRef lookupWithAt(const GlobalState &gs) const;

    NameRef prepend(GlobalState &gs, std::string_view s) const;

    NameRef lookupMangledPrivatePackageName(const GlobalState &gs) const;

    bool isClassName(const GlobalState &gs) const;

    // Convenience method, because enums need to be special cased in more places than other kinds of
    // unique names, and everyone always forget to unwrap the first layer (NameKind::CONSTANT)
    // before checking for UniqueNameKind::TEnum.
    bool isTEnumName(const GlobalState &gs) const;

    // Convenience method to avoid forgeting to unwrap the first layer (NameKind::CONSTANT)
    // before checking for UniqueNameKind::Packager.
    bool isPackagerName(const GlobalState &gs) const;
    bool isPackagerPrivateName(const GlobalState &gs) const;

    bool isValidConstantName(const GlobalState &gs) const;

    std::string_view shortName(const GlobalState &gs) const;
    std::string showRaw(const GlobalState &gs) const;
    std::string toString(const GlobalState &gs) const;
    std::string show(const GlobalState &gs) const;

    void enforceCorrectGlobalState(const GlobalState &gs) const;
    void sanityCheckSubstitution(const NameSubstitution &subst) const;
    void sanityCheck(const GlobalState &gs) const;
};
CheckSize(NameRef, 4, 4);

template <typename H> H AbslHashValue(H h, const NameRef &m) {
    return H::combine(std::move(h), m.rawId());
}
} // namespace sorbet::core

#endif // RUBY_TYPER_NAMEREF_H
