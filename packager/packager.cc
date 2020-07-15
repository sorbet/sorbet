#include "packager/packager.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "common/FileOps.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/concurrency/WorkerPool.h"
#include "common/sort.h"
#include "core/Unfreeze.h"
#include "core/errors/packager.h"

using namespace std;

namespace sorbet::packager {
namespace {

constexpr string_view PACKAGE_FILE_NAME = "__package.rb"sv;

struct PackageName {
    core::LocOffsets loc;
    core::NameRef mangledName = core::NameRef::noName();
    vector<core::NameRef> fullName;
    ast::TreePtr getFullNameLiteral(core::LocOffsets loc) const;
};

struct PackageInfo {
    // The path prefix before every file in the package, including path separator at end.
    std::string packagePathPrefix;
    PackageName name;
    // loc for the package definition. Used for error messages.
    core::Loc loc;
    // The mangled names of each package imported by this package.
    vector<PackageName> importedPackageNames;
    // List of exported items that form the body of this package's public API.
    // These are copied into every package that imports this package.
    ast::ClassDef::RHS_store exportedItems;
};

class NameFormatter {
    const core::GlobalState &gs;

public:
    NameFormatter(const core::GlobalState &gs) : gs(gs) {}

    void operator()(std::string *out, core::NameRef name) const {
        out->append(name.data(gs)->shortName(gs));
    }
};

// Gets the package name in `name` if applicable.
PackageName getPackageName(core::MutableContext ctx, ast::TreePtr &name, bool errorOnInvalidName = false) {
    PackageName pName;
    pName.loc = name->loc;

    auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(name);
    while (constLit != nullptr) {
        if (errorOnInvalidName && absl::StrContains(constLit->cnst.data(ctx)->shortName(ctx), "_")) {
            // By forbidding package names to have an underscore, we can trivially convert between mangled names and
            // unmangled names by replacing `_` with `::`.
            if (auto e = ctx.beginError(constLit->loc, core::errors::Packager::InvalidPackageName)) {
                e.setHeader("Package names cannot contain an underscore");
                auto replacement = absl::StrReplaceAll(constLit->cnst.data(ctx)->shortName(ctx), {{"_", ""}});
                auto nameLoc = constLit->loc;
                // cnst is the last characters in the constant literal
                nameLoc.beginLoc = nameLoc.endLoc - constLit->cnst.data(ctx)->shortName(ctx).size();

                e.addAutocorrect(core::AutocorrectSuggestion{
                    fmt::format("Replace `{}` with `{}`", constLit->cnst.data(ctx)->shortName(ctx), replacement),
                    {core::AutocorrectSuggestion::Edit{core::Loc(ctx.file, nameLoc), replacement}}});
            }
        }
        pName.fullName.emplace_back(constLit->cnst);
        constLit = ast::cast_tree<ast::UnresolvedConstantLit>(constLit->scope);
    }
    reverse(pName.fullName.begin(), pName.fullName.end());
    if (!pName.fullName.empty()) {
        // Foo::Bar => Foo_Bar_Package
        auto mangledName = absl::StrCat(absl::StrJoin(pName.fullName, "_", NameFormatter(ctx)), "_Package");
        pName.mangledName = ctx.state.enterNameConstant(mangledName);
    } else {
        // Not a valid name.
        pName.mangledName = core::NameRef::noName();
    }
    return pName;
}

bool isReferenceToPackageSpec(core::Context ctx, ast::TreePtr &expr) {
    auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return constLit != nullptr && constLit->cnst == core::Names::Constants::PackageSpec();
}

unique_ptr<ast::UnresolvedConstantLit> copyConstantLit(ast::UnresolvedConstantLit *lit) {
    auto copy = lit->deepCopy();
    // Cast from Expression to UnresolvedConstantLit.
    auto copyUcl = ast::cast_tree<ast::UnresolvedConstantLit>(copy);
    ENFORCE(copyUcl != nullptr);
    unique_ptr<ast::UnresolvedConstantLit> rv(copyUcl);
    copy.release();
    return rv;
}

ast::TreePtr name2Expr(core::NameRef name, ast::TreePtr scope = ast::MK::EmptyTree()) {
    return ast::MK::UnresolvedConstant(core::LocOffsets::none(), move(scope), name);
}

ast::TreePtr PackageName::getFullNameLiteral(core::LocOffsets loc) const {
    ast::TreePtr name = ast::MK::EmptyTree();
    for (auto namePart : fullName) {
        name = name2Expr(namePart, move(name));
    }
    // Outer name should have the provided loc.
    if (auto lit = ast::cast_tree<ast::UnresolvedConstantLit>(name)) {
        name = ast::MK::UnresolvedConstant(loc, move(lit->scope), lit->cnst);
    }
    return name;
}

ast::UnresolvedConstantLit *verifyConstant(core::MutableContext ctx, core::NameRef fun, ast::TreePtr &expr) {
    auto target = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    if (target == nullptr) {
        if (auto e = ctx.beginError(expr->loc, core::errors::Packager::InvalidImportOrExport)) {
            e.setHeader("Argument to `{}` must be a constant", fun.show(ctx));
        }
    }
    return target;
}

core::LocOffsets firstLineOfFile(core::Context ctx) {
    auto file = ctx.file;
    core::LocOffsets firstLine{0, 0};
    // Line 0 is reserved.
    if (file.data(ctx).lineCount() > 1) {
        firstLine.endLoc = static_cast<u4>(file.data(ctx).lineBreaks()[1]);
    } else {
        firstLine.endLoc = static_cast<u4>(file.data(ctx).source().length());
    }
    return firstLine;
}

struct PackageInfoFinder {
    unique_ptr<PackageInfo> info = nullptr;
    vector<unique_ptr<ast::UnresolvedConstantLit>> exported;
    vector<unique_ptr<ast::UnresolvedConstantLit>> exportedMethods;
    core::LocOffsets exportMethodsLoc = core::LocOffsets::none();

