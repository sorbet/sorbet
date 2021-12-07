#include "core/Names.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/NameSubstitution.h"
#include "core/Names.h"
#include "core/hashing/hashing.h"
#include <numeric> // accumulate

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"

using namespace std;

namespace sorbet::core {

NameRef::NameRef(const GlobalState &gs, NameKind kind, uint32_t id)
    : DebugOnlyCheck(gs, kind, id), _id{(id << KIND_BITS) | static_cast<uint32_t>(kind)} {
    // If this fails, the symbol table is too big :(
    ENFORCE_NO_TIMER(id <= MAX_ID);
}

NameRef::NameRef(const GlobalState &gs, NameRef ref)
    : DebugOnlyCheck(gs, ref.kind(), ref.unsafeTableIndex()), _id(ref.rawId()) {
    // If this fails, the symbol table is too big :(
    ENFORCE_NO_TIMER(this->unsafeTableIndex() <= MAX_ID);
}

string NameRef::showRaw(const GlobalState &gs) const {
    switch (kind()) {
        case NameKind::UTF8: {
            auto raw = dataUtf8(gs);
            return fmt::format("<U {}>", string(raw->utf8.begin(), raw->utf8.end()));
        }
        case NameKind::UNIQUE: {
            auto unique = dataUnique(gs);
            string kind;
            switch (unique->uniqueNameKind) {
                case UniqueNameKind::Parser:
                    kind = "P";
                    break;
                case UniqueNameKind::Desugar:
                    kind = "D";
                    break;
                case UniqueNameKind::Namer:
                    kind = "N";
                    break;
                case UniqueNameKind::MangleRename:
                    kind = "M";
                    break;
                case UniqueNameKind::Singleton:
                    kind = "S";
                    break;
                case UniqueNameKind::Overload:
                    kind = "O";
                    break;
                case UniqueNameKind::TypeVarName:
                    kind = "T";
                    break;
                case UniqueNameKind::PositionalArg:
                    kind = "A";
                    break;
                case UniqueNameKind::MangledKeywordArg:
                    kind = "K";
                    break;
                case UniqueNameKind::ResolverMissingClass:
                    kind = "R";
                    break;
                case UniqueNameKind::TEnum:
                    kind = "E";
                    break;
                case UniqueNameKind::Packager:
                    kind = "G";
                    break;
                case UniqueNameKind::PackagerPrivate:
                    kind = "V";
                    break;
            }
            if (gs.censorForSnapshotTests && unique->uniqueNameKind == UniqueNameKind::Namer &&
                unique->original == core::Names::staticInit()) {
                return fmt::format("<{} {} ${}>", kind, unique->original.showRaw(gs), "CENSORED");
            } else {
                return fmt::format("<{} {} ${}>", kind, unique->original.showRaw(gs), unique->num);
            }
        }
        case NameKind::CONSTANT:
            return fmt::format("<C {}>", dataCnst(gs)->original.showRaw(gs));
    }
}

string NameRef::toString(const GlobalState &gs) const {
    switch (kind()) {
        case NameKind::UTF8: {
            auto raw = dataUtf8(gs);
            return string(raw->utf8.begin(), raw->utf8.end());
        }
        case NameKind::UNIQUE: {
            auto unique = dataUnique(gs);
            if (unique->uniqueNameKind == UniqueNameKind::Singleton) {
                return fmt::format("<Class:{}>", unique->original.show(gs));
            } else if (unique->uniqueNameKind == UniqueNameKind::Overload) {
                return absl::StrCat(unique->original.show(gs), " (overload.", unique->num, ")");
            }
            if (gs.censorForSnapshotTests && unique->uniqueNameKind == UniqueNameKind::Namer &&
                unique->original == core::Names::staticInit()) {
                return fmt::format("{}${}", unique->original.show(gs), "CENSORED");
            } else {
                return fmt::format("{}${}", unique->original.show(gs), unique->num);
            }
        }
        case NameKind::CONSTANT:
            return fmt::format("<C {}>", dataCnst(gs)->original.toString(gs));
    }
}

string NameRef::show(const GlobalState &gs) const {
    switch (kind()) {
        case NameKind::UTF8: {
            auto raw = dataUtf8(gs);
            return string(raw->utf8.begin(), raw->utf8.end());
        }
        case NameKind::UNIQUE: {
            auto unique = dataUnique(gs);
            if (unique->uniqueNameKind == UniqueNameKind::Singleton) {
                return fmt::format("<Class:{}>", unique->original.show(gs));
            } else if (unique->uniqueNameKind == UniqueNameKind::Overload) {
                return absl::StrCat(unique->original.show(gs), " (overload.", unique->num, ")");
            } else if (unique->uniqueNameKind == UniqueNameKind::MangleRename) {
                return unique->original.show(gs);
            } else if (unique->uniqueNameKind == UniqueNameKind::TEnum) {
                // The entire goal of UniqueNameKind::TEnum is to have Name::show print the name as if on the
                // original name, so that our T::Enum DSL-synthesized class names are kept as an implementation detail.
                // Thus, we fall through.
            }
            return unique->original.show(gs);
        }
        case NameKind::CONSTANT:
            return dataCnst(gs)->original.show(gs);
    }
}
string_view NameRef::shortName(const GlobalState &gs) const {
    switch (kind()) {
        case NameKind::UTF8: {
            auto raw = dataUtf8(gs);
            return string_view(raw->utf8.begin(), raw->utf8.end() - raw->utf8.begin());
        }
        case NameKind::UNIQUE:
            return dataUnique(gs)->original.shortName(gs);
        case NameKind::CONSTANT:
            return dataCnst(gs)->original.shortName(gs);
    }
}

void NameRef::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode) {
        return;
    }
    switch (kind()) {
        case NameKind::UTF8:
            ENFORCE_NO_TIMER(*this == const_cast<GlobalState &>(gs).enterNameUTF8(dataUtf8(gs)->utf8),
                             "Name table corrupted, re-entering UTF8 name gives different id");
            break;
        case NameKind::UNIQUE: {
            auto unique = dataUnique(gs);
            ENFORCE_NO_TIMER(unique->num > 0, "unique num == 0");
            NameRef current2 =
                const_cast<GlobalState &>(gs).freshNameUnique(unique->uniqueNameKind, unique->original, unique->num);
            ENFORCE_NO_TIMER(*this == current2, "Name table corrupted, re-entering UNIQUE name gives different id");
            break;
        }
        case NameKind::CONSTANT:
            ENFORCE_NO_TIMER(*this == const_cast<GlobalState &>(gs).enterNameConstant(dataCnst(gs)->original),
                             "Name table corrupted, re-entering CONSTANT name gives different id");
            break;
    }
}

