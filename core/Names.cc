#include "core/Names.h"
#include "core/Context.h"
#include "core/GlobalState.h"
#include "core/GlobalSubstitution.h"
#include "core/Hashing.h"
#include "core/Names.h"
#include <numeric> // accumulate

#include "absl/strings/str_cat.h"

using namespace std;

namespace sorbet::core {

NameRef::NameRef(const GlobalState &gs, unsigned int id) : DebugOnlyCheck(gs, id), _id(id) {}

Name::~Name() noexcept {
    if (kind == NameKind::UNIQUE) {
        unique.~UniqueName();
    }
}

unsigned int Name::hash(const GlobalState &gs) const {
    // TODO: use https://github.com/Cyan4973/xxHash
    // !!! keep this in sync with GlobalState.enter*
    switch (kind) {
        case NameKind::UTF8:
            return _hash(raw.utf8);
        case NameKind::UNIQUE:
            return _hash_mix_unique((u2)unique.uniqueNameKind, NameKind::UNIQUE, unique.num, unique.original.id());
        case NameKind::CONSTANT:
            return _hash_mix_constant(NameKind::CONSTANT, cnst.original.id());
    }
}

string Name::showRaw(const GlobalState &gs) const {
    switch (this->kind) {
        case NameKind::UTF8:
            return fmt::format("<U {}>", string(raw.utf8.begin(), raw.utf8.end()));
        case NameKind::UNIQUE: {
            string kind;
            switch (this->unique.uniqueNameKind) {
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
            if (gs.censorForSnapshotTests && this->unique.uniqueNameKind == UniqueNameKind::Namer &&
                this->unique.original == core::Names::staticInit()) {
                return fmt::format("<{} {} ${}>", kind, this->unique.original.data(gs)->showRaw(gs), "CENSORED");
            } else {
                return fmt::format("<{} {} ${}>", kind, this->unique.original.data(gs)->showRaw(gs), this->unique.num);
            }
        }
        case NameKind::CONSTANT:
            return fmt::format("<C {}>", this->cnst.original.showRaw(gs));
    }
}

string Name::toString(const GlobalState &gs) const {
    switch (this->kind) {
        case NameKind::UTF8:
            return string(raw.utf8.begin(), raw.utf8.end());
        case NameKind::UNIQUE:
            if (this->unique.uniqueNameKind == UniqueNameKind::Singleton) {
                return fmt::format("<Class:{}>", this->unique.original.data(gs)->show(gs));
            } else if (this->unique.uniqueNameKind == UniqueNameKind::Overload) {
                return absl::StrCat(this->unique.original.data(gs)->show(gs), " (overload.", this->unique.num, ")");
            }
            if (gs.censorForSnapshotTests && this->unique.uniqueNameKind == UniqueNameKind::Namer &&
                this->unique.original == core::Names::staticInit()) {
                return fmt::format("{}${}", this->unique.original.data(gs)->show(gs), "CENSORED");
            } else {
                return fmt::format("{}${}", this->unique.original.data(gs)->show(gs), this->unique.num);
            }
        case NameKind::CONSTANT:
            return fmt::format("<C {}>", this->cnst.original.toString(gs));
    }
}

string Name::show(const GlobalState &gs) const {
    switch (this->kind) {
        case NameKind::UTF8:
            return string(raw.utf8.begin(), raw.utf8.end());
        case NameKind::UNIQUE:
            if (this->unique.uniqueNameKind == UniqueNameKind::Singleton) {
                return fmt::format("<Class:{}>", this->unique.original.data(gs)->show(gs));
            } else if (this->unique.uniqueNameKind == UniqueNameKind::Overload) {
                return absl::StrCat(this->unique.original.data(gs)->show(gs), " (overload.", this->unique.num, ")");
            } else if (this->unique.uniqueNameKind == UniqueNameKind::MangleRename) {
                return this->unique.original.data(gs)->show(gs);
            } else if (this->unique.uniqueNameKind == UniqueNameKind::TEnum) {
                // The entire goal of UniqueNameKind::TEnum is to have Name::show print the name as if on the
                // original name, so that our T::Enum DSL-synthesized class names are kept as an implementation detail.
                // Thus, we fall through.
            }
            return this->unique.original.data(gs)->show(gs);
        case NameKind::CONSTANT:
            return this->cnst.original.show(gs);
    }
}
string_view Name::shortName(const GlobalState &gs) const {
    switch (this->kind) {
        case NameKind::UTF8:
            return string_view(raw.utf8.begin(), raw.utf8.end() - raw.utf8.begin());
        case NameKind::UNIQUE:
            return this->unique.original.data(gs)->shortName(gs);
        case NameKind::CONSTANT:
            return this->cnst.original.data(gs)->shortName(gs);
    }
}

void Name::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode) {
        return;
    }
    NameRef current = this->ref(gs);
    switch (this->kind) {
        case NameKind::UTF8:
            ENFORCE_NO_TIMER(current == const_cast<GlobalState &>(gs).enterNameUTF8(this->raw.utf8),
                             "Name table corrupted, re-entering UTF8 name gives different id");
            break;
        case NameKind::UNIQUE: {
            ENFORCE_NO_TIMER(this->unique.original._id < current._id, "unique name id not bigger than original");
            ENFORCE_NO_TIMER(this->unique.num > 0, "unique num == 0");
            NameRef current2 = const_cast<GlobalState &>(gs).freshNameUnique(this->unique.uniqueNameKind,
                                                                             this->unique.original, this->unique.num);
            ENFORCE_NO_TIMER(current == current2, "Name table corrupted, re-entering UNIQUE name gives different id");
            break;
        }
        case NameKind::CONSTANT:
            ENFORCE_NO_TIMER(this->cnst.original._id < current._id, "constant name id not bigger than original");
            ENFORCE_NO_TIMER(current == const_cast<GlobalState &>(gs).enterNameConstant(this->cnst.original),
                             "Name table corrupted, re-entering CONSTANT name gives different id");
            break;
    }
}