    ast::TreePtr postTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);

        // Ignore methods
        if (send->fun == core::Names::keepDef() || send->fun == core::Names::keepSelfDef()) {
            return tree;
        }

        // Blacklisted methods
        if (send->fun == core::Names::extend() || send->fun == core::Names::include()) {
            if (auto e = ctx.beginError(send->loc, core::errors::Packager::InvalidPackageExpression)) {
                e.setHeader("Invalid expression in package: `{}` is not allowed", send->fun.data(ctx)->shortName(ctx));
            }
            return tree;
        }

        // Sanity check arguments for unrecognized methods
        if (send->fun != core::Names::export_() && send->fun != core::Names::import() &&
            send->fun != core::Names::exportMethods()) {
            for (const auto &arg : send->args) {
                if (!ast::isa_tree<ast::Literal>(arg)) {
                    if (auto e = ctx.beginError(arg->loc, core::errors::Packager::InvalidPackageExpression)) {
                        e.setHeader("Invalid expression in package: Arguments to functions must be literals");
                    }
                }
            }
        }

        if (info == nullptr) {
            // We haven't yet entered the package class.
            return tree;
        }

        if (send->fun == core::Names::export_() && send->args.size() == 1) {
            // null indicates an invalid export.
            if (auto target = verifyConstant(ctx, core::Names::export_(), send->args[0])) {
                exported.emplace_back(copyConstantLit(target));
                // Transform the constant lit to refer to the target within the mangled package namespace.
                send->args[0] = prependInternalPackageNameToScope(move(send->args[0]));
            }
        }