bool NameRef::isClassName(const GlobalState &gs) const {
    switch (kind()) {
        case NameKind::UTF8:
            return false;
        case NameKind::UNIQUE:
            switch (dataUnique(gs)->uniqueNameKind) {
                case UniqueNameKind::Singleton:
                case UniqueNameKind::MangleRename:
                case UniqueNameKind::TEnum:
                    return dataUnique(gs)->original.isClassName(gs);
                case UniqueNameKind::ResolverMissingClass:
                case UniqueNameKind::Packager:
                case UniqueNameKind::PackagerPrivate:
                case UniqueNameKind::Parser:
                case UniqueNameKind::Desugar:
                case UniqueNameKind::Namer:
                case UniqueNameKind::Overload:
                case UniqueNameKind::TypeVarName:
                case UniqueNameKind::PositionalArg:
                case UniqueNameKind::MangledKeywordArg:
                    return false;
            }
        case NameKind::CONSTANT:
            ENFORCE(dataCnst(gs)->original.isValidConstantName(gs));
            return true;
    }
}

bool NameRef::isTEnumName(const GlobalState &gs) const {
    if (kind() != NameKind::CONSTANT) {
        return false;
    }
    auto original = dataCnst(gs)->original;
    return original.kind() == NameKind::UNIQUE && original.dataUnique(gs)->uniqueNameKind == UniqueNameKind::TEnum;
}

bool NameRef::isPackagerName(const GlobalState &gs) const {
    if (kind() != NameKind::CONSTANT) {
        return false;
    }
    auto original = dataCnst(gs)->original;
    return original.kind() == NameKind::UNIQUE &&
           (original.dataUnique(gs)->uniqueNameKind == UniqueNameKind::Packager ||
            original.dataUnique(gs)->uniqueNameKind == UniqueNameKind::PackagerPrivate);
}

