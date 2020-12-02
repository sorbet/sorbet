#include "core/Names.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/GlobalSubstitution.h"
#include "core/Hashing.h"
#include "core/Names.h"
#include <numeric> // accumulate

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"

using namespace std;

namespace sorbet::core {

NameRef::NameRef(const GlobalState &gs, Kind kind, u4 id)
    : DebugOnlyCheck(gs, static_cast<u2>(kind), id), _id(id | (static_cast<u4>(kind) << ID_BITS)) {
    // If this fails, the name table is too big :(
    ENFORCE_NO_TIMER(id <= ID_MASK);
}

const UTF8NameData NameRef::utf8NameData(const core::GlobalState &gs) const {
    ENFORCE_NO_TIMER(utf8Index() < gs.utf8Names.size());
    return UTF8NameData(gs.utf8Names[utf8Index()], gs);
}
const ConstantNameData NameRef::constantNameData(const core::GlobalState &gs) const {
    ENFORCE_NO_TIMER(constantIndex() < gs.constantNames.size());
    return ConstantNameData(gs.constantNames[constantIndex()], gs);
}

const UniqueNameData NameRef::uniqueNameData(const core::GlobalState &gs) const {
    ENFORCE_NO_TIMER(uniqueIndex() < gs.uniqueNames.size());
    return UniqueNameData(gs.uniqueNames[uniqueIndex()], gs);
}

unsigned int UTF8Name::hash(const GlobalState &gs) const {
    return _hash(utf8);
}

unsigned int UniqueName::hash(const GlobalState &gs) const {
    // TODO: use https://github.com/Cyan4973/xxHash
    // !!! keep this in sync with GlobalState.enter*
    return _hash_mix_unique(static_cast<u2>(uniqueNameKind), NameRef::Kind::UNIQUE, num, original.rawId());
}

unsigned int ConstantName::hash(const GlobalState &gs) const {
    return _hash_mix_constant(NameRef::Kind::CONSTANT, original.rawId());
}

string NameRef::showRaw(const GlobalState &gs) const {
    switch (this->kind()) {
        case Kind::UTF8: {
            auto raw = utf8NameData(gs);
            return fmt::format("<U {}>", string(raw->utf8.begin(), raw->utf8.end()));
        }
        case Kind::UNIQUE: {
            auto unique = uniqueNameData(gs);
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
            }
            if (gs.censorForSnapshotTests && unique->uniqueNameKind == UniqueNameKind::Namer &&
                unique->original == core::Names::staticInit()) {
                return fmt::format("<{} {} ${}>", kind, unique->original.showRaw(gs), "CENSORED");
            } else {
                return fmt::format("<{} {} ${}>", kind, unique->original.showRaw(gs), unique->num);
            }
        }
        case Kind::CONSTANT: {
            auto cnst = constantNameData(gs);
            return fmt::format("<C {}>", cnst->original.showRaw(gs));
        }
    }
}

string NameRef::toString(const GlobalState &gs) const {
    switch (this->kind()) {
        case Kind::UTF8: {
            auto raw = utf8NameData(gs);
            return string(raw->utf8.begin(), raw->utf8.end());
        }
        case Kind::UNIQUE: {
            auto unique = uniqueNameData(gs);
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
        case Kind::CONSTANT: {
            auto cnst = constantNameData(gs);
            return fmt::format("<C {}>", cnst->original.toString(gs));
        }
    }
}

string NameRef::show(const GlobalState &gs) const {
    switch (this->kind()) {
        case Kind::UTF8: {
            auto raw = utf8NameData(gs);
            return string(raw->utf8.begin(), raw->utf8.end());
        }
        case Kind::UNIQUE: {
            auto unique = uniqueNameData(gs);
            if (unique->uniqueNameKind == UniqueNameKind::Singleton) {
                return fmt::format("<Class:{}>", unique->original.show(gs));
            } else if (unique->uniqueNameKind == UniqueNameKind::Overload) {
                return absl::StrCat(unique->original.show(gs), " (overload.", unique->num, ")");
            } else if (unique->uniqueNameKind == UniqueNameKind::MangleRename) {
                return unique->original.show(gs);
            } else if (unique->uniqueNameKind == UniqueNameKind::TEnum) {
                // The entire goal of UniqueNameKind::TEnum is to have Name::show print the name as if on the
                // original name, so that our T::Enum DSL-synthesized class names are kept as an implementation
                // detail. Thus, we fall through.
            }
            return unique->original.show(gs);
        }
        case Kind::CONSTANT: {
            auto cnst = constantNameData(gs);
            return cnst->original.show(gs);
        }
    }
}
string_view NameRef::shortName(const GlobalState &gs) const {
    switch (this->kind()) {
        case Kind::UTF8: {
            auto raw = utf8NameData(gs);
            return string_view(raw->utf8.begin(), raw->utf8.end() - raw->utf8.begin());
        }
        case Kind::UNIQUE: {
            auto unique = uniqueNameData(gs);
            return unique->original.shortName(gs);
        }
        case Kind::CONSTANT: {
            auto cnst = constantNameData(gs);
            return cnst->original.shortName(gs);
        }
    }
}

unsigned int NameRef::hash(const GlobalState &gs) const {
    switch (this->kind()) {
        case Kind::UTF8:
            return utf8NameData(gs)->hash(gs);
        case Kind::CONSTANT:
            return constantNameData(gs)->hash(gs);
        case Kind::UNIQUE:
            return uniqueNameData(gs)->hash(gs);
    }
}

void NameRef::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode) {
        return;
    }
    switch (this->kind()) {
        case Kind::UTF8: {
            auto raw = utf8NameData(gs);
            ENFORCE_NO_TIMER(*this == const_cast<GlobalState &>(gs).enterNameUTF8(raw->utf8),
                             "Name table corrupted, re-entering UTF8 name gives different id");
            break;
        }
        case Kind::UNIQUE: {
            auto unique = uniqueNameData(gs);
            ENFORCE_NO_TIMER(unique->num > 0, "unique num == 0");
            NameRef current2 =
                const_cast<GlobalState &>(gs).freshNameUnique(unique->uniqueNameKind, unique->original, unique->num);
            ENFORCE_NO_TIMER(*this == current2, "Name table corrupted, re-entering UNIQUE name gives different id");
            break;
        }
        case Kind::CONSTANT: {
            auto cnst = constantNameData(gs);
            ENFORCE_NO_TIMER(*this == const_cast<GlobalState &>(gs).enterNameConstant(cnst->original),
                             "Name table corrupted, re-entering CONSTANT name gives different id");
            break;
        }
    }
}