        if (send->fun == core::Names::import() && send->args.size() == 1) {
            // null indicates an invalid import.
            if (auto target = verifyConstant(ctx, core::Names::import(), send->args[0])) {
                auto name = getPackageName(ctx, send->args[0]);
                if (name.mangledName.exists()) {
                    if (name.mangledName == info->name.mangledName) {
                        if (auto e = ctx.beginError(target->loc, core::errors::Packager::NoSelfImport)) {
                            e.setHeader("Package `{}` cannot import itself",
                                        absl::StrJoin(info->name.fullName, "::", NameFormatter(ctx)));
                        }
                    }
                    info->importedPackageNames.emplace_back(move(name));
                }
            }
        }

        if (send->fun == core::Names::exportMethods()) {
            const bool alreadyCalled = exportMethodsLoc.exists();
            if (alreadyCalled) {
                if (auto e = ctx.beginError(send->loc, core::errors::Packager::MultipleExportMethodsCalls)) {
                    e.setHeader("export_methods can only be called once in a package");
                    e.addErrorLine(core::Loc(ctx.file, exportMethodsLoc), "Previous call to export_methods found here");
                }
            } else {
                exportMethodsLoc = send->loc;
            }
            for (auto &arg : send->args) {
                if (auto target = verifyConstant(ctx, core::Names::exportMethods(), arg)) {
                    exportedMethods.emplace_back(copyConstantLit(target));
                    // Transform the constant lit to refer to the target within the mangled package namespace.
                    arg = prependInternalPackageNameToScope(move(arg));
                }
            }
        }

        return tree;
    }

    ast::TreePtr preTransformClassDef(core::MutableContext ctx, ast::TreePtr tree) {
        auto *classDef = ast::cast_tree<ast::ClassDef>(tree);
        if (classDef->symbol == core::Symbols::root()) {
            // Ignore top-level <root>
            return tree;
        }

        if (classDef->ancestors.size() != 1 || !isReferenceToPackageSpec(ctx, classDef->ancestors[0])) {
            if (auto e = ctx.beginError(classDef->loc, core::errors::Packager::InvalidPackageDefinition)) {
                e.setHeader("Expected package definition of form `Foo::Bar < PackageSpec`");
            }
        } else if (info == nullptr) {
            info = make_unique<PackageInfo>();
            info->name = getPackageName(ctx, classDef->name, true);
            info->loc = core::Loc(ctx.file, classDef->loc);
        } else {
            if (auto e = ctx.beginError(classDef->loc, core::errors::Packager::MultiplePackagesInOneFile)) {
                e.setHeader("Package files can only declare one package");
                e.addErrorLine(info->loc, "Previous package declaration found here");
            }
        }

        return tree;
    }

    ast::TreePtr postTransformClassDef(core::MutableContext ctx, ast::TreePtr tree) {
        auto *classDef = ast::cast_tree<ast::ClassDef>(tree);
        if (classDef->symbol != core::Symbols::root() || info == nullptr || exportedMethods.empty()) {
            return tree;
        }

        // Inject <PackageRegistry>::Name_Package::<PackageMethods> within the <root> class
        ast::ClassDef::RHS_store includeStatements;
        for (auto &klass : exportedMethods) {
            // include <PackageRegistry>::MangledPackageName::Path::To::Item
            includeStatements.emplace_back(
                ast::MK::Send1(klass->loc, ast::MK::Self(klass->loc), core::Names::include(),
                               name2Expr(klass->cnst, prependInternalPackageNameToScope(klass->scope.deepCopy()))));
        }

        // Use the loc of the `export_methods` call so `include` errors goes to the right place.
        classDef->rhs.emplace_back(ast::MK::Module(
            exportMethodsLoc, core::Loc(ctx.file, exportMethodsLoc),
            name2Expr(core::Names::Constants::PackageMethods(),
                      name2Expr(this->info->name.mangledName, name2Expr(core::Names::Constants::PackageRegistry()))),
            {}, std::move(includeStatements)));

        return tree;
    }

