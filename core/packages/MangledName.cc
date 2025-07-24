#include "core/packages/MangledName.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core::packages {
MangledName MangledName::lookupMangledName(const GlobalState &gs, const vector<string> &parts) {
    auto owner = core::Symbols::root();
    for (auto part : parts) {
        auto member = owner.data(gs)->findMember(gs, gs.lookupNameConstant(part));
        if (!member.exists() || !member.isClassOrModule()) {
            owner = core::Symbols::noClassOrModule();
            break;
        }
        owner = member.asClassOrModuleRef();
    }

    if (owner == core::Symbols::root()) {
        return MangledName();
    }

    auto packageSpecStorage = owner.data(gs)->findMember(gs, core::Names::Constants::PackageSpec_Storage());
    if (!packageSpecStorage.isClassOrModule()) {
        return MangledName();
    }

    return MangledName(packageSpecStorage.asClassOrModuleRef());
}

} // namespace sorbet::core::packages
