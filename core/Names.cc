#include "Names.h"
#include "Context.h"
#include "Hashing.h"
#include <numeric> // accumulate

using namespace std;

namespace ruby_typer {
namespace core {

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
            DEBUG_ONLY(Error::raise("Unknown name kind?", kind);)
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
            }
            return this->unique.original.name(gs).toString(gs) + "$" + to_string(this->unique.num);
        case CONSTANT:
            return "<constant:" + this->cnst.original.toString(gs) + ">";
        default:
            Error::notImplemented();
    }
}

Name &NameRef::name(GlobalState &gs) const {
    DEBUG_ONLY(Error::check(_id < gs.names.size()));
    DEBUG_ONLY(Error::check(exists()));
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
    Error::check(name.kind == UTF8);
    string nameEq = string(name.raw.utf8.begin(), name.raw.utf8.end()) + "=";
    return gs.enterNameUTF8(nameEq);
}

} // namespace core
} // namespace ruby_typer
