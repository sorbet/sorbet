#include "rewriter/PackageSpec.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "ast/packager/packager.h"
#include "core/core.h"
#include "core/errors/packager.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

void mustContainPackageDef(core::MutableContext ctx, core::LocOffsets loc) {
    // HACKFIX: Tolerate completely empty packages. LSP does not support the notion of a deleted file, and
    // instead replaces deleted files with the empty string. It should really mark files as Tombstones instead.
    if (!ctx.file.data(ctx).source().empty()) {
        if (auto e = ctx.beginIndexerError(loc, core::errors::Packager::InvalidPackageDefinition)) {
            e.setHeader("`{}` file must contain a package definition", "__package.rb");
            e.addErrorNote("Package definitions are class definitions like `{}`.\n"
                           "    For more information, see http://go/package-layout",
                           "class Foo::Bar < PackageSpec");
        }
    }
}

[[nodiscard]] bool validatePackageName(core::MutableContext ctx, const ast::UnresolvedConstantLit *constLit) {
    bool valid = true;
    while (constLit != nullptr) {
        if (absl::StrContains(constLit->cnst.shortName(ctx), "_")) {
            // By forbidding package names to have an underscore, we can trivially convert between
            // mangled names and unmangled names by replacing `_` with `::`.
            //
            // Even with packages into the symbol table this restriction is useful, because we have
            // a lot of tooling that will create directory structures like Foo_Bar to store
            // generated files associated with package Foo::Bar
            if (auto e = ctx.beginIndexerError(constLit->loc, core::errors::Packager::InvalidPackageName)) {
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
    // Aware of whether the --sorbet-packages option was passed
    if (!ctx.file.data(ctx).isPackage(ctx)) {
        return;
    }

    if (klass->symbol != core::Symbols::root()) {
        // Only process ClassDef that are at the top level
        return;
    }

    bool reportedError = false;
    for (auto &rootStmt : klass->rhs) {
        auto packageSpecClass = ast::cast_tree<ast::ClassDef>(rootStmt);
        if (packageSpecClass == nullptr) {
            // No error here; let this be reported in the tree walk later as a bad node type,
            // or at the end of this function if no PackageSpec is found.
            continue;
        }

        if (packageSpecClass->ancestors.size() != 1 ||
            !ast::isa_tree<ast::UnresolvedConstantLit>(packageSpecClass->name)) {
            mustContainPackageDef(ctx, packageSpecClass->declLoc);
            reportedError = true;
            continue;
        }

        auto &superClass = packageSpecClass->ancestors[0];
        auto superClassLit = ast::cast_tree<ast::UnresolvedConstantLit>(superClass);
        if (superClassLit == nullptr || superClassLit->cnst != core::Names::Constants::PackageSpec()) {
            mustContainPackageDef(ctx, superClass.loc());
            reportedError = true;
            continue;
        }

        // ---- Mutates the tree ----

        auto nameTree = ast::cast_tree<ast::UnresolvedConstantLit>(packageSpecClass->name);
        if (!validatePackageName(ctx, nameTree)) {
            reportedError = true;

            // "Remove" the superclass.
            //
            // `::PackageSpec` doesn't exist. Normally we would rewrite it
            // to Sorbet::Private::Static::PackageSpec, but we don't do that if the package spec
            // definition is invalid.
            //
            // To avoid the chance that the user only sees the "Unable to resolve PackageSpec" error
            // and then gets confused, we "remove" the super class here, treating it like the user
            // omitted the superclass.
            //
            // This establishes an invariant that if the superClass is a resolved constant and
            // equal to Symbols::PackageSpec(), then it's the canonical package def in this file
            superClass = ast::MK::Constant(packageSpecClass->loc, core::Symbols::todo());

            continue;
        }

        // Pre-resolve the super class. This makes it easier to detect that this is a package
        // spec-related class def in later passes without having to recursively walk up the constant
        // lit's scope to find if it starts with <PackageSpecRegistry>.
        superClass = ast::make_expression<ast::ConstantLit>(core::Symbols::PackageSpec(),
                                                            superClass.toUnique<ast::UnresolvedConstantLit>());

        // `class Foo < PackageSpec` -> `class <PackageSpecRegistry>::Foo < PackageSpec`
        // This removes the PackageSpec's themselves from the top-level namespace
        packageSpecClass->name = ast::packager::prependRegistry(move(packageSpecClass->name));

        // Return eagerly so we don't report duplicate errors on subsequent statements:
        // we'll let those errors be reported in the tree walk later as a bad node type.
        return;
    }

    // Only report an error if we didn't already
    // (the one we reported will have been more descriptive than this one)
    if (!reportedError) {
        auto errLoc = klass->rhs.empty() ? core::LocOffsets{0, 0} : klass->rhs[0].loc();
        mustContainPackageDef(ctx, errLoc);
    }
}

} // namespace sorbet::rewriter
