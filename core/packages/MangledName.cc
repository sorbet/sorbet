#include "core/packages/MangledName.h"
#include "absl/strings/str_join.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core::packages {
MangledName MangledName::mangledNameFromParts(GlobalState &gs, const std::vector<std::string_view> &parts,
                                              ClassOrModuleRef owner) {
    // Foo::Bar => Foo_Bar
    auto mangledName = absl::StrCat(absl::StrJoin(parts, "_"));

    auto utf8Name = gs.enterNameUTF8(mangledName);
    auto packagerName = gs.freshNameUnique(UniqueNameKind::Packager, utf8Name, 1);
    return MangledName(gs.enterNameConstant(packagerName), owner);
}

MangledName MangledName::mangledNameFromParts(GlobalState &gs, const std::vector<NameRef> &parts,
                                              ClassOrModuleRef owner) {
    // Foo::Bar => Foo_Bar
    auto mangledName = absl::StrCat(absl::StrJoin(parts, "_", NameFormatter(gs)));

    auto utf8Name = gs.enterNameUTF8(mangledName);
    auto packagerName = gs.freshNameUnique(UniqueNameKind::Packager, utf8Name, 1);
    return MangledName(gs.enterNameConstant(packagerName), owner);
}

} // namespace sorbet::core::packages