bool NameRef::isPackagerPrivateName(const GlobalState &gs) const {
    if (kind() != NameKind::CONSTANT) {
        return false;
    }
    auto original = dataCnst(gs)->original;
    return original.kind() == NameKind::UNIQUE &&
           original.dataUnique(gs)->uniqueNameKind == UniqueNameKind::PackagerPrivate;
}

bool NameRef::isValidConstantName(const GlobalState &gs) const {
    switch (kind()) {
        case NameKind::UTF8:
            return true;
        case NameKind::UNIQUE:
            switch (dataUnique(gs)->uniqueNameKind) {
                case UniqueNameKind::ResolverMissingClass:
                case UniqueNameKind::TEnum:
                case UniqueNameKind::Packager:
                case UniqueNameKind::PackagerPrivate:
                    return true;
                case UniqueNameKind::Parser:
                case UniqueNameKind::Desugar:
                case UniqueNameKind::Namer:
                case UniqueNameKind::MangleRename:
                case UniqueNameKind::Singleton:
                case UniqueNameKind::Overload:
                case UniqueNameKind::TypeVarName:
                case UniqueNameKind::PositionalArg:
                case UniqueNameKind::MangledKeywordArg:
                    return false;
            }
        case NameKind::CONSTANT:
            return false;
    }
}

NameRefDebugCheck::NameRefDebugCheck(const GlobalState &gs, NameKind kind, uint32_t index) {
    // store the globalStateId of the creating global state to allow sharing refs between siblings
    // when the ref refers to a name in the common ancestor
    globalStateId = gs.globalStateId;
    for (const auto &deepCloneInfo : gs.deepCloneHistory) {
        switch (kind) {
            case NameKind::UTF8:
                if (index < deepCloneInfo.lastUTF8NameKnownByParentGlobalState) {
                    globalStateId = deepCloneInfo.globalStateId;
                    return;
                }
                break;
            case NameKind::CONSTANT:
                if (index < deepCloneInfo.lastConstantNameKnownByParentGlobalState) {
                    globalStateId = deepCloneInfo.globalStateId;
                    return;
                }
                break;
            case NameKind::UNIQUE:
                if (index < deepCloneInfo.lastUniqueNameKnownByParentGlobalState) {
                    globalStateId = deepCloneInfo.globalStateId;
                    return;
                }
                break;
        }
    }
}

void NameRefDebugCheck::check(const GlobalState &gs, NameKind kind, uint32_t index) const {
    if (globalStateId == -1) {
        return;
    }
    if (kind == NameKind::UTF8 && index <= Names::LAST_WELL_KNOWN_UTF8_NAME) {
        return;
    }
    if (kind == NameKind::CONSTANT && index <= Names::LAST_WELL_KNOWN_CONSTANT_NAME) {
        return;
    }
    if (globalStateId == gs.globalStateId) {
        return;
    }
    for (const auto &deepCloneInfo : gs.deepCloneHistory) {
        if (globalStateId == deepCloneInfo.globalStateId) {
            switch (kind) {
                case NameKind::UTF8:
                    if (index < deepCloneInfo.lastUTF8NameKnownByParentGlobalState) {
                        return;
                    }
                    break;
                case NameKind::CONSTANT:
                    if (index < deepCloneInfo.lastConstantNameKnownByParentGlobalState) {
                        return;
                    }
                    break;
                case NameKind::UNIQUE:
                    if (index < deepCloneInfo.lastUniqueNameKnownByParentGlobalState) {
                        return;
                    }
                    break;
            }
        }
    }
    ENFORCE(false, "NameRef not owned by correct GlobalState");
}

void NameRefDebugCheck::check(const NameSubstitution &subst) const {
    ENFORCE(globalStateId != subst.toGlobalStateId, "substituting a name twice!");
}

void NameRef::enforceCorrectGlobalState(const GlobalState &gs) const {
    runDebugOnlyCheck(gs, kind(), unsafeTableIndex());
}

void NameRef::sanityCheckSubstitution(const NameSubstitution &subst) const {
    runDebugOnlyCheck(subst);
}

