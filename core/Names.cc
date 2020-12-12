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

NameRef::NameRef(const GlobalState &gs, NameKind kind, u4 id)
    : DebugOnlyCheck(gs, id), _id{(id & ID_MASK) | (static_cast<u4>(kind) << ID_BITS)} {
    // If this fails, the symbol table is too big :(
    ENFORCE_NO_TIMER(id <= ID_MASK);
}

NameRef::NameRef(const GlobalState &gs, NameRef ref) : DebugOnlyCheck(gs, ref.unsafeTableIndex()), _id(ref.rawId()) {
    // If this fails, the symbol table is too big :(
    ENFORCE_NO_TIMER(this->unsafeTableIndex() <= ID_MASK);
}

Name::~Name() noexcept {
    if (kind == NameKind::UNIQUE) {
        unique.~UniqueName();
    }
}

unsigned int NameRef::hash(const GlobalState &gs) const {
    // TODO: use https://github.com/Cyan4973/xxHash
    // !!! keep this in sync with GlobalState.enter*
    auto &name = data(gs);
    switch (name.kind) {
        case NameKind::UTF8:
            return _hash(name.raw.utf8);
        case NameKind::UNIQUE:
            return _hash_mix_unique((u2)name.unique.uniqueNameKind, NameKind::UNIQUE, name.unique.num,
                                    name.unique.original.rawId());
        case NameKind::CONSTANT:
            return _hash_mix_constant(NameKind::CONSTANT, name.cnst.original.rawId());
    }
}

string NameRef::showRaw(const GlobalState &gs) const {
    auto &name = data(gs);
    switch (name.kind) {
        case NameKind::UTF8:
            return fmt::format("<U {}>", string(name.raw.utf8.begin(), name.raw.utf8.end()));
        case NameKind::UNIQUE: {
            string kind;
            switch (name.unique.uniqueNameKind) {
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
            if (gs.censorForSnapshotTests && name.unique.uniqueNameKind == UniqueNameKind::Namer &&
                name.unique.original == core::Names::staticInit()) {
                return fmt::format("<{} {} ${}>", kind, name.unique.original.showRaw(gs), "CENSORED");
            } else {
                return fmt::format("<{} {} ${}>", kind, name.unique.original.showRaw(gs), name.unique.num);
            }
        }
        case NameKind::CONSTANT:
            return fmt::format("<C {}>", name.cnst.original.showRaw(gs));
    }
}

string NameRef::toString(const GlobalState &gs) const {
    auto &name = data(gs);
    switch (name.kind) {
        case NameKind::UTF8:
            return string(name.raw.utf8.begin(), name.raw.utf8.end());
        case NameKind::UNIQUE:
            if (name.unique.uniqueNameKind == UniqueNameKind::Singleton) {
                return fmt::format("<Class:{}>", name.unique.original.show(gs));
            } else if (name.unique.uniqueNameKind == UniqueNameKind::Overload) {
                return absl::StrCat(name.unique.original.show(gs), " (overload.", name.unique.num, ")");
            }
            if (gs.censorForSnapshotTests && name.unique.uniqueNameKind == UniqueNameKind::Namer &&
                name.unique.original == core::Names::staticInit()) {
                return fmt::format("{}${}", name.unique.original.show(gs), "CENSORED");
            } else {
                return fmt::format("{}${}", name.unique.original.show(gs), name.unique.num);
            }
        case NameKind::CONSTANT:
            return fmt::format("<C {}>", name.cnst.original.toString(gs));
    }
}

string NameRef::show(const GlobalState &gs) const {
    auto &name = data(gs);
    switch (name.kind) {
        case NameKind::UTF8:
            return string(name.raw.utf8.begin(), name.raw.utf8.end());
        case NameKind::UNIQUE:
            if (name.unique.uniqueNameKind == UniqueNameKind::Singleton) {
                return fmt::format("<Class:{}>", name.unique.original.show(gs));
            } else if (name.unique.uniqueNameKind == UniqueNameKind::Overload) {
                return absl::StrCat(name.unique.original.show(gs), " (overload.", name.unique.num, ")");
            } else if (name.unique.uniqueNameKind == UniqueNameKind::MangleRename) {
                return name.unique.original.show(gs);
            } else if (name.unique.uniqueNameKind == UniqueNameKind::TEnum) {
                // The entire goal of UniqueNameKind::TEnum is to have Name::show print the name as if on the
                // original name, so that our T::Enum DSL-synthesized class names are kept as an implementation detail.
                // Thus, we fall through.
            }
            return name.unique.original.show(gs);
        case NameKind::CONSTANT:
            return name.cnst.original.show(gs);
    }
}
string_view NameRef::shortName(const GlobalState &gs) const {
    auto &name = data(gs);
    switch (name.kind) {
        case NameKind::UTF8:
            return string_view(name.raw.utf8.begin(), name.raw.utf8.end() - name.raw.utf8.begin());
        case NameKind::UNIQUE:
            return name.unique.original.shortName(gs);
        case NameKind::CONSTANT:
            return name.cnst.original.shortName(gs);
    }
}

void NameRef::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode) {
        return;
    }
    auto &name = data(gs);
    switch (name.kind) {
        case NameKind::UTF8:
            ENFORCE_NO_TIMER(*this == const_cast<GlobalState &>(gs).enterNameUTF8(name.raw.utf8),
                             "Name table corrupted, re-entering UTF8 name gives different id");
            break;
        case NameKind::UNIQUE: {
            ENFORCE_NO_TIMER(name.unique.original.unsafeTableIndex() < this->unsafeTableIndex(),
                             "unique name id not bigger than original");
            ENFORCE_NO_TIMER(name.unique.num > 0, "unique num == 0");
            NameRef current2 = const_cast<GlobalState &>(gs).freshNameUnique(name.unique.uniqueNameKind,
                                                                             name.unique.original, name.unique.num);
            ENFORCE_NO_TIMER(*this == current2, "Name table corrupted, re-entering UNIQUE name gives different id");
            break;
        }
        case NameKind::CONSTANT:
            ENFORCE_NO_TIMER(name.cnst.original.unsafeTableIndex() < this->unsafeTableIndex(),
                             "constant name id not bigger than original");
            ENFORCE_NO_TIMER(*this == const_cast<GlobalState &>(gs).enterNameConstant(name.cnst.original),
                             "Name table corrupted, re-entering CONSTANT name gives different id");
            break;
    }
}

