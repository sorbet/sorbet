#include "core/packages/MangledName.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core::packages {
MangledName MangledName::mangledNameFromParts(core::GlobalState &gs, const std::vector<std::string_view> &parts) {
    // Foo::Bar => Foo_Bar_Package
    auto mangledName = absl::StrCat(absl::StrJoin(parts, "_"), core::PACKAGE_SUFFIX);

    auto utf8Name = gs.enterNameUTF8(mangledName);
    auto packagerName = gs.freshNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    return MangledName(gs.enterNameConstant(packagerName));
}

MangledName MangledName::mangledNameFromParts(core::GlobalState &gs, const std::vector<core::NameRef> &parts) {
    // Foo::Bar => Foo_Bar_Package
    auto mangledName = absl::StrCat(absl::StrJoin(parts, "_", NameFormatter(gs)), core::PACKAGE_SUFFIX);

    auto utf8Name = gs.enterNameUTF8(mangledName);
    auto packagerName = gs.freshNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    return MangledName(gs.enterNameConstant(packagerName));
}

MangledName MangledName::mangledNameFromHuman(const core::GlobalState &gs, string_view nameStr) {
    auto mangled = absl::StrCat(absl::StrReplaceAll(nameStr, {{"::", "_"}}), core::PACKAGE_SUFFIX);
    auto utf8Name = gs.lookupNameUTF8(mangled);
    if (!utf8Name.exists()) {
        return MangledName();
    }

    auto packagerName = gs.lookupNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    if (!packagerName.exists()) {
        return MangledName();
    }

    auto cnst = gs.lookupNameConstant(packagerName);
    if (!cnst.exists()) {
        return MangledName();
    }

    return MangledName(cnst);
}
} // namespace sorbet::core::packages
