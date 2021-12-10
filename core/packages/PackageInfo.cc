#include "core/packages/PackageInfo.h"
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

bool PackageInfo::isPackageModule(const core::GlobalState &gs, core::ClassOrModuleRef klass) {
    while (klass.exists() && klass != core::Symbols::root()) {
        if (klass == core::Symbols::PackageRegistry() || klass == core::Symbols::PackageTests()) {
            return true;
        }
        klass = klass.data(gs)->owner;
    }
    return false;
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
        auto &pkgInfo = db.getPackageInfo(pkg);
        if (!info.importsPackage(pkgInfo)) {
            continue;
        }

        auto &fullName = pkgInfo.fullName();

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

} // namespace sorbet::core::packages