const UniqueNameData NameRef::dataUnique(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(kind() == NameKind::UNIQUE);
    ENFORCE_NO_TIMER(uniqueIndex() < gs.uniqueNames.size(), "unique name {} id out of bounds {}", uniqueIndex(),
                     gs.uniqueNames.size());
    return UniqueNameData(gs.uniqueNames[uniqueIndex()], gs);
}

const UTF8NameData NameRef::dataUtf8(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(kind() == NameKind::UTF8);
    ENFORCE_NO_TIMER(utf8Index() < gs.utf8Names.size(), "utf8 name {} id out of bounds {}", utf8Index(),
                     gs.utf8Names.size());
    return UTF8NameData(gs.utf8Names[utf8Index()], gs);
}

const ConstantNameData NameRef::dataCnst(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(kind() == NameKind::CONSTANT);
    ENFORCE_NO_TIMER(constantIndex() < gs.constantNames.size(), "constant name {} id out of bounds {}", constantIndex(),
                     gs.constantNames.size());
    return ConstantNameData(gs.constantNames[constantIndex()], gs);
}

NameRef NameRef::addEq(GlobalState &gs) const {
    auto name = this->dataUtf8(gs);
    string nameEq = absl::StrCat(name->utf8, "=");
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::lookupWithEq(const GlobalState &gs) const {
    auto name = this->dataUtf8(gs);
    string nameEq = absl::StrCat(name->utf8, "=");
    return gs.lookupNameUTF8(nameEq);
}

NameRef NameRef::addQuestion(GlobalState &gs) const {
    auto name = this->dataUtf8(gs);
    string nameEq = absl::StrCat(name->utf8, "?");
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::lookupWithAt(const GlobalState &gs) const {
    auto name = this->dataUtf8(gs);
    string nameEq = absl::StrCat("@", name->utf8);
    return gs.lookupNameUTF8(nameEq);
}

NameRef NameRef::addAt(GlobalState &gs) const {
    auto name = this->dataUtf8(gs);
    string nameEq = absl::StrCat("@", name->utf8);
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::prepend(GlobalState &gs, string_view s) const {
    auto name = this->dataUtf8(gs);
    string nameEq = absl::StrCat(s, name->utf8);
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::lookupMangledPrivatePackageName(const GlobalState &gs) const {
    auto name = this->dataUtf8(gs);
    auto parts = absl::StrSplit(name->utf8, "::");
    string mangledName = absl::StrCat(absl::StrJoin(parts, "_"), core::PACKAGE_PRIVATE_SUFFIX);

    auto utf8Name = gs.lookupNameUTF8(mangledName);
    if (!utf8Name.exists()) {
        return utf8Name;
    }

    auto packagerName = gs.lookupNameUnique(core::UniqueNameKind::PackagerPrivate, utf8Name, 1);
    if (!packagerName.exists()) {
        return packagerName;
    }

    return gs.lookupNameConstant(packagerName);
}

UTF8Name UTF8Name::deepCopy(const GlobalState &to) const {
    return UTF8Name{this->utf8};
}

ConstantName ConstantName::deepCopy(const GlobalState &to) const {
    return ConstantName{NameRef(to, this->original)};
}

UniqueName UniqueName::deepCopy(const GlobalState &to) const {
    return UniqueName{NameRef(to, this->original), this->num, this->uniqueNameKind};
}

UniqueNameData::UniqueNameData(const UniqueName &ref, const GlobalState &gs) : DebugOnlyCheck(gs), name(ref) {}
ConstantNameData::ConstantNameData(const ConstantName &ref, const GlobalState &gs) : DebugOnlyCheck(gs), name(ref) {}
UTF8NameData::UTF8NameData(const UTF8Name &ref, const GlobalState &gs) : DebugOnlyCheck(gs), name(ref) {}

NameDataDebugCheck::NameDataDebugCheck(const GlobalState &gs) : gs(gs), nameCountAtCreation(gs.namesUsedTotal()) {}

void NameDataDebugCheck::check() const {
    ENFORCE(nameCountAtCreation == gs.namesUsedTotal());
}

const UniqueName *UniqueNameData::operator->() const {
    runDebugOnlyCheck();
    return &name;
};

const ConstantName *ConstantNameData::operator->() const {
    runDebugOnlyCheck();
    return &name;
};

const UTF8Name *UTF8NameData::operator->() const {
    runDebugOnlyCheck();
    return &name;
};

} // namespace sorbet::core
