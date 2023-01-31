#include "core/packages/PackageInfo.h"
#include "absl/strings/str_join.h"
#include "core/GlobalState.h"
#include "core/Loc.h"
#include "core/NameRef.h"
#include "core/Symbols.h"

using namespace std;

namespace sorbet::core::packages {
bool PackageInfo::exists() const {
    return mangledName().exists();
}

bool PackageInfo::operator==(const PackageInfo &rhs) const {
    return mangledName() == rhs.mangledName();
}

PackageInfo::~PackageInfo() {
    // see https://eli.thegreenplace.net/2010/11/13/pure-virtual-destructors-in-c
}

bool PackageInfo::lexCmp(const std::vector<core::NameRef> &lhs, const std::vector<core::NameRef> &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                                        [](NameRef a, NameRef b) -> bool { return a.rawId() < b.rawId(); });
}

ImportInfo ImportInfo::fromPackage(const core::GlobalState &gs, const PackageInfo &info) {
    ImportInfo res;
    res.package = info.mangledName();

    auto &thisName = info.fullName();

    auto &db = gs.packageDB();

    for (auto pkg : db.packages()) {
        if (!info.importsPackage(pkg)) {
            continue;
        }

        auto &fullName = db.getPackageInfo(pkg).fullName();
        if (thisName.size() >= fullName.size()) {
            if (std::equal(fullName.begin(), fullName.end(), thisName.begin())) {
                res.parentImports.emplace_back(pkg);
                continue;
            }
        }

        res.regularImports.emplace_back(pkg);
    }

    return res;
}

string PackageInfo::show(const core::GlobalState &gs) const {
    return absl::StrJoin(fullName(),
                         "::", [&](string *out, core::NameRef name) { absl::StrAppend(out, name.show(gs)); });
}

core::ClassOrModuleRef getTestSym(const core::GlobalState &gs) {
    return core::Symbols::root().data(gs)->findMember(gs, core::Names::Constants::Test()).asClassOrModuleRef();
}

core::ClassOrModuleRef getParentNamespaceSym(const core::GlobalState &gs, const core::SymbolRef sym) {
    auto testSym = getTestSym(gs);
    if (!testSym.exists()) {
        return core::Symbols::root();
    }

    if (sym.isUnderNamespace(gs, testSym)) {
        return testSym;
    }

    return core::Symbols::root();
}

core::ClassOrModuleRef lookupNameOn(const core::GlobalState &gs, const core::ClassOrModuleRef root,
                                    const std::vector<core::NameRef> &name) {
    auto curSym = root;
    if (!curSym.exists()) {
        return {};
    }

    for (const auto part : name) {
        auto member = curSym.data(gs)->findMember(gs, part);
        if (!member.exists() || !member.isClassOrModule()) {
            return {};
        }
        curSym = member.asClassOrModuleRef();
    }

    return curSym;
}

core::ClassOrModuleRef PackageInfo::getPackageScope(const core::GlobalState &gs) const {
    return lookupNameOn(gs, core::Symbols::root(), fullName());
}

core::ClassOrModuleRef PackageInfo::getPackageTestScope(const core::GlobalState &gs) const {
    auto testSym = core::Symbols::root().data(gs)->findMember(gs, core::Names::Constants::Test());
    if (!testSym.isClassOrModule()) {
        return {};
    }

    return lookupNameOn(gs, testSym.asClassOrModuleRef(), fullName());
}

// Given a package named Project::MyPackage, returns the class/module ref corresponding to
// the symbol Project::MyPackage or Test::Project::MyPackage, depending on whether the suggestion scope
// is a primary namespace constant or a test namespace constant. See packager/packager.cc for further explanation of
// test namespaces.
core::ClassOrModuleRef PackageInfo::getRootSymbolForAutocorrectSearch(const core::GlobalState &gs,
                                                                      const core::SymbolRef suggestionScope) const {
    auto parentSym = getParentNamespaceSym(gs, suggestionScope);
    return lookupNameOn(gs, parentSym, fullName());
}

} // namespace sorbet::core::packages