bool NameRef::isClassName(const GlobalState &gs) const {
    switch (this->kind()) {
        case Kind::UTF8:
            return false;
        case Kind::UNIQUE: {
            auto unique = uniqueNameData(gs);
            return (unique->uniqueNameKind == UniqueNameKind::Singleton ||
                    unique->uniqueNameKind == UniqueNameKind::MangleRename ||
                    unique->uniqueNameKind == UniqueNameKind::TEnum) &&
                   unique->original.isClassName(gs);
        }
        case Kind::CONSTANT: {
            auto cnst = constantNameData(gs);
            ENFORCE(cnst->original.kind() == Kind::UTF8 ||
                    (cnst->original.kind() == Kind::UNIQUE &&
                     (cnst->original.uniqueNameData(gs)->uniqueNameKind == UniqueNameKind::ResolverMissingClass ||
                      cnst->original.uniqueNameData(gs)->uniqueNameKind == UniqueNameKind::TEnum)));
            return true;
        }
    }
}

bool NameRef::isTEnumName(const GlobalState &gs) const {
    if (this->kind() != Kind::CONSTANT) {
        return false;
    }
    auto cnst = constantNameData(gs);
    auto original = cnst->original;
    return original.kind() == Kind::UNIQUE && original.uniqueNameData(gs)->uniqueNameKind == UniqueNameKind::TEnum;
}

NameRefDebugCheck::NameRefDebugCheck(const GlobalState &gs, u2 kind, u4 index) {
    // store the globalStateId of the creating global state to allow sharing refs between siblings
    // when the ref refers to a name in the common ancestor
    globalStateId = gs.globalStateId;
    for (const auto &deepCloneInfo : gs.deepCloneHistory) {
        switch (static_cast<NameRef::Kind>(kind)) {
            case NameRef::Kind::CONSTANT:
                if (index < deepCloneInfo.lastConstantNameKnownByParentGlobalState) {
                    globalStateId = deepCloneInfo.globalStateId;
                }
                break;
            case NameRef::Kind::UTF8:
                if (index < deepCloneInfo.lastUTF8NameKnownByParentGlobalState) {
                    globalStateId = deepCloneInfo.globalStateId;
                }
                break;
            case NameRef::Kind::UNIQUE:
                if (index < deepCloneInfo.lastUniqueNameKnownByParentGlobalState) {
                    globalStateId = deepCloneInfo.globalStateId;
                }
                break;
        }
    }
}

