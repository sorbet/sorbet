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
    from = allowSameFromTo ? core::NameRef::fromRaw(fromGS, from.rawId()) : from;
    NameRef to;
    switch (from.kind()) {
        case NameKind::UNIQUE: {
            auto unique = from.dataUnique(fromGS);
            to = this->toGS.freshNameUnique(unique->uniqueNameKind, substitute(unique->original), unique->num);
            break;
        }
        case NameKind::UTF8: {
            auto utf8 = from.dataUtf8(fromGS);
            to = this->toGS.enterNameUTF8(utf8->utf8);
            break;
        }
        case NameKind::CONSTANT: {
            auto constant = from.dataCnst(fromGS);
            to = this->toGS.enterNameConstant(substitute(constant->original));
            break;
        }
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
