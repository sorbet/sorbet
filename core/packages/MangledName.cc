#include "core/packages/MangledName.h"
#include "absl/strings/str_split.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core::packages {
MangledName MangledName::lookupMangledName(const GlobalState &gs, const vector<string> &parts) {
    auto owner = core::Symbols::PackageSpecRegistry();
    for (auto part : parts) {
        auto member = owner.data(gs)->findMember(gs, gs.lookupNameConstant(part));
        if (!member.exists() || !member.isClassOrModule()) {
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

MangledName MangledName::parse(const GlobalState &gs, std::string_view pkgName) {
    auto parts = absl::StrSplit(pkgName, "::");
    return MangledName::lookupMangledName(gs, parts);
}

UnorderedSet<MangledName> MangledName::parseNames(const core::GlobalState &gs, absl::Span<const std::string> pkgNames) {
    UnorderedSet<MangledName> names;
    for (auto pkgName : pkgNames) {
        names.insert(MangledName::parse(gs, pkgName));
    }
    return names;
}

} // namespace sorbet::core::packages