void NameRefDebugCheck::check(const GlobalState &gs, u2 kind, u4 index) const {
    if (globalStateId == -1) {
        return;
    }
    auto kindEnum = static_cast<NameRef::Kind>(kind);
    if (kindEnum == NameRef::Kind::CONSTANT && index <= Names::LAST_WELL_KNOWN_CONSTANT_NAME) {
        return;
    }
    if (kindEnum == NameRef::Kind::UTF8 && index <= Names::LAST_WELL_KNOWN_UTF8_NAME) {
        return;
    }
    if (globalStateId == gs.globalStateId) {
        return;
    }
    for (const auto &deepCloneInfo : gs.deepCloneHistory) {
        if (globalStateId == deepCloneInfo.globalStateId) {
            switch (kindEnum) {
                case NameRef::Kind::CONSTANT:
                    if (index < deepCloneInfo.lastConstantNameKnownByParentGlobalState) {
                        return;
                    }
                    break;
                case NameRef::Kind::UTF8:
                    if (index < deepCloneInfo.lastUTF8NameKnownByParentGlobalState) {
                        return;
                    }
                    break;
                case NameRef::Kind::UNIQUE:
                    if (index < deepCloneInfo.lastUniqueNameKnownByParentGlobalState) {
                        return;
                    }
                    break;
            }
        }
    }
    ENFORCE(false, "NameRef not owned by correct GlobalState");
}

void NameRefDebugCheck::check(const GlobalSubstitution &subst) const {
    ENFORCE(globalStateId != subst.toGlobalStateId, "substituting a name twice!");
}

void NameRef::enforceCorrectGlobalState(const GlobalState &gs) const {
    runDebugOnlyCheck(gs, static_cast<u2>(kind()), unsafeTableIndex());
}

void NameRef::sanityCheckSubstitution(const GlobalSubstitution &subst) const {
    runDebugOnlyCheck(subst);
}

NameRef NameRef::addEq(GlobalState &gs) const {
    ENFORCE(kind() == Kind::UTF8, "addEq over non-utf8 name");
    auto &raw = utf8NameData(gs);
    string nameEq = absl::StrCat(raw->utf8, "=");
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::addQuestion(GlobalState &gs) const {
    ENFORCE(kind() == Kind::UTF8, "addQuestion over non-utf8 name");
    auto &raw = utf8NameData(gs);
    string nameEq = absl::StrCat(raw->utf8, "?");
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::addAt(GlobalState &gs) const {
    ENFORCE(kind() == Kind::UTF8, "addAt over non-utf8 name");
    auto &raw = utf8NameData(gs);
    string nameEq = absl::StrCat("@", raw->utf8);
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::prepend(GlobalState &gs, string_view s) const {
    ENFORCE(kind() == Kind::UTF8, "prepend over non-utf8 name");
    auto &raw = utf8NameData(gs);
    string nameEq = absl::StrCat(s, raw->utf8);
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::lookupMangledPackageName(const GlobalState &gs) const {
    ENFORCE(kind() == Kind::UTF8, "manglePackageName over non-utf8 name");
    auto &raw = utf8NameData(gs);
    auto parts = absl::StrSplit(raw->utf8, "::");
    string nameEq = absl::StrCat(absl::StrJoin(parts, "_"), "_Package");
    return gs.lookupNameConstant(nameEq);
}

UTF8Name::UTF8Name(string_view utf8) : utf8(utf8) {}

ConstantName::ConstantName(NameRef original) : original(original) {}

UniqueName::UniqueName(NameRef original, u4 num, UniqueNameKind uniqueNameKind)
    : original(original), num(num), uniqueNameKind(uniqueNameKind) {}

UTF8Name UTF8Name::deepCopy(const GlobalState &to) const {
    return UTF8Name(this->utf8);
}

ConstantName ConstantName::deepCopy(const GlobalState &to) const {
    return ConstantName(NameRef(to, this->original.kind(), this->original.unsafeTableIndex()));
}

UniqueName UniqueName::deepCopy(const GlobalState &to) const {
    return UniqueName(NameRef(to, this->original.kind(), this->original.unsafeTableIndex()), this->num,
                      this->uniqueNameKind);
}

NameDataDebugCheck::NameDataDebugCheck(const GlobalState &gs) : gs(gs), nameCountAtCreation(gs.namesUsedTotal()) {}

void NameDataDebugCheck::check() const {
    ENFORCE(nameCountAtCreation == gs.namesUsedTotal());
}

UniqueNameData::UniqueNameData(const UniqueName &ref, const GlobalState &gs) : DebugOnlyCheck(gs), name(ref) {}

const UniqueName *UniqueNameData::operator->() const {
    runDebugOnlyCheck();
    return &name;
};

UTF8NameData::UTF8NameData(const UTF8Name &ref, const GlobalState &gs) : DebugOnlyCheck(gs), name(ref) {}

const UTF8Name *UTF8NameData::operator->() const {
    runDebugOnlyCheck();
    return &name;
};

ConstantNameData::ConstantNameData(const ConstantName &ref, const GlobalState &gs) : DebugOnlyCheck(gs), name(ref) {}

const ConstantName *ConstantNameData::operator->() const {
    runDebugOnlyCheck();
    return &name;
};

} // namespace sorbet::core