NameRef Name::ref(const GlobalState &gs) const {
    auto distance = this - gs.names.data();
    return NameRef(gs, distance);
}

bool Name::isClassName(const GlobalState &gs) const {
    switch (this->kind) {
        case NameKind::UTF8:
            return false;
        case NameKind::UNIQUE: {
            return (this->unique.uniqueNameKind == UniqueNameKind::Singleton ||
                    this->unique.uniqueNameKind == UniqueNameKind::MangleRename ||
                    this->unique.uniqueNameKind == UniqueNameKind::TEnum) &&
                   this->unique.original.data(gs)->isClassName(gs);
        }
        case NameKind::CONSTANT:
            ENFORCE(this->cnst.original.data(gs)->kind == NameKind::UTF8 ||
                    this->cnst.original.data(gs)->kind == NameKind::UNIQUE &&
                        (this->cnst.original.data(gs)->unique.uniqueNameKind == UniqueNameKind::ResolverMissingClass ||
                         this->cnst.original.data(gs)->unique.uniqueNameKind == UniqueNameKind::TEnum));
            return true;
    }
}

bool Name::isTEnumName(const GlobalState &gs) const {
    if (this->kind != NameKind::CONSTANT) {
        return false;
    }
    auto original = this->cnst.original;
    return original.data(gs)->kind == NameKind::UNIQUE &&
           original.data(gs)->unique.uniqueNameKind == UniqueNameKind::TEnum;
}

NameRefDebugCheck::NameRefDebugCheck(const GlobalState &gs, int _id) {
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

void NameRefDebugCheck::check(const GlobalState &gs, int _id) const {
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
    runDebugOnlyCheck(gs, id());
}

void NameRef::sanityCheckSubstitution(const GlobalSubstitution &subst) const {
    runDebugOnlyCheck(subst);
}

NameData NameRef::data(GlobalState &gs) const {
    ENFORCE(_id < gs.names.size(), "name id out of bounds");
    ENFORCE(exists(), "non existing name");
    enforceCorrectGlobalState(gs);
    return NameData(gs.names[_id], gs);
}

const NameData NameRef::data(const GlobalState &gs) const {
    ENFORCE(_id < gs.names.size(), "name {} id out of bounds {}", _id, gs.names.size());
    ENFORCE(exists(), "non existing name");
    enforceCorrectGlobalState(gs);
    return NameData(const_cast<Name &>(gs.names[_id]), gs);
}
string NameRef::showRaw(const GlobalState &gs) const {
    return data(gs)->showRaw(gs);
}
string NameRef::toString(const GlobalState &gs) const {
    return data(gs)->toString(gs);
}
string NameRef::show(const GlobalState &gs) const {
    return data(gs)->show(gs);
}

NameRef NameRef::addEq(GlobalState &gs) const {
    auto name = this->data(gs);
    ENFORCE(name->kind == NameKind::UTF8, "addEq over non-utf8 name");
    string nameEq = absl::StrCat(name->raw.utf8, "=");
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::addQuestion(GlobalState &gs) const {
    auto name = this->data(gs);
    ENFORCE(name->kind == NameKind::UTF8, "addQuestion over non-utf8 name");
    string nameEq = absl::StrCat(name->raw.utf8, "?");
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::addAt(GlobalState &gs) const {
    auto name = this->data(gs);
    ENFORCE(name->kind == NameKind::UTF8, "addAt over non-utf8 name");
    string nameEq = absl::StrCat("@", name->raw.utf8);
    return gs.enterNameUTF8(nameEq);
}

NameRef NameRef::prepend(GlobalState &gs, string_view s) const {
    auto name = this->data(gs);
    ENFORCE(name->kind == NameKind::UTF8, "prepend over non-utf8 name");
    string nameEq = absl::StrCat(s, name->raw.utf8);
    return gs.enterNameUTF8(nameEq);
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
            out.unique.original = NameRef(to, this->unique.original.id());
            break;

        case NameKind::CONSTANT:
            out.cnst.original = NameRef(to, this->cnst.original.id());
            break;
    }

    return out;
}

NameData::NameData(Name &ref, const GlobalState &gs) : DebugOnlyCheck(gs), name(ref) {}

NameDataDebugCheck::NameDataDebugCheck(const GlobalState &gs) : gs(gs), nameCountAtCreation(gs.namesUsed()) {}

void NameDataDebugCheck::check() const {
    ENFORCE(nameCountAtCreation == gs.namesUsed());
}

Name *NameData::operator->() {
    runDebugOnlyCheck();
    return &name;
};

const Name *NameData::operator->() const {
    runDebugOnlyCheck();
    return &name;
};

} // namespace sorbet::core
