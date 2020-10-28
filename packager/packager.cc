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

struct FullyQualifiedName {
    vector<core::NameRef> parts;
    core::Loc loc;
    ast::TreePtr toLiteral(core::LocOffsets loc) const;
};

class NameFormatter final {
    const core::GlobalState &gs;

public:
    NameFormatter(const core::GlobalState &gs) : gs(gs) {}

    void operator()(std::string *out, core::NameRef name) const {
        out->append(name.data(gs)->shortName(gs));
    }
};

struct PackageName {
    core::LocOffsets loc;
    core::NameRef mangledName = core::NameRef::noName();
    FullyQualifiedName fullName;

    // Pretty print the package's (user-observable) name (e.g. Foo::Bar)
    string toString(const core::GlobalState &gs) const {
        return absl::StrJoin(fullName.parts, "::", NameFormatter(gs));
    }
};

struct PackageInfo {
    // The path prefix before every file in the package, including path separator at end.
    std::string packagePathPrefix;
    PackageName name;
    // loc for the package definition. Used for error messages.
    core::Loc loc;
    // The names of each package imported by this package.
    vector<PackageName> importedPackageNames;
    // List of exported items that form the body of this package's public API.
    // These are copied into every package that imports this package.
    ast::ClassDef::RHS_store exportedItems;
};

/**
 * Container class that facilitates thread-safe read-only access to packages.
 */
class PackageDB final {
private:
    // The only thread that is allowed write access to this class.
    const std::thread::id owner;
    vector<shared_ptr<const PackageInfo>> packages;
    bool finalized = false;
    UnorderedMap<core::FileRef, shared_ptr<const PackageInfo>> packageInfoByFile;
    UnorderedMap<core::NameRef, shared_ptr<const PackageInfo>> packageInfoByMangledName;

public:
    PackageDB() : owner(this_thread::get_id()) {}

    void addPackage(core::Context ctx, shared_ptr<PackageInfo> pkg) {
        ENFORCE(owner == this_thread::get_id());
        if (finalized) {
            Exception::raise("Cannot add additional packages after finalizing PackageDB");
        }
        if (pkg == nullptr) {
            // There was an error creating a PackageInfo for this file, and getPackageInfo has already surfaced that
            // error to the user. Nothing to do here.
            return;
        }
        auto it = packageInfoByMangledName.find(pkg->name.mangledName);
        if (it != packageInfoByMangledName.end()) {
            if (auto e = ctx.beginError(pkg->loc.offsets(), core::errors::Packager::RedefinitionOfPackage)) {
                auto pkgName = pkg->name.toString(ctx);
                e.setHeader("Redefinition of package `{}`", pkgName);
                e.addErrorLine(it->second->loc, "Package `{}` originally defined here", pkgName);
            }
        } else {
            packageInfoByMangledName[pkg->name.mangledName] = pkg;
        }

        packageInfoByFile[ctx.file] = pkg;
        packages.emplace_back(pkg);
    }

    void finalizePackages() {
        ENFORCE(owner == this_thread::get_id());
        // Sort packages so that packages with the longest/most specific paths are first.
        // That way, the first package to match a file's path is the most specific package match.
        fast_sort(packages, [](const auto &a, const auto &b) -> bool {
            return a->packagePathPrefix.size() > b->packagePathPrefix.size();
        });
        finalized = true;
    }

