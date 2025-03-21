#include "rewriter/PackageSpec.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "core/core.h"
#include "core/errors/packager.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

void mustContainPackageDef(core::Context ctx, core::LocOffsets loc) {
    // HACKFIX: Tolerate completely empty packages. LSP does not support the notion of a deleted file, and
    // instead replaces deleted files with the empty string. It should really mark files as Tombstones instead.
    if (!ctx.file.data(ctx).source().empty()) {
        if (auto e = ctx.beginError(loc, core::errors::Packager::InvalidPackageDefinition)) {
            e.setHeader("`{}` file must contain a package definition", "__package.rb");
            e.addErrorNote("Package definitions are class definitions like `{}`.\n"
                           "    For more information, see http://go/package-layout",
                           "class Foo::Bar < PackageSpec");
        }
    }
}

[[nodiscard]] bool validatePackageName(core::Context ctx, const ast::UnresolvedConstantLit *constLit) {
    bool valid = true;
    while (constLit != nullptr) {
        if (absl::StrContains(constLit->cnst.shortName(ctx), "_")) {
            // By forbidding package names to have an underscore, we can trivially convert between
            // mangled names and unmangled names by replacing `_` with `::`.
            //
            // Even with packages into the symbol table this restriction is useful, because we have
            // a lot of tooling that will create directory structures like Foo_Bar to store
            // generated files associated with package Foo::Bar
            if (auto e = ctx.beginError(constLit->loc, core::errors::Packager::InvalidPackageName)) {
                e.setHeader("Package names cannot contain an underscore");
                auto replacement = absl::StrReplaceAll(constLit->cnst.shortName(ctx), {{"_", ""}});
                auto nameLoc = constLit->loc;
                // cnst is the last characters in the constant literal
                nameLoc.beginLoc = nameLoc.endLoc - constLit->cnst.shortName(ctx).size();

                e.addAutocorrect(core::AutocorrectSuggestion{
                    fmt::format("Replace `{}` with `{}`", constLit->cnst.shortName(ctx), replacement),
                    {core::AutocorrectSuggestion::Edit{ctx.locAt(nameLoc), replacement}}});
            }
            valid = false;
        }
        constLit = ast::cast_tree<ast::UnresolvedConstantLit>(constLit->scope);
    }

    return valid;
}

} // namespace

void PackageSpec::run(core::MutableContext ctx, ast::ClassDef *klass) {
    ENFORCE(ctx.state.packageDB().enabled(), "Should only run on __package.rb files");
    ENFORCE(ctx.file.data(ctx).isPackage(ctx), "Should only run on __package.rb files");

    if (ctx.owner != core::Symbols::root()) {
        // Only process ClassDef that are at the top level
        ENFORCE(ctx.owner == core::Symbols::todo());
        return;
    }

    return;
}

} // namespace sorbet::rewriter
