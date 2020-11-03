#include "core/GlobalSubstitution.h"
#include "core/GlobalState.h"
#include "core/Names.h"
using namespace std;
namespace sorbet::core {

LazyGlobalSubstitution::LazyGlobalSubstitution(const GlobalState &fromGS, GlobalState &toGS)
    : fromGS(fromGS), toGS(toGS) {
    // Pre-define an entry for the empty name.
    nameSubstitution[core::NameRef()] = core::NameRef();
};

NameRef LazyGlobalSubstitution::defineName(NameRef from, bool allowSameFromTo) {
    // Avoid failures in debug builds.
    auto &nm = allowSameFromTo ? core::NameRef(fromGS, from.id()).data(this->fromGS) : from.data(this->fromGS);
    NameRef to;
    switch (nm->kind) {
        case NameKind::UNIQUE:
            to = this->toGS.freshNameUnique(nm->unique.uniqueNameKind, substitute(nm->unique.original), nm->unique.num);
            break;
        case NameKind::UTF8:
            to = this->toGS.enterNameUTF8(nm->raw.utf8);
            break;
        case NameKind::CONSTANT:
            to = this->toGS.enterNameConstant(substitute(nm->cnst.original));
            break;
        default:
            ENFORCE(false, "NameKind missing");
            break;
    }
    nameSubstitution[from] = to;
    return to;
}

core::UsageHash LazyGlobalSubstitution::getAllNames() {
    core::NameHash::sortAndDedupe(acc.sends);
    core::NameHash::sortAndDedupe(acc.constants);
    return move(acc);
}
} // namespace sorbet::core