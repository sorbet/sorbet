#include "core/packages/MangledName.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core::packages {
MangledName MangledName::lookupMangledName(const GlobalState &gs, const vector<string> &parts) {
    auto owner = core::Symbols::PackageSpecRegistry();
    for (auto part : parts) {
        auto member = owner.data(gs)->findMember(gs, gs.lookupNameConstant(part));
        if (!member.isClassOrModule()) {
            owner = core::Symbols::noClassOrModule();
            break;
        }
        owner = member.asClassOrModuleRef();
    }

    if (owner == core::Symbols::PackageSpecRegistry()) {
        owner = core::Symbols::noClassOrModule();
    }

    return MangledName(owner);
}

} // namespace sorbet::core::packages
