#include "core/FileHash.h"
#include "common/sort/sort.h"
#include "core/FoundDefinitions.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/hashing/hashing.h"

using namespace std;
namespace sorbet::core {
namespace {

// We use 0 to signal whether the hash is defined (see isDefined in the header)
uint32_t incZero(uint32_t a) {
    return a == 0 ? 1 : a;
};

// We ignore unique names because those are names created by Sorbet not written by the user. If a
// symbol with a unique name changes (like an overload, or a T::Enum name, or a Singleton class), we
// want to force the fast path to retypecheck any file that mentioned the original name in any way
//
// For example, `A#foo (overload.1)` changes, that means we should typecheck all files with `foo`,
// because the that `foo` might end up dispatching to the unique name, despite not mentioning the
// unique name directly.
//
// Similarly for constant names--changes to the the fake `MyEnum::X$1` symbols we create to model
// `T::Enum` subclasses could affect a file that simply mentions a constant with the name `X` (but
// not a file that mentions a method called `X()`, which is why we still include the name kind in
// the hash for non-UNIQUE names).
uint32_t hashNameRefWithoutUniques(const GlobalState &gs, NameRef nm) {
    uint32_t result;
    auto kind = nm.kind();

    switch (kind) {
        case NameKind::UTF8: {
            result = _hash(nm.dataUtf8(gs)->utf8);
            break;
        }
        case NameKind::CONSTANT: {
            result = hashNameRefWithoutUniques(gs, nm.dataCnst(gs)->original);
            break;
        }
        case NameKind::UNIQUE: {
            result = hashNameRefWithoutUniques(gs, nm.dataUnique(gs)->original);
            break;
        }
    }

    if (kind != NameKind::UNIQUE) {
        auto hashedKind = static_cast<underlying_type<NameKind>::type>(kind);
        result = mix(result, hashedKind);
    }
    return result;
}

uint32_t hashFullNameRef(const GlobalState &gs, NameRef nm) {
    uint32_t result;
    auto kind = nm.kind();

    switch (kind) {
        case NameKind::UTF8: {
            result = _hash(nm.dataUtf8(gs)->utf8);
            break;
        }
        case NameKind::CONSTANT: {
            result = hashFullNameRef(gs, nm.dataCnst(gs)->original);
            break;
        }
        case NameKind::UNIQUE: {
            auto data = nm.dataUnique(gs);
            auto hashedUniqueNameKind = static_cast<underlying_type<UniqueNameKind>::type>(data->uniqueNameKind);
            result = mix(mix(hashFullNameRef(gs, data->original), data->num), hashedUniqueNameKind);
            break;
        }
    }

    auto hashedKind = static_cast<underlying_type<NameKind>::type>(kind);
    return mix(result, hashedKind);
}

} // namespace

WithoutUniqueNameHash::WithoutUniqueNameHash(const GlobalState &gs, NameRef nm)
    : _hashValue(incZero(hashNameRefWithoutUniques(gs, nm))){};

void WithoutUniqueNameHash::sortAndDedupe(std::vector<core::WithoutUniqueNameHash> &hashes) {
    fast_sort(hashes);
    hashes.resize(std::distance(hashes.begin(), std::unique(hashes.begin(), hashes.end())));
    hashes.shrink_to_fit();
}

FullNameHash::FullNameHash(const GlobalState &gs, NameRef nm) : _hashValue(incZero(hashFullNameRef(gs, nm))) {}

void FullNameHash::sortAndDedupe(std::vector<core::FullNameHash> &hashes) {
    fast_sort(hashes);
    hashes.resize(std::distance(hashes.begin(), std::unique(hashes.begin(), hashes.end())));
    hashes.shrink_to_fit();
}

FoundDefinitionRef FoundStaticFieldHash::owner() const {
    if (this->ownerIsSymbol) {
        return {FoundDefinitionRef::Kind::Symbol, this->ownerIdx};
    } else {
        return {FoundDefinitionRef::Kind::Class, this->ownerIdx};
    }
}

void FoundStaticFieldHash::sanityCheck() const {
    ENFORCE(nameHash.isDefined());
}

string FoundStaticFieldHash::toString() const {
    return fmt::format("FoundStaticFieldHash {{ ownerIdx = {}, ownerIsSymbol = {}, nameHash = {} }}", ownerIdx,
                       ownerIsSymbol, nameHash._hashValue);
}

FoundDefinitionRef FoundTypeMemberHash::owner() const {
    if (this->ownerIsSymbol) {
        return {FoundDefinitionRef::Kind::Symbol, this->ownerIdx};
    } else {
        return {FoundDefinitionRef::Kind::Class, this->ownerIdx};
    }
}

void FoundTypeMemberHash::sanityCheck() const {
    ENFORCE(nameHash.isDefined());
}

string FoundTypeMemberHash::toString() const {
    return fmt::format(
        "FoundTypeMemberHash {{ ownerIdx = {}, ownerIsSymbol = {}, isTypeTemplate = {}, nameHash = {} }}", ownerIdx,
        ownerIsSymbol, isTypeTemplate, nameHash._hashValue);
}

FoundDefinitionRef FoundMethodHash::owner() const {
    if (this->ownerIsSymbol) {
        return {FoundDefinitionRef::Kind::Symbol, this->ownerIdx};
    } else {
        return {FoundDefinitionRef::Kind::Class, this->ownerIdx};
    }
}

void FoundMethodHash::sanityCheck() const {
    ENFORCE(nameHash.isDefined());
}

string FoundMethodHash::toString() const {
    return fmt::format("FoundMethodHash {{ ownerIdx = {}, ownerIsSymbol = {}, useSingletonClass = {}, nameHash = {}, "
                       "arityHash = {} }}",
                       ownerIdx, ownerIsSymbol, useSingletonClass, nameHash._hashValue, arityHash._hashValue);
}

FoundDefinitionRef FoundFieldHash::owner() const {
    if (this->ownerIsSymbol) {
        return {FoundDefinitionRef::Kind::Symbol, this->ownerIdx};
    } else {
        return {FoundDefinitionRef::Kind::Class, this->ownerIdx};
    }
}

void FoundFieldHash::sanityCheck() const {
    ENFORCE(nameHash.isDefined());
}

string FoundFieldHash::toString() const {
    return fmt::format(
        "FoundFieldHash {{ ownerIdx = {}, ownerIsSymbol = {}, onSingletonClass = {}, isInstanceVariable = {}, "
        "fromWithinMethod = {}, nameHash = {} }}",
        ownerIdx, ownerIsSymbol, onSingletonClass, isInstanceVariable, fromWithinMethod, nameHash._hashValue);
}

FileHash::FileHash(LocalSymbolTableHashes &&localSymbolTableHashes, UsageHash &&usages, FoundDefHashes &&foundHashes)
    : localSymbolTableHashes(move(localSymbolTableHashes)), usages(move(usages)), foundHashes(move(foundHashes)) {}

} // namespace sorbet::core