bool NameRef::isClassName(const GlobalState &gs) const {
    auto &name = data(gs);
    switch (name.kind) {
        case NameKind::UTF8:
            return false;
        case NameKind::UNIQUE: {
            return (name.unique.uniqueNameKind == UniqueNameKind::Singleton ||
                    name.unique.uniqueNameKind == UniqueNameKind::MangleRename ||
                    name.unique.uniqueNameKind == UniqueNameKind::TEnum) &&
                   name.unique.original.isClassName(gs);
        }
        case NameKind::CONSTANT:
            ENFORCE(name.cnst.original.kind() == NameKind::UTF8 ||
                    name.cnst.original.dataUnique(gs)->uniqueNameKind == UniqueNameKind::ResolverMissingClass ||
                    name.cnst.original.dataUnique(gs)->uniqueNameKind == UniqueNameKind::TEnum);
            return true;
    }
}

bool NameRef::isTEnumName(const GlobalState &gs) const {
    auto &name = data(gs);
    if (name.kind != NameKind::CONSTANT) {
        return false;
    }
    auto original = name.cnst.original;
    return original.data(gs).kind == NameKind::UNIQUE &&
           original.data(gs).unique.uniqueNameKind == UniqueNameKind::TEnum;
}

NameRefDebugCheck::NameRefDebugCheck(const GlobalState &gs, u4 _id) {
    // store the globalStateId of the creating global state to allow sharing refs between siblings
    // when the ref refers to a name in the common ancestor
    globalStateId = gs.globalStateId;
    for (const auto &deepCloneInfo : gs.deepCloneHistory) {
        if (_id < deepCloneInfo.lastNameKnownByParentGlobalState) {
            globalStateId = deepCloneInfo.globalStateId;
            break;
        }
    }
}

