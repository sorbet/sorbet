#include "core/packages/PackageInfo.h"
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
        klass = klass.data(gs)->owner.asClassOrModuleRef();
    }
    return false;
}

bool PackageInfo::lexCmp(const std::vector<core::NameRef> &lhs, const std::vector<core::NameRef> &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                                        [](NameRef a, NameRef b) -> bool { return a.rawId() < b.rawId(); });
}
} // namespace sorbet::core::packages