    /**
     * Given a file of type PACKAGE, return its PackageInfo or nullptr if one does not exist.
     */
    const PackageInfo *getPackageByFile(core::FileRef packageFile) const {
        const auto &it = packageInfoByFile.find(packageFile);
        if (it == packageInfoByFile.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    /**
     * Given the mangled name for a package (e.g., Foo::Bar's mangled name is Foo_Bar_Package), return that package's
     * info or nullptr if it does not exist.
     */
    const PackageInfo *getPackageByMangledName(core::NameRef name) const {
        const auto &it = packageInfoByMangledName.find(name);
        if (it == packageInfoByMangledName.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    /**
     * Given a context, return the active package or nullptr if one does not exist.
     */
    const PackageInfo *getPackageForContext(core::Context ctx) const {
        if (!finalized) {
            Exception::raise("Cannot map files to packages until all packages are added and PackageDB is finalized");
        }
        // TODO(jvilk): Could use a prefix array to make this lookup more efficient.
        auto path = ctx.file.data(ctx).path();
        for (const auto &pkg : packages) {
            if (absl::StartsWith(path, pkg->packagePathPrefix)) {
                return pkg.get();
            }
        }
        return nullptr;
    }
};

void checkPackageName(core::Context ctx, ast::UnresolvedConstantLit *constLit) {
    while (constLit != nullptr) {
        if (absl::StrContains(constLit->cnst.data(ctx)->shortName(ctx), "_")) {
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
        constLit = ast::cast_tree<ast::UnresolvedConstantLit>(constLit->scope);
    }
}

FullyQualifiedName getFullyQualifiedName(core::Context ctx, ast::UnresolvedConstantLit *constantLit) {
    FullyQualifiedName fqn;
    fqn.loc = core::Loc(ctx.file, constantLit->loc);
    while (constantLit != nullptr) {
        fqn.parts.emplace_back(constantLit->cnst);
        constantLit = ast::cast_tree<ast::UnresolvedConstantLit>(constantLit->scope);
    }
    reverse(fqn.parts.begin(), fqn.parts.end());
    ENFORCE(!fqn.parts.empty());
    return fqn;
}

// Gets the package name in `tree` if applicable.
PackageName getPackageName(core::MutableContext ctx, ast::UnresolvedConstantLit *constantLit) {
    ENFORCE(constantLit != nullptr);

    PackageName pName;
    pName.loc = constantLit->loc;
    pName.fullName = getFullyQualifiedName(ctx, constantLit);

    // Foo::Bar => Foo_Bar_Package
    auto mangledName = absl::StrCat(absl::StrJoin(pName.fullName.parts, "_", NameFormatter(ctx)), "_Package");
    pName.mangledName = ctx.state.enterNameConstant(mangledName);

    return pName;
}

bool isReferenceToPackageSpec(core::Context ctx, ast::TreePtr &expr) {
    auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return constLit != nullptr && constLit->cnst == core::Names::Constants::PackageSpec();
}

ast::TreePtr name2Expr(core::NameRef name, ast::TreePtr scope = ast::MK::EmptyTree()) {
    return ast::MK::UnresolvedConstant(core::LocOffsets::none(), move(scope), name);
}

ast::TreePtr FullyQualifiedName::toLiteral(core::LocOffsets loc) const {
    ast::TreePtr name = ast::MK::EmptyTree();
    for (auto part : parts) {
        name = name2Expr(part, move(name));
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
        if (auto e = ctx.beginError(expr.loc(), core::errors::Packager::InvalidImportOrExport)) {
            e.setHeader("Argument to `{}` must be a constant", fun.show(ctx));
        }
    }
    return target;
}

struct PackageInfoFinder {
    unique_ptr<PackageInfo> info = nullptr;
    vector<FullyQualifiedName> exported;
    vector<FullyQualifiedName> exportedMethods;
    core::LocOffsets exportMethodsLoc = core::LocOffsets::none();

    ast::TreePtr postTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);

        // Ignore methods
        if (send.fun == core::Names::keepDef() || send.fun == core::Names::keepSelfDef()) {
            return tree;
        }

        // Blacklisted methods
        if (send.fun == core::Names::extend() || send.fun == core::Names::include()) {
            if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidPackageExpression)) {
                e.setHeader("Invalid expression in package: `{}` is not allowed", send.fun.data(ctx)->shortName(ctx));
            }
            return tree;
        }

        // Sanity check arguments for unrecognized methods
        if (send.fun != core::Names::export_() && send.fun != core::Names::import() &&
            send.fun != core::Names::exportMethods()) {
            for (const auto &arg : send.args) {
                if (!ast::isa_tree<ast::Literal>(arg)) {
                    if (auto e = ctx.beginError(arg.loc(), core::errors::Packager::InvalidPackageExpression)) {
                        e.setHeader("Invalid expression in package: Arguments to functions must be literals");
                    }
                }
            }
        }

        if (info == nullptr) {
            // We haven't yet entered the package class.
            return tree;
        }

        if (send.fun == core::Names::export_() && send.args.size() == 1) {
            // null indicates an invalid export.
            if (auto target = verifyConstant(ctx, core::Names::export_(), send.args[0])) {
                exported.push_back(getFullyQualifiedName(ctx, target));
                // Transform the constant lit to refer to the target within the mangled package namespace.
                send.args[0] = prependInternalPackageName(move(send.args[0]));
            }
        }

        if (send.fun == core::Names::import() && send.args.size() == 1) {
            // null indicates an invalid import.
            if (auto target = verifyConstant(ctx, core::Names::import(), send.args[0])) {
                auto name = getPackageName(ctx, target);
                if (name.mangledName == info->name.mangledName) {
                    if (auto e = ctx.beginError(target->loc, core::errors::Packager::NoSelfImport)) {
                        e.setHeader("Package `{}` cannot import itself", info->name.toString(ctx));
                    }
                }
                info->importedPackageNames.emplace_back(move(name));
            }
        }

        if (send.fun == core::Names::exportMethods()) {
            const bool alreadyCalled = exportMethodsLoc.exists();
            if (alreadyCalled) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::MultipleExportMethodsCalls)) {
                    e.setHeader("`{}` can only be called once in a package", "export_methods");
                    e.addErrorLine(core::Loc(ctx.file, exportMethodsLoc), "Previous call to export_methods found here");
                }
            } else {
                exportMethodsLoc = send.loc;
            }
            for (auto &arg : send.args) {
                if (auto target = verifyConstant(ctx, core::Names::exportMethods(), arg)) {
                    // Don't export the methods if export_methods was already called. Let dependents error until it is
                    // fixed.
                    if (!alreadyCalled) {
                        exportedMethods.push_back(getFullyQualifiedName(ctx, target));
                    }
                    // Transform the constant lit to refer to the target within the mangled package namespace.
                    arg = prependInternalPackageName(move(arg));
                }
            }
        }

        return tree;
    }

    ast::TreePtr preTransformClassDef(core::MutableContext ctx, ast::TreePtr tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root()) {
            // Ignore top-level <root>
            return tree;
        }

        if (classDef.ancestors.size() != 1 || !isReferenceToPackageSpec(ctx, classDef.ancestors[0]) ||
            !ast::isa_tree<ast::UnresolvedConstantLit>(classDef.name)) {
            if (auto e = ctx.beginError(classDef.loc, core::errors::Packager::InvalidPackageDefinition)) {
                e.setHeader("Expected package definition of form `Foo::Bar < PackageSpec`");
            }
        } else if (info == nullptr) {
            auto nameTree = ast::cast_tree<ast::UnresolvedConstantLit>(classDef.name);
            info = make_unique<PackageInfo>();
            checkPackageName(ctx, nameTree);
            info->name = getPackageName(ctx, nameTree);
            info->loc = core::Loc(ctx.file, classDef.loc);
        } else {
            if (auto e = ctx.beginError(classDef.loc, core::errors::Packager::MultiplePackagesInOneFile)) {
                e.setHeader("Package files can only declare one package");
                e.addErrorLine(info->loc, "Previous package declaration found here");
            }
        }

        return tree;
    }

    ast::TreePtr postTransformClassDef(core::MutableContext ctx, ast::TreePtr tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol != core::Symbols::root() || info == nullptr || exportedMethods.empty()) {
            return tree;
        }

        // Inject <PackageRegistry>::Name_Package::<PackageMethods> within the <root> class
        ast::ClassDef::RHS_store includeStatements;
        for (auto &exportedMethod : exportedMethods) {
            // include <PackageRegistry>::MangledPackageName::Path::To::Item
            // Use the loc of the name so the error goes to the right place.
            ENFORCE(exportedMethod.loc.file() == ctx.file);
            includeStatements.emplace_back(ast::MK::Send1(
                exportedMethod.loc.offsets(), ast::MK::Self(exportedMethod.loc.offsets()), core::Names::include(),
                prependInternalPackageName(exportedMethod.toLiteral(exportedMethod.loc.offsets()))));
        }

        // Use the loc of the `export_methods` call so `include` errors goes to the right place.
        classDef.rhs.emplace_back(ast::MK::Module(
            exportMethodsLoc, exportMethodsLoc,
            name2Expr(core::Names::Constants::PackageMethods(),
                      name2Expr(this->info->name.mangledName, name2Expr(core::Names::Constants::PackageRegistry()))),
            {}, std::move(includeStatements)));

        return tree;
    }

    // Bar::Baz => <PackageRegistry>::Foo_Package::Bar::Baz
    ast::TreePtr prependInternalPackageName(ast::TreePtr scope) {
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
            if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::InvalidPackageDefinition)) {
                e.setHeader("Package file must contain a package definition of form `Foo::Bar < PackageSpec`");
            }
            return;
        }

        // NOTE: Don't assign locs to the nodes generated below. They will be copied into other __package.rbs that
        // import this package with __package.rb-specific locs.
        for (auto &klass : exported) {
            // Item = <PackageRegistry>::MangledPackageName::Path::To::Item
            ENFORCE(!klass.parts.empty());
            info->exportedItems.emplace_back(
                ast::MK::Assign(core::LocOffsets::none(), name2Expr(klass.parts.back()),
                                prependInternalPackageName(klass.toLiteral(core::LocOffsets::none()))));
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

    /* Forbid arbitrary computation in packages */

    void illegalNode(core::MutableContext ctx, core::LocOffsets loc, string_view type) {
        if (auto e = ctx.beginError(loc, core::errors::Packager::InvalidPackageExpression)) {
            e.setHeader("Invalid expression in package: {} not allowed", type);
        }
    }

    ast::TreePtr preTransformIf(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "`if`");
        return original;
    }

    ast::TreePtr preTransformWhile(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "`while`");
        return original;
    }

    ast::TreePtr postTransformBreak(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "`break`");
        return original;
    }

    ast::TreePtr postTransformRetry(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "`retry`");
        return original;
    }

    ast::TreePtr postTransformNext(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "`next`");
        return original;
    }

    ast::TreePtr preTransformReturn(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "`return`");
        return original;
    }

    ast::TreePtr preTransformRescueCase(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "`rescue case`");
        return original;
    }

    ast::TreePtr preTransformRescue(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "`rescue`");
        return original;
    }

    ast::TreePtr preTransformAssign(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "`=`");
        return original;
    }

    ast::TreePtr preTransformHash(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "hash literals");
        return original;
    }

    ast::TreePtr preTransformArray(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "array literals");
        return original;
    }

    ast::TreePtr preTransformMethodDef(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "method definitions");
        return original;
    }

    ast::TreePtr preTransformBlock(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "blocks");
        return original;
    }

    ast::TreePtr preTransformInsSeq(core::MutableContext ctx, ast::TreePtr original) {
        illegalNode(ctx, original.loc(), "`begin` and `end`");
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
ast::ParsedFile rewritePackage(core::Context ctx, ast::ParsedFile file, const PackageDB &packageDB) {
    ast::ClassDef::RHS_store importedPackages;

    auto package = packageDB.getPackageByFile(file.file);
    if (package == nullptr) {
        // We already produced an error on this package when producing its package info.
        // The correct course of action is to abort the transform.
        return file;
    }

    // Sanity check: __package.rb files _must_ be typed: strict
    // TODO: figure out why package files are being turned `false` under autogen?
    // if (file.file.data(ctx).strictLevel < core::StrictLevel::Strict) {
    //     if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::PackageFileMustBeStrict)) {
    //         e.setHeader("Package files must be at least `{}`", "# typed: strict");
    //     }
    // }

    {
        UnorderedMap<core::NameRef, core::LocOffsets> importedNames;
        for (auto imported : package->importedPackageNames) {
            auto importedPackage = packageDB.getPackageByMangledName(imported.mangledName);
            if (importedPackage == nullptr) {
                if (auto e = ctx.beginError(imported.loc, core::errors::Packager::PackageNotFound)) {
                    e.setHeader("Cannot find package `{}`", imported.toString(ctx));
                }
                continue;
            }

            if (importedNames.contains(imported.mangledName)) {
                if (auto e = ctx.beginError(imported.loc, core::errors::Packager::InvalidImportOrExport)) {
                    e.setHeader("Duplicate package import `{}`", imported.toString(ctx));
                    e.addErrorLine(core::Loc(ctx.file, importedNames[imported.mangledName]),
                                   "Previous package import found here");
                }
            } else {
                importedNames[imported.mangledName] = imported.loc;

                ast::ClassDef::RHS_store exportedItemsCopy;
                for (const auto &exported : importedPackage->exportedItems) {
                    exportedItemsCopy.emplace_back(exported.deepCopy());
                }

                ENFORCE(!imported.fullName.parts.empty());
                // Create a module for the imported package that sets up constant references to exported items.
                // Use proper loc information on the module name so that `import Foo` displays in the results of LSP
                // Find All References on `Foo`.
                importedPackages.emplace_back(ast::MK::Module(imported.loc, imported.loc,
                                                              imported.fullName.toLiteral(imported.loc), {},
                                                              std::move(exportedItemsCopy)));
            }
        }
    }

    auto packageNamespace =
        ast::MK::Module(core::LocOffsets::none(), core::LocOffsets::none(),
                        name2Expr(package->name.mangledName, name2Expr(core::Names::Constants::PackageRegistry())), {},
                        std::move(importedPackages));

    auto &rootKlass = ast::cast_tree_nonnull<ast::ClassDef>(file.tree);
    rootKlass.rhs.emplace_back(move(packageNamespace));
    return file;
}

ast::ParsedFile rewritePackagedFile(core::Context ctx, ast::ParsedFile file, core::NameRef packageMangledName) {
    if (ast::isa_tree<ast::EmptyTree>(file.tree)) {
        // Nothing to wrap. This occurs when a file is marked typed: Ignore.
        return file;
    }

    auto &rootKlass = ast::cast_tree_nonnull<ast::ClassDef>(file.tree);
    auto moduleWrapper =
        ast::MK::Module(core::LocOffsets::none(), core::LocOffsets::none(),
                        name2Expr(packageMangledName, name2Expr(core::Names::Constants::PackageRegistry())), {},
                        std::move(rootKlass.rhs));
    rootKlass.rhs.clear();
    rootKlass.rhs.emplace_back(move(moduleWrapper));
    return file;
}

// We can't run packages without having all package ASTs. Assert that they are all present.
bool checkContainsAllPackages(const core::GlobalState &gs, const vector<ast::ParsedFile> &files) {
    UnorderedSet<core::FileRef> filePackages;
    for (const auto &f : files) {
        if (f.file.data(gs).sourceType == core::File::Type::Package) {
            filePackages.insert(f.file);
        }
    }

    for (u4 i = 1; i < gs.filesUsed(); i++) {
        core::FileRef fref(i);
        if (fref.data(gs).sourceType == core::File::Type::Package && !filePackages.contains(fref)) {
            return false;
        }
    }

    return true;
}

} // namespace

