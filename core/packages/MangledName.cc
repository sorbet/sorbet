#include "core/packages/MangledName.h"
#include "absl/strings/str_join.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::core::packages {
core::NameRef MangledName::mangledNameFromParts(core::MutableContext ctx, std::vector<std::string> &parts) {
    // Foo::Bar => Foo_Bar_Package
    auto mangledName = absl::StrCat(absl::StrJoin(parts, "_"), core::PACKAGE_SUFFIX);

    auto utf8Name = ctx.state.enterNameUTF8(mangledName);
    auto packagerName = ctx.state.freshNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    return ctx.state.enterNameConstant(packagerName);
}

core::NameRef MangledName::mangledNameFromParts(core::MutableContext ctx, std::vector<core::NameRef> &parts) {
    // Foo::Bar => Foo_Bar_Package
    auto mangledName = absl::StrCat(absl::StrJoin(parts, "_", NameFormatter(ctx)), core::PACKAGE_SUFFIX);

    auto utf8Name = ctx.state.enterNameUTF8(mangledName);
    auto packagerName = ctx.state.freshNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    return ctx.state.enterNameConstant(packagerName);
}
} // namespace sorbet::core::packages
