#include "core/NameSubstitution.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Unfreeze.h"
using namespace std;
namespace sorbet::core {

NameSubstitution::NameSubstitution(const GlobalState &from, GlobalState &to) : NameSubstitution(to) {
    Timer timeit(to.tracer(), "NameSubstitution.new", from.creation);
    extend(from, to);
    mergeExtensions(from, to);
}

NameSubstitution::NameSubstitution(const GlobalState &to) : toGlobalStateId(to.globalStateId) {}

void NameSubstitution::mergeExtensions(const GlobalState &from, GlobalState &to) {
    ENFORCE_NO_TIMER(to.globalStateId == toGlobalStateId);
    for (auto &extension : to.semanticExtensions) {
        extension->merge(from, to, *this);
    }

    SLOW_DEBUG_ONLY(to.sanityCheck());
}

void NameSubstitution::extend(const GlobalState &from, GlobalState &to) {
    Timer timeit(to.tracer(), "NameSubstitution.extend");
    ENFORCE_NO_TIMER(to.globalStateId == toGlobalStateId);
    ENFORCE_NO_TIMER(utf8NameSubstitution.size() <= from.utf8Names.size());
    ENFORCE_NO_TIMER(constantNameSubstitution.size() <= from.constantNames.size());
    ENFORCE_NO_TIMER(uniqueNameSubstitution.size() <= from.uniqueNames.size());

    SLOW_DEBUG_ONLY(from.sanityCheck());

    {
        UnfreezeNameTable unfreezeNames(to);
        utf8NameSubstitution.reserve(from.utf8Names.size());
        constantNameSubstitution.reserve(from.constantNames.size());
        uniqueNameSubstitution.reserve(from.uniqueNames.size());
        // Hash the whole batch first, so that we can prefetch the bucket in `to` that each lookup will probe. With
        // millions of names `to`'s table is far larger than the caches, so this loop is otherwise bound by one cache
        // miss per name.
        const auto utf8Begin = utf8NameSubstitution.size();
        const auto utf8End = from.utf8Names.size();
        vector<NameHash::Hash> hashes;
        hashes.reserve(utf8End - utf8Begin);
        for (auto i = utf8Begin; i < utf8End; i++) {
            hashes.emplace_back(NameHash::hashMixUTF8(from.utf8Names[i].utf8));
        }
        constexpr size_t prefetchDistance = 8;
        for (auto i = utf8Begin; i < utf8End; i++) {
            if (i + prefetchDistance < utf8End) {
                to.namesByHash.prefetch(hashes[i + prefetchDistance - utf8Begin]);
            }
            ENFORCE_NO_TIMER(utf8NameSubstitution.size() == i, "UTF8 name substitution has wrong size");
            utf8NameSubstitution.emplace_back(to.enterNameUTF8(from.utf8Names[i].utf8, hashes[i - utf8Begin]));
        }
        // UniqueNames and ConstantNames may reference each other, necessitating some special logic here to avoid
        // crashing. We process UniqueNames first because there are fewer of them, so fewer loop iterations require
        // this special check. Tested in `core_test.cc`.
        for (auto i = uniqueNameSubstitution.size(); i < from.uniqueNames.size(); i++) {
            const UniqueName &nm = from.uniqueNames[i];
            ENFORCE(uniqueNameSubstitution.size() == i, "Unique name substitution has wrong size");
            if (nm.original.kind() == NameKind::CONSTANT &&
                nm.original.constantIndex() >= constantNameSubstitution.size()) {
                // Note: Duplicate of loop body below. If you change one, change the other!
                for (auto j = constantNameSubstitution.size(); j <= nm.original.constantIndex(); j++) {
                    auto &cnst = from.constantNames[j];
                    ENFORCE_NO_TIMER(constantNameSubstitution.size() == j, "Constant name substitution has wrong size");
                    // N.B.: cnst may reference a UniqueName, but since names are linearizeable we should have
                    // already substituted it by now.
                    constantNameSubstitution.emplace_back(to.enterNameConstant(substitute(cnst.original)));
                }
            }

            uniqueNameSubstitution.emplace_back(to.freshNameUnique(nm.uniqueNameKind, substitute(nm.original), nm.num));
        }
        for (auto i = constantNameSubstitution.size(); i < from.constantNames.size(); i++) {
            ENFORCE_NO_TIMER(constantNameSubstitution.size() == i, "Constant name substitution has wrong size");
            auto &nm = from.constantNames[i];
            constantNameSubstitution.emplace_back(to.enterNameConstant(substitute(nm.original)));
        }
    }
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
