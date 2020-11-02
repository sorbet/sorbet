#include "core/GlobalSubstitution.h"
#include "core/GlobalState.h"
#include "core/Names.h"
using namespace std;
namespace sorbet::core {

LazyGlobalSubstitution::LazyGlobalSubstitution(const GlobalState &fromGS, GlobalState &toGS)
    : fromGS(fromGS), toGS(toGS){};

void LazyGlobalSubstitution::defineName(NameRef from, NameRef &to) {
    auto &nm = from.data(this->fromGS);
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
    }
}

core::UsageHash LazyGlobalSubstitution::getAllNames() {
    core::NameHash::sortAndDedupe(acc.sends);
    core::NameHash::sortAndDedupe(acc.constants);
    return move(acc);
}
} // namespace sorbet::core