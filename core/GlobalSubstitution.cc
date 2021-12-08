#include "core/GlobalSubstitution.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Unfreeze.h"
using namespace std;
namespace sorbet::core {

GlobalSubstitution::GlobalSubstitution(const GlobalState &from, GlobalState &to,
                                       const GlobalState *optionalCommonParent)
    : toGlobalStateId(to.globalStateId) {
    Timer timeit(to.tracer(), "GlobalSubstitution.new", from.creation);
    ENFORCE(toGlobalStateId != 0, "toGlobalStateId is only used for sanity checks, but should always be set.");
    ENFORCE(from.classAndModules.size() == to.classAndModules.size(), "Can't substitute symbols yet");
    ENFORCE(from.methods.size() == to.methods.size(), "Can't substitute symbols yet");
    ENFORCE(from.fields.size() == to.fields.size(), "Can't substitute symbols yet");
    ENFORCE(from.typeArguments.size() == to.typeArguments.size(), "Can't substitute symbols yet");
    ENFORCE(from.typeMembers.size() == to.typeMembers.size(), "Can't substitute symbols yet");

    from.sanityCheck();
    {
        UnfreezeFileTable unfreezeFiles(to);
        int fileIdx = 0; // Skip file 0
        while (fileIdx + 1 < from.filesUsed()) {
            fileIdx++;
            if (from.files[fileIdx]->sourceType == File::Type::NotYetRead) {
                continue;
            }
            if (fileIdx < to.filesUsed() && from.files[fileIdx].get() == to.files[fileIdx].get()) {
                continue;
            }
            ENFORCE(fileIdx >= to.filesUsed() || to.files[fileIdx]->sourceType == File::Type::NotYetRead);
            to.enterNewFileAt(from.files[fileIdx], fileIdx);
        }
    }

    fastPath = false;
    if (optionalCommonParent != nullptr) {
        if (from.namesUsedTotal() == optionalCommonParent->namesUsedTotal() &&
            from.symbolsUsedTotal() == optionalCommonParent->symbolsUsedTotal()) {
            ENFORCE(to.namesUsedTotal() >= from.namesUsedTotal());
            ENFORCE(to.symbolsUsedTotal() >= from.symbolsUsedTotal());
            fastPath = true;
        }
    }

    if (!fastPath || debug_mode) {
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
                ENFORCE(!fastPath || utf8NameSubstitution.back().utf8Index() == i);
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
                        ENFORCE_NO_TIMER(constantNameSubstitution.size() == i,
                                         "Constant name substitution has wrong size");
                        // N.B.: cnst may reference a UniqueName, but since names are linearizeable we should have
                        // already substituted it by now.
                        constantNameSubstitution.emplace_back(to.enterNameConstant(substitute(cnst.original)));
                    }
                }

                uniqueNameSubstitution.emplace_back(
                    to.freshNameUnique(nm.uniqueNameKind, substitute(nm.original), nm.num));
                ENFORCE(!fastPath || uniqueNameSubstitution.back().uniqueIndex() == i);
            }
            for (i = constantNameSubstitution.size(); i < from.constantNames.size(); i++) {
                ENFORCE_NO_TIMER(constantNameSubstitution.size() == i, "Constant name substitution has wrong size");
                auto &nm = from.constantNames[i];
                constantNameSubstitution.emplace_back(to.enterNameConstant(substitute(nm.original)));
                ENFORCE(!fastPath || constantNameSubstitution.back().constantIndex() == i);
            }
        }

        // Enforce that the symbol tables are the same
        for (int i = 0; i < from.classAndModules.size(); ++i) {
            ENFORCE(substitute(from.classAndModules[i].name) == from.classAndModules[i].name);
            ENFORCE(from.classAndModules[i].name == to.classAndModules[i].name);
        }
        for (int i = 0; i < from.methods.size(); ++i) {
            ENFORCE(substitute(from.methods[i].name) == from.methods[i].name);
            ENFORCE(from.methods[i].name == to.methods[i].name);
        }
        for (int i = 0; i < from.fields.size(); ++i) {
            ENFORCE(substitute(from.fields[i].name) == from.fields[i].name);
            ENFORCE(from.fields[i].name == to.fields[i].name);
        }
        for (int i = 0; i < from.typeArguments.size(); ++i) {
            ENFORCE(substitute(from.typeArguments[i].name) == from.typeArguments[i].name);
            ENFORCE(from.typeArguments[i].name == to.typeArguments[i].name);
        }
        for (int i = 0; i < from.typeMembers.size(); ++i) {
            ENFORCE(substitute(from.typeMembers[i].name) == from.typeMembers[i].name);
            ENFORCE(from.typeMembers[i].name == to.typeMembers[i].name);
        }
    }

    for (auto &extension : to.semanticExtensions) {
        extension->merge(from, to, *this);
    }

    to.sanityCheck();
}

bool GlobalSubstitution::useFastPath() const {
    return fastPath;
}

LazyGlobalSubstitution::LazyGlobalSubstitution(const GlobalState &fromGS, GlobalState &toGS)
    : fromGS(fromGS), toGS(toGS) {
    // Pre-define an entry for the empty name.
    nameSubstitution[core::NameRef()] = core::NameRef();
};

NameRef LazyGlobalSubstitution::defineName(NameRef from, bool allowSameFromTo) {
    ENFORCE_NO_TIMER(&fromGS != &toGS);

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
    core::NameHash::sortAndDedupe(acc.symbols);
    return move(acc);
}
} // namespace sorbet::core