    // Bar::Baz => <PackageRegistry>::Foo_Package::Bar::Baz
    ast::TreePtr prependInternalPackageNameToScope(ast::TreePtr scope) {
        ENFORCE(info != nullptr);
        // For `Bar::Baz::Bat`, `UnresolvedConstantLit` will contain `Bar`.
        ast::UnresolvedConstantLit *lastConstLit = ast::cast_tree<ast::UnresolvedConstantLit>(scope);
        if (lastConstLit != nullptr) {
            while (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(lastConstLit->scope)) {
                lastConstLit = constLit;
            }
        }

        // If `lastConstLit` is `nullptr`, then `scope` should be EmptyTree.
        ENFORCE(lastConstLit != nullptr || ast::cast_tree<ast::EmptyTree>(scope) != nullptr);

        auto scopeToPrepend =
            name2Expr(this->info->name.mangledName, name2Expr(core::Names::Constants::PackageRegistry()));
        if (lastConstLit == nullptr) {
            return scopeToPrepend;
        } else {
            lastConstLit->scope = move(scopeToPrepend);
            return scope;
        }
    }

    // Generates `exportModule`, which dependent packages copy to set up their namespaces.
    // For package Foo::Bar:
    //   module Foo::Bar
    //     ExportedItem1 = <PackageRegistry>::Foo_Bar_Package::Path::To::ExportedItem1
    //     extend <PackageRegistry>::Foo_Bar_Package::<PackageMethods>
    //   end
    void finalize(core::MutableContext ctx) {
        if (info == nullptr) {
            if (auto e = ctx.beginError(firstLineOfFile(ctx), core::errors::Packager::InvalidPackageDefinition)) {
                e.setHeader("Package file must contain a package definition of form `Foo::Bar < PackageSpec`");
            }
            return;
        }

        // NOTE: Don't assign locs to the nodes generated below. They will be copied into other __package.rbs that
        // import this package with __package.rb-specific locs.
        for (auto &klass : exported) {
            // Item = <PackageRegistry>::MangledPackageName::Path::To::Item
            info->exportedItems.emplace_back(
                ast::MK::Assign(core::LocOffsets::none(), name2Expr(klass->cnst),
                                name2Expr(klass->cnst, prependInternalPackageNameToScope(klass->scope.deepCopy()))));
        }

        if (!exportedMethods.empty()) {
            // extend <PackageRegistry>::Name_of_package::<PackageMethods>
            info->exportedItems.emplace_back(
                ast::MK::Send1(core::LocOffsets::none(), ast::MK::Self(core::LocOffsets::none()), core::Names::extend(),
                               name2Expr(core::Names::Constants::PackageMethods(),
                                         name2Expr(this->info->name.mangledName,
                                                   name2Expr(core::Names::Constants::PackageRegistry())))));
        }
    }

    /* Forbidden control flow in packages */

    void illegalControlFlow(core::MutableContext ctx, core::LocOffsets loc, string_view type) {
        if (auto e = ctx.beginError(loc, core::errors::Packager::InvalidPackageExpression)) {
            e.setHeader("Invalid expression in package: {} not allowed", type);
        }
    }

    ast::TreePtr preTransformIf(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "`if`");
        return original;
    }

    ast::TreePtr preTransformWhile(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "`while`");
        return original;
    }

    ast::TreePtr postTransformBreak(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "`break`");
        return original;
    }

    ast::TreePtr postTransformRetry(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "`retry`");
        return original;
    }

    ast::TreePtr postTransformNext(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "`next`");
        return original;
    }

    ast::TreePtr preTransformReturn(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "`return`");
        return original;
    }

    ast::TreePtr preTransformRescueCase(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "`rescue case`");
        return original;
    }

    ast::TreePtr preTransformRescue(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "`rescue`");
        return original;
    }

    ast::TreePtr preTransformAssign(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "`=`");
        return original;
    }

    ast::TreePtr preTransformHash(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "hash literals");
        return original;
    }

    ast::TreePtr preTransformArray(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "array literals");
        return original;
    }

    ast::TreePtr preTransformMethodDef(core::MutableContext ctx, ast::TreePtr original) {
        illegalControlFlow(ctx, original->loc, "method definitions");
        return original;
    }
};