void NameRefDebugCheck::check(const GlobalState &gs, u4 _id) const {
    if (globalStateId == -1) {
        return;
    }
    if (_id <= Names::LAST_WELL_KNOWN_NAME) {
        return;
    }
    if (globalStateId == gs.globalStateId) {
        return;
    }
    for (const auto &deepCloneInfo : gs.deepCloneHistory) {
        if (globalStateId == deepCloneInfo.globalStateId && _id < deepCloneInfo.lastNameKnownByParentGlobalState) {
            return;
        }
    }
    ENFORCE(false, "NameRef not owned by correct GlobalState");
}

void NameRefDebugCheck::check(const GlobalSubstitution &subst) const {
    ENFORCE(globalStateId != subst.toGlobalStateId, "substituting a name twice!");
}

void NameRef::enforceCorrectGlobalState(const GlobalState &gs) const {
    runDebugOnlyCheck(gs, unsafeTableIndex());
}

void NameRef::sanityCheckSubstitution(const GlobalSubstitution &subst) const {
    runDebugOnlyCheck(subst);
}

const Name &NameRef::data(const GlobalState &gs) const {
    ENFORCE(unsafeTableIndex() < gs.names.size(), "name {} id out of bounds {}", _id, gs.names.size());
    ENFORCE(exists(), "non existing name");
    enforceCorrectGlobalState(gs);
    return gs.names[unsafeTableIndex()];
}

const UniqueNameData NameRef::dataUnique(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(kind() == NameKind::UNIQUE);
    auto &name = data(gs);
    ENFORCE_NO_TIMER(name.kind == NameKind::UNIQUE);
    return UniqueNameData(name.unique, gs);
}

const UTF8NameData NameRef::dataUtf8(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(kind() == NameKind::UTF8);
    auto &name = data(gs);
    ENFORCE(name.kind == NameKind::UTF8);
    return UTF8NameData(name.raw, gs);
}

const ConstantNameData NameRef::dataCnst(const GlobalState &gs) const {
    ENFORCE_NO_TIMER(kind() == NameKind::CONSTANT);
    auto &name = data(gs);
    ENFORCE_NO_TIMER(name.kind == NameKind::CONSTANT);
    return ConstantNameData(name.cnst, gs);
}

NameRef NameRef::addEq(GlobalState &gs) const {
    auto name = this->dataUtf8(gs);
    string nameEq = absl::StrCat(name->utf8, "=");
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::addQuestion(GlobalState &gs) const {
    auto name = this->dataUtf8(gs);
    string nameEq = absl::StrCat(name->utf8, "?");
    return gs.enterNameUTF8(nameEq);
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

NameRef NameRef::lookupMangledPackageName(const GlobalState &gs) const {
    auto name = this->dataUtf8(gs);
    auto parts = absl::StrSplit(name->utf8, "::");
    string nameEq = absl::StrCat(absl::StrJoin(parts, "_"), "_Package");
    return gs.lookupNameConstant(nameEq);
}

Name Name::deepCopy(const GlobalState &to) const {
    Name out;
    out.kind = this->kind;

    switch (this->kind) {
        case NameKind::UTF8:
            out.raw = this->raw;
            break;

        case NameKind::UNIQUE:
            out.unique.uniqueNameKind = this->unique.uniqueNameKind;
            out.unique.num = this->unique.num;
            out.unique.original = NameRef(to, this->unique.original);
            break;

        case NameKind::CONSTANT:
            out.cnst.original = NameRef(to, this->cnst.original);
            break;
    }

    return out;
}

UniqueNameData::UniqueNameData(const UniqueName &ref, const GlobalState &gs) : DebugOnlyCheck(gs), name(ref) {}
ConstantNameData::ConstantNameData(const ConstantName &ref, const GlobalState &gs) : DebugOnlyCheck(gs), name(ref) {}
UTF8NameData::UTF8NameData(const UTF8Name &ref, const GlobalState &gs) : DebugOnlyCheck(gs), name(ref) {}

NameDataDebugCheck::NameDataDebugCheck(const GlobalState &gs) : gs(gs), nameCountAtCreation(gs.namesUsed()) {}

void NameDataDebugCheck::check() const {
    ENFORCE(nameCountAtCreation == gs.namesUsed());
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
