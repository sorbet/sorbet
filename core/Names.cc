#include "Names.h"
#include "Context.h"
#include "Hashing.h"
#include "core/Names/core.h"
#include <numeric> // accumulate

using namespace std;

namespace ruby_typer {
namespace core {

NameRef::NameRef(const GlobalState &gs, unsigned int id) : _id(id) {
#ifdef DEBUG_MODE
    globalStateId = gs.globalStateId;
#endif
}

bool NameRef::isWellKnownName() const {
    return _id <= Names::LAST_WELL_KNOWN_NAME;
}

ruby_typer::core::Name::~Name() noexcept {
    if (kind == NameKind::UNIQUE) {
        unique.~UniqueName();
    }
}

unsigned int Name::hashNames(vector<NameRef> &lhs, GlobalState &gs) {
    return accumulate(lhs.begin(), lhs.end(), 0, [](int acc, NameRef &necc) -> int { return mix(acc, necc.id()); }) *
               8 +
           lhs.size();
}

unsigned int Name::hash(const GlobalState &gs) const {
    // TODO: use https://github.com/Cyan4973/xxHash
    // !!! keep this in sync with GlobalState.enter*
    switch (kind) {
        case UTF8:
            return _hash(raw.utf8);
        case UNIQUE:
            return _hash_mix_unique((u2)unique.uniqueNameKind, UNIQUE, unique.num, unique.original.id());
        case CONSTANT:
            return _hash_mix_constant(CONSTANT, cnst.original.id());
        default:
            Error::raise("Unknown name kind?", kind);
    }
}

string Name::toString(GlobalState &gs) const {
    switch (this->kind) {
        case UTF8:
            return string(raw.utf8.begin(), raw.utf8.end());
        case UNIQUE:
            if (this->unique.uniqueNameKind == UniqueNameKind::Singleton) {
                return "<singleton class:" + this->unique.original.name(gs).toString(gs) + ">";
            } else if (this->unique.uniqueNameKind == UniqueNameKind::NestedScope) {
                return "<block-nested: " + this->unique.original.name(gs).toString(gs) + ">";
            } else if (this->unique.uniqueNameKind == UniqueNameKind::Overload) {
                return "<overload N." + to_string(this->unique.num) + " : " +
                       this->unique.original.name(gs).toString(gs) + ">";
            }
            return this->unique.original.name(gs).toString(gs) + "$" + to_string(this->unique.num);
        case CONSTANT:
            return "<constant:" + this->cnst.original.toString(gs) + ">";
        default:
            Error::notImplemented();
    }
}

void Name::sanityCheck(const GlobalState &gs) const {
    if (!debug_mode)
        return;
    NameRef current = this->ref(gs);
    switch (this->kind) {
        case UTF8:
            ENFORCE(current == const_cast<GlobalState &>(gs).enterNameUTF8(this->raw.utf8),
                    "Name table corrupted, re-entering UTF8 name gives different id");
            break;
        case UNIQUE: {
            ENFORCE(this->unique.original._id < current._id, "unique name id not bigger than original");
            ENFORCE(this->unique.num > 0, "unique num == 0");
            NameRef current2 = const_cast<GlobalState &>(gs).freshNameUnique(this->unique.uniqueNameKind,
                                                                             this->unique.original, this->unique.num);
            ENFORCE(current == current2, "Name table corrupted, re-entering UNIQUE name gives different id");
            break;
        }
        case CONSTANT:
            ENFORCE(this->cnst.original._id < current._id, "constant name id not bigger than original");
            ENFORCE(current == const_cast<GlobalState &>(gs).enterNameConstant(this->cnst.original),
                    "Name table corrupted, re-entering CONSTANT name gives different id");
            break;
        default:
            Error::notImplemented();
    }
}

NameRef Name::ref(const GlobalState &gs) const {
    auto distance = this - gs.names.data();
    return NameRef(gs, distance);
}

bool Name::isClassName(GlobalState &gs) const {
    switch (this->kind) {
        case UTF8:
            return false;
        case UNIQUE: {
            return this->unique.uniqueNameKind == core::Singleton && this->unique.original.name(gs).isClassName(gs);
        }
        case CONSTANT:
            ENFORCE(this->cnst.original.name(gs).kind == UTF8);
            return true;
        default:
            Error::notImplemented();
    }
}

Name &NameRef::name(GlobalState &gs) const {
    ENFORCE(_id < gs.names.size(), "name id out of bounds");
    ENFORCE(exists(), "non existing name");
#ifdef DEBUG_MODE
    ENFORCE(isWellKnownName() || globalStateId == gs.globalStateId);
#endif
    return gs.names[_id];
}
string NameRef::toString(GlobalState &gs) const {
    return name(gs).toString(gs);
}

bool NameRef::isBlockClashSafe(GlobalState &gs) const {
    Name &nm = this->name(gs);
    return nm.kind == NameKind ::UNIQUE && (nm.unique.uniqueNameKind == UniqueNameKind ::CFG ||
                                            nm.unique.uniqueNameKind == UniqueNameKind ::NestedScope);
}

NameRef NameRef::addEq(GlobalState &gs) const {
    Name &name = this->name(gs);
    ENFORCE(name.kind == UTF8, "addEq over non-utf8 name");
    string nameEq = string(name.raw.utf8.begin(), name.raw.utf8.end()) + "=";
    return gs.enterNameUTF8(nameEq);
}

Name Name::deepCopy(const GlobalState &to) const {
    Name out;
    out.kind = this->kind;

    switch (this->kind) {
        case UTF8:
            out.raw = this->raw;
            break;

        case UNIQUE:
            out.unique.uniqueNameKind = this->unique.uniqueNameKind;
            out.unique.num = this->unique.num;
            out.unique.original = NameRef(to, this->unique.original.id());
            break;

        case CONSTANT:
            out.cnst.original = NameRef(to, this->cnst.original.id());
            break;

        default:
            Error::notImplemented();
    }

    return out;
}

} // namespace core
} // namespace ruby_typer