// Sanity checks package files, mutates arguments to export / export_methods to point to item in namespace,
// builds up the expression injected into packages that import the package, and codegens the <PackagedMethods>  module.
unique_ptr<PackageInfo> getPackageInfo(core::MutableContext ctx, ast::ParsedFile &package) {
    ENFORCE(package.file.exists());
    ENFORCE(package.file.data(ctx).sourceType == core::File::Type::Package);
    // Assumption: Root of AST is <root> class.
    ENFORCE(ast::isa_tree<ast::ClassDef>(package.tree));
    ENFORCE(ast::cast_tree_nonnull<ast::ClassDef>(package.tree).symbol == core::Symbols::root());
    auto packageFilePath = package.file.data(ctx).path();
    ENFORCE(FileOps::getFileName(packageFilePath) == PACKAGE_FILE_NAME);
    PackageInfoFinder finder;
    package.tree = ast::TreeMap::apply(ctx, finder, move(package.tree));
    finder.finalize(ctx);
    if (finder.info) {
        finder.info->packagePathPrefix = packageFilePath.substr(0, packageFilePath.find_last_of('/') + 1);
    }
    return move(finder.info);
}

// Add:
//    module <PackageRegistry>::Mangled_Name_Package
//      module ImportedPackage1
//        # imported aliases go here
//      end
//    end
// ...to __package.rb files to set up the package namespace.
ast::ParsedFile rewritePackage(core::Context ctx, ast::ParsedFile file, const PackageInfo &package,
                               const UnorderedMap<core::NameRef, shared_ptr<PackageInfo>> &packageInfoByMangledName) {
    ast::ClassDef::RHS_store importedPackages;

    // Sanity check: Packages _must_ be typed: strict
    if (file.file.data(ctx).strictLevel != core::StrictLevel::Strict) {
        if (auto e = ctx.beginError(firstLineOfFile(ctx), core::errors::Packager::PackageFileMustBeStrict)) {
            e.setHeader("Package files must be typed: strict");
        }
    }

    {
        UnorderedMap<core::NameRef, core::LocOffsets> importedNames;
        for (auto imported : package.importedPackageNames) {
            auto it = packageInfoByMangledName.find(imported.mangledName);
            if (it == packageInfoByMangledName.end()) {
                if (auto e = ctx.beginError(imported.loc, core::errors::Packager::PackageNotFound)) {
                    e.setHeader("Cannot find package `{}`", absl::StrJoin(imported.fullName, "::", NameFormatter(ctx)));
                }
                continue;
            }

            if (importedNames.contains(imported.mangledName)) {
                if (auto e = ctx.beginError(imported.loc, core::errors::Packager::InvalidImportOrExport)) {
                    e.setHeader("Duplicate package import `{}`",
                                absl::StrJoin(imported.fullName, "::", NameFormatter(ctx)));
                    e.addErrorLine(core::Loc(ctx.file, importedNames[imported.mangledName]),
                                   "Previous package import found here");
                }
            } else {
                importedNames[imported.mangledName] = imported.loc;
                const auto &found = *it->second;

                ast::ClassDef::RHS_store exportedItemsCopy;
                for (const auto &exported : found.exportedItems) {
                    exportedItemsCopy.emplace_back(exported.deepCopy());
                }

                ENFORCE(!imported.fullName.empty());
                // Create a module for the imported package that sets up constant references to exported items.
                // Use proper loc information on the module name so that `import Foo` displays in the results of LSP
                // Find All References on `Foo`.
                importedPackages.emplace_back(ast::MK::Module(imported.loc, core::Loc(ctx.file, imported.loc),
                                                              imported.getFullNameLiteral(imported.loc), {},
                                                              std::move(exportedItemsCopy)));
            }
        }
    }

    // Include Kernel in the package.
    importedPackages.emplace_back(ast::MK::Send1(core::LocOffsets::none(), ast::MK::Self(core::LocOffsets::none()),
                                                 core::Names::include(), name2Expr(core::Names::Constants::Kernel())));

    auto packageNamespace =
        ast::MK::Module(core::LocOffsets::none(), core::Loc(ctx.file, core::LocOffsets::none()),
                        name2Expr(package.name.mangledName, name2Expr(core::Names::Constants::PackageRegistry())), {},
                        std::move(importedPackages));

    auto rootKlass = ast::cast_tree_nonnull<ast::ClassDef>(file.tree);
    rootKlass.rhs.emplace_back(move(packageNamespace));
    return file;
}

