#include "core/packages/MangledName.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core::packages {
MangledName MangledName::mangledNameFromParts(core::GlobalState &gs, const std::vector<std::string_view> &parts) {
    // Foo::Bar => Foo_Bar
    auto mangledName = absl::StrCat(absl::StrJoin(parts, "_"));

    auto utf8Name = gs.enterNameUTF8(mangledName);
    auto packagerName = gs.freshNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    return MangledName(gs.enterNameConstant(packagerName));
}

MangledName MangledName::mangledNameFromParts(core::GlobalState &gs, const std::vector<core::NameRef> &parts) {
    // Foo::Bar => Foo_Bar
    auto mangledName = absl::StrCat(absl::StrJoin(parts, "_", NameFormatter(gs)));

    auto utf8Name = gs.enterNameUTF8(mangledName);
    auto packagerName = gs.freshNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    return MangledName(gs.enterNameConstant(packagerName));
}

} // namespace sorbet::core::packages
