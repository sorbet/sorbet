#include "core/packages/PackageInfo.h"
#include "absl/strings/str_join.h"
#include "core/GlobalState.h"
#include "core/Loc.h"
#include "core/NameRef.h"
#include "core/Symbols.h"

using namespace std;

namespace sorbet::core::packages {
string_view strictDependenciesLevelToString(core::packages::StrictDependenciesLevel level) {
    switch (level) {
        case core::packages::StrictDependenciesLevel::False:
            return "false";
        case core::packages::StrictDependenciesLevel::Layered:
            return "layered";
        case core::packages::StrictDependenciesLevel::LayeredDag:
            return "layered_dag";
        case core::packages::StrictDependenciesLevel::Dag:
            return "dag";
    }
}

bool PackageInfo::exists() const {
    return mangledName().exists();
}

bool PackageInfo::operator==(const PackageInfo &rhs) const {
    return mangledName() == rhs.mangledName();
}

PackageInfo::~PackageInfo() {
    // see https://eli.thegreenplace.net/2010/11/13/pure-virtual-destructors-in-c
}

string PackageInfo::show(const core::GlobalState &gs) const {
    return this->mangledName().owner.show(gs);
}

} // namespace sorbet::core::packages
