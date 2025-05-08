#include "core/packages/MangledName.h"
#include "absl/strings/str_join.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core::packages {
MangledName MangledName::mangledNameFromParts(GlobalState &gs, const std::vector<std::string_view> &parts,
                                              ClassOrModuleRef owner) {
    // Foo::Bar => Foo_Bar
    auto mangledName = absl::StrJoin(parts, "_");

    auto utf8Name = gs.enterNameUTF8(mangledName);
    auto packagerName = gs.freshNameUnique(UniqueNameKind::Packager, utf8Name, 1);
    gs.enterNameConstant(packagerName);
    return MangledName(owner);
}

MangledName MangledName::mangledNameFromParts(GlobalState &gs, const std::vector<NameRef> &parts,
                                              ClassOrModuleRef owner) {
    // Foo::Bar => Foo_Bar
    auto mangledName = absl::StrJoin(parts, "_", NameFormatter(gs));

    auto utf8Name = gs.enterNameUTF8(mangledName);
    auto packagerName = gs.freshNameUnique(UniqueNameKind::Packager, utf8Name, 1);
    gs.enterNameConstant(packagerName);
    return MangledName(owner);
}

MangledName MangledName::lookupMangledName(const GlobalState &gs, const vector<string> &parts) {
    // Foo::Bar => Foo_Bar
    auto mangledName = absl::StrJoin(parts, "_");

    auto utf8Name = gs.lookupNameUTF8(mangledName);
    if (!utf8Name.exists()) {
        return MangledName();
    }

    auto packagerName = gs.lookupNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    if (!packagerName.exists()) {
        return MangledName();
    }

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

    gs.lookupNameConstant(packagerName);
    return MangledName(owner);
}

} // namespace sorbet::core::packages
