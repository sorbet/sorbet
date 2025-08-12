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
    return absl::StrJoin(fullName(),
                         "::", [&](string *out, core::NameRef name) { absl::StrAppend(out, name.show(gs)); });
}

namespace {

core::ClassOrModuleRef lookupNameOn(const core::GlobalState &gs, const core::ClassOrModuleRef root,
                                    absl::Span<const core::NameRef> name) {
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

} // namespace

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

} // namespace sorbet::core::packages