vector<ast::ParsedFile> Packager::run(core::GlobalState &gs, WorkerPool &workers, vector<ast::ParsedFile> files) {
    Timer timeit(gs.tracer(), "packager");
    // Ensure files are in canonical order.
    fast_sort(files, [](const auto &a, const auto &b) -> bool { return a.file < b.file; });

    // Step 1: Find packages and determine their imports/exports.
    PackageDB packageDB;
    {
        Timer timeit(gs.tracer(), "packager.findPackages");
        core::UnfreezeNameTable unfreeze(gs);
        for (auto &file : files) {
            if (FileOps::getFileName(file.file.data(gs).path()) == PACKAGE_FILE_NAME) {
                file.file.data(gs).sourceType = core::File::Type::Package;
                core::MutableContext ctx(gs, core::Symbols::root(), file.file);
                packageDB.addPackage(ctx, getPackageInfo(ctx, file));
            }
        }
        // We're done adding packages.
        packageDB.finalizePackages();
    }

    {
        Timer timeit(gs.tracer(), "packager.rewritePackages");
        // Step 2: Rewrite packages. Can be done in parallel (and w/ step 3) if this becomes a bottleneck.
        for (auto &file : files) {
            if (file.file.data(gs).sourceType == core::File::Type::Package) {
                core::Context ctx(gs, core::Symbols::root(), file.file);
                file = rewritePackage(ctx, move(file), packageDB);
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

        const PackageDB &constPkgDB = packageDB;

        workers.multiplexJob("rewritePackagedFiles", [&gs, constPkgDB, fileq, resultq]() {
            Timer timeit(gs.tracer(), "packager.rewritePackagedFilesWorker");
            vector<ast::ParsedFile> results;
            u4 filesProcessed = 0;
            ast::ParsedFile job;
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    filesProcessed++;
                    if (job.file.data(gs).sourceType == core::File::Type::Normal) {
                        core::Context ctx(gs, core::Symbols::root(), job.file);
                        if (auto pkg = constPkgDB.getPackageForContext(ctx)) {
                            job = rewritePackagedFile(ctx, move(job), pkg->name.mangledName);
                        } else {
                            // Don't transform, but raise an error on the first line.
                            if (auto e =
                                    ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::UnpackagedFile)) {
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
    // TODO(jvilk): This incremental pass reprocesses every package file in the project. It should instead only process
    // the packages needed to understand file changes.
    ENFORCE(checkContainsAllPackages(gs, files));
    auto namesUsed = gs.namesUsed();
    auto emptyWorkers = WorkerPool::create(0, gs.tracer());
    files = Packager::run(gs, *emptyWorkers, move(files));
    ENFORCE(gs.namesUsed() == namesUsed);
    return files;
}

} // namespace sorbet::packager