ast::ParsedFile rewritePackagedFile(core::Context ctx, ast::ParsedFile file, core::NameRef packageMangledName) {
    auto rootKlass = ast::cast_tree<ast::ClassDef>(file.tree);
    ENFORCE(rootKlass != nullptr);
    auto moduleWrapper =
        ast::MK::Module(core::LocOffsets::none(), core::Loc(ctx.file, core::LocOffsets::none()),
                        name2Expr(packageMangledName, name2Expr(core::Names::Constants::PackageRegistry())), {},
                        std::move(rootKlass->rhs));
    rootKlass->rhs.clear();
    rootKlass->rhs.emplace_back(move(moduleWrapper));
    return file;
}

// We can't run packages without having all package ASTs. Assert that they are all present.
bool checkContainsAllPackages(const core::GlobalState &gs, const vector<ast::ParsedFile> &files) {
    UnorderedSet<core::FileRef> allPackages;
    for (u4 i = 1; i < gs.filesUsed(); i++) {
        core::FileRef fref(i);
        if (fref.data(gs).sourceType == core::File::Type::Package) {
            allPackages.insert(fref);
        }
    }

    for (const auto &f : files) {
        if (f.file.data(gs).sourceType == core::File::Type::Package) {
            allPackages.erase(f.file);
        }
    }
    return allPackages.empty();
}

} // namespace

