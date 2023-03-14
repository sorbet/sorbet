#include "core/NameSubstitution.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Unfreeze.h"
using namespace std;
namespace sorbet::core {

NameSubstitution::NameSubstitution(const GlobalState &from, GlobalState &to) : toGlobalStateId(to.globalStateId) {
    Timer timeit(to.tracer(), "NameSubstitution.new", from.creation);

    SLOW_DEBUG_ONLY(from.sanityCheck());

    {
        UnfreezeNameTable unfreezeNames(to);
        utf8NameSubstitution.reserve(from.utf8Names.size());
        constantNameSubstitution.reserve(from.constantNames.size());
        uniqueNameSubstitution.reserve(from.uniqueNames.size());
        int i = -1;
        for (const UTF8Name &nm : from.utf8Names) {
            i++;
            ENFORCE_NO_TIMER(utf8NameSubstitution.size() == i, "UTF8 name substitution has wrong size");
            utf8NameSubstitution.emplace_back(to.enterNameUTF8(nm.utf8));
        }
        // UniqueNames and ConstantNames may reference each other, necessitating some special logic here to avoid
        // crashing. We process UniqueNames first because there are fewer of them, so fewer loop iterations require
        // this special check. Tested in `core_test.cc`.
        i = -1;
        for (const UniqueName &nm : from.uniqueNames) {
            i++;
            ENFORCE(uniqueNameSubstitution.size() == i, "Unique name substitution has wrong size");
            if (nm.original.kind() == NameKind::CONSTANT &&
                nm.original.constantIndex() >= constantNameSubstitution.size()) {
                // Note: Duplicate of loop body below. If you change one, change the other!
                for (uint32_t i = constantNameSubstitution.size(); i <= nm.original.constantIndex(); i++) {
                    auto &cnst = from.constantNames[i];
                    ENFORCE_NO_TIMER(constantNameSubstitution.size() == i, "Constant name substitution has wrong size");
                    // N.B.: cnst may reference a UniqueName, but since names are linearizeable we should have
                    // already substituted it by now.
                    constantNameSubstitution.emplace_back(to.enterNameConstant(substitute(cnst.original)));
                }
            }

            uniqueNameSubstitution.emplace_back(to.freshNameUnique(nm.uniqueNameKind, substitute(nm.original), nm.num));
        }
        for (i = constantNameSubstitution.size(); i < from.constantNames.size(); i++) {
            ENFORCE_NO_TIMER(constantNameSubstitution.size() == i, "Constant name substitution has wrong size");
            auto &nm = from.constantNames[i];
            constantNameSubstitution.emplace_back(to.enterNameConstant(substitute(nm.original)));
        }
    }

    for (auto &extension : to.semanticExtensions) {
        extension->merge(from, to, *this);
    }

    SLOW_DEBUG_ONLY(to.sanityCheck());
}

LazyNameSubstitution::LazyNameSubstitution(const GlobalState &fromGS, GlobalState &toGS) : fromGS(fromGS), toGS(toGS) {
    // Pre-define an entry for the empty name.
    nameSubstitution[core::NameRef()] = core::NameRef();
};

NameRef LazyNameSubstitution::defineName(NameRef from) {
    ENFORCE_NO_TIMER(&fromGS != &toGS);

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

core::UsageHash LazyNameSubstitution::getAllNames() {
    core::WithoutUniqueNameHash::sortAndDedupe(acc.nameHashes);
    return move(acc);
}
} // namespace sorbet::core
