#include "core/packages/MangledName.h"
#include "absl/strings/str_join.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core::packages {
core::NameRef MangledName::mangledNameFromParts(core::GlobalState &gs, std::vector<std::string_view> &parts) {
    // Foo::Bar => Foo_Bar_Package
    auto mangledName = absl::StrCat(absl::StrJoin(parts, "_"), core::PACKAGE_SUFFIX);

    auto utf8Name = gs.enterNameUTF8(mangledName);
    auto packagerName = gs.freshNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    return gs.enterNameConstant(packagerName);
}

core::NameRef MangledName::mangledNameFromParts(core::GlobalState &gs, std::vector<core::NameRef> &parts) {
    // Foo::Bar => Foo_Bar_Package
    auto mangledName = absl::StrCat(absl::StrJoin(parts, "_", NameFormatter(gs)), core::PACKAGE_SUFFIX);

    auto utf8Name = gs.enterNameUTF8(mangledName);
    auto packagerName = gs.freshNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    return gs.enterNameConstant(packagerName);
}
} // namespace sorbet::core::packages