vector<ast::ParsedFile> Packager::run(core::GlobalState &gs, WorkerPool &workers, vector<ast::ParsedFile> files) {
    Timer timeit(gs.tracer(), "packager");
    // Ensure files are in canonical order.
    fast_sort(files, [](const auto &a, const auto &b) -> bool { return a.file < b.file; });

    // Step 1: Find packages and determine their imports/exports.
    UnorderedMap<core::FileRef, shared_ptr<PackageInfo>> packageInfoByFile;
    UnorderedMap<core::NameRef, shared_ptr<PackageInfo>> packageInfoByMangledName;
    vector<shared_ptr<PackageInfo>> packages;
    {
        Timer timeit(gs.tracer(), "packager.findPackages");
        core::UnfreezeNameTable unfreeze(gs);
        for (auto &file : files) {
            if (FileOps::getFileName(file.file.data(gs).path()) == PACKAGE_FILE_NAME) {
                file.file.data(gs).sourceType = core::File::Type::Package;
                core::MutableContext ctx(gs, core::Symbols::root(), file.file);
                // getPackageInfo emits an error if the package is malformed.
                if (auto pkg = getPackageInfo(ctx, file)) {
                    auto &existing = packageInfoByMangledName[pkg->name.mangledName];
                    if (existing != nullptr) {
                        if (auto e =
                                ctx.beginError(pkg->loc.offsets(), core::errors::Packager::RedefinitionOfPackage)) {
                            auto pkgName = absl::StrJoin(pkg->name.fullName, "::", NameFormatter(ctx));
                            e.setHeader("Redefinition of package `{}`", pkgName);
                            e.addErrorLine(existing->loc, "Package `{}` originally defined here", pkgName);
                        }
                    } else {
                        existing = move(pkg);
                    }
                    packageInfoByFile[file.file] = move(pkg);
                    packages.emplace_back(move(pkg));
                }
            }
        }
    }

    // Sort packages so that packages with the longest/most specific paths are first.
    // That way, the first package to match a file's path is the most specific package match.
    fast_sort(packages, [](const auto &a, const auto &b) -> bool {
        return a->packagePathPrefix.size() > b->packagePathPrefix.size();
    });

    {
        Timer timeit(gs.tracer(), "packager.rewritePackages");
        // Step 2: Rewrite packages. Can be done in parallel (and w/ step 3) if this becomes a bottleneck.
        for (auto &file : files) {
            if (file.file.data(gs).sourceType == core::File::Type::Package) {
                if (auto &packageInfo = packageInfoByFile[file.file]) {
                    core::Context ctx(gs, core::Symbols::root(), file.file);
                    file = rewritePackage(ctx, move(file), *packageInfo, packageInfoByMangledName);
                }
            }
        }
    }

    // Step 3: Find files within each package and rewrite each.
    {
        Timer timeit(gs.tracer(), "packager.rewritePackagedFiles");

        auto resultq = make_shared<BlockingBoundedQueue<vector<ast::ParsedFile>>>(files.size());
        auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(files.size());
        for (auto &file : files) {
            fileq->push(move(file), 1);
        }

        workers.multiplexJob("rewritePackagedFiles", [&gs, &packages, fileq, resultq]() {
            Timer timeit(gs.tracer(), "packager.rewritePackagedFilesWorker");
            vector<ast::ParsedFile> results;
            u4 filesProcessed = 0;
            ast::ParsedFile job;
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    filesProcessed++;
                    if (job.file.data(gs).sourceType == core::File::Type::Normal) {
                        // Check if file path is part of a package.
                        // TODO(jvilk): Could use a prefix array to make this lookup more efficient.
                        auto path = job.file.data(gs).path();
                        bool packaged = false;
                        core::Context ctx(gs, core::Symbols::root(), job.file);
                        for (const auto &pkg : packages) {
                            if (absl::StartsWith(path, pkg->packagePathPrefix)) {
                                job = rewritePackagedFile(ctx, move(job), pkg->name.mangledName);
                                packaged = true;
                                break;
                            }
                        }
                        if (!packaged) {
                            // Don't transform, but raise an error.
                            if (auto e = ctx.beginError(job.tree->loc, core::errors::Packager::UnpackagedFile)) {
                                e.setHeader("File `{}` does not belong to a package; add a `__package.rb` file to one "
                                            "of its parent directories",
                                            ctx.file.data(gs).path());
                            }
                        }
                    }
                    results.emplace_back(move(job));
                }
            }
            if (filesProcessed > 0) {
                resultq->push(move(results), filesProcessed);
            }
        });
        files.clear();

        {
            vector<ast::ParsedFile> threadResult;
            for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
                 !result.done();
                 result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
                if (result.gotItem()) {
                    files.insert(files.end(), make_move_iterator(threadResult.begin()),
                                 make_move_iterator(threadResult.end()));
                }
            }
        }
    }

    fast_sort(files, [](const auto &a, const auto &b) -> bool { return a.file < b.file; });

    return files;
}

vector<ast::ParsedFile> Packager::runIncremental(core::GlobalState &gs, vector<ast::ParsedFile> files) {
    // Just run all packages w/ the changed files through Packager again. It should not define any new names.
    // TODO(jvilk): This incremental pass retypechecks every package file in the project. It should instead only process
    // the packages needed to understand file changes.
    ENFORCE(checkContainsAllPackages(gs, files));
    auto namesUsed = gs.namesUsed();
    auto emptyWorkers = WorkerPool::create(0, gs.tracer());
    files = Packager::run(gs, *emptyWorkers, move(files));
    ENFORCE(gs.namesUsed() == namesUsed);
    return files;
}

} // namespace sorbet::packager
