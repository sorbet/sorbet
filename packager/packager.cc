#include "packager/packager.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
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
};

struct PackageInfo {
    // The path prefix before every file in the package, including path separator at end.
    std::string packagePathPrefix;
    PackageName name;
    // loc for the package definition. Used for error messages.
    core::Loc loc;
    // The mangled names of each package imported by this package.
    vector<PackageName> importedPackageNames;
    // An AST expression that contains a module definition containing the exported items from this package.
    // Is copied into every package that imports this package.
    ast::TreePtr exportModule = ast::MK::EmptyTree();
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
PackageName getPackageName(core::MutableContext ctx, ast::TreePtr &name) {
    PackageName pName;
    pName.loc = name->loc;

    auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(name);
    while (constLit != nullptr) {
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

ast::UnresolvedConstantLit *verifyConstant(core::MutableContext ctx, core::NameRef fun, ast::TreePtr &expr) {
    auto target = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    if (target == nullptr) {
        if (auto e = ctx.beginError(expr->loc, core::errors::Packager::InvalidImportOrExport)) {
            e.setHeader("Argument to `{}` must be a constant", fun.show(ctx));
        }
    }
    return target;
}

struct PackageInfoFinder {
    shared_ptr<PackageInfo> info = nullptr;
    ast::TreePtr packageModuleName;
    vector<unique_ptr<ast::UnresolvedConstantLit>> exported;
    vector<unique_ptr<ast::UnresolvedConstantLit>> exportedMethods;
    core::LocOffsets exportMethodsLoc = core::LocOffsets::none();

    ast::TreePtr postTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);
        if (info == nullptr) {
            // We haven't yet entered the package class.
            return tree;
        }

        if (send->fun == core::Names::export_() && send->args.size() == 1) {
            auto target = verifyConstant(ctx, core::Names::export_(), send->args[0]);
            // null indicates an invalid export.
            if (target != nullptr) {
                exported.emplace_back(copyConstantLit(target));
                // Transform the constant lit to refer to the target within the mangled package namespace.
                send->args[0] = prependInternalPackageNameToScope(move(send->args[0]));
            }
        }

        if (send->fun == core::Names::import_() && send->args.size() == 1) {
            auto target = verifyConstant(ctx, core::Names::import_(), send->args[0]);
            // null indicates an invalid import.
            if (target != nullptr) {
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
                auto target = verifyConstant(ctx, core::Names::exportMethods(), arg);
                if (target != nullptr) {
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
            info = make_shared<PackageInfo>();
            // TODO(jvilk): Can getPackageName fail? e.g. empty name?
            info->name = getPackageName(ctx, classDef->name);
            packageModuleName = classDef->name->deepCopy();
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
        ast::ClassDef::RHS_store extendStatements;
        for (auto &klass : exportedMethods) {
            // include <PackageRegistry>::MangledPackageName::Path::To::Item
            extendStatements.emplace_back(
                ast::MK::Send1(klass->loc, ast::MK::Self(klass->loc), core::Names::include(),
                               name2Expr(klass->cnst, prependInternalPackageNameToScope(klass->scope->deepCopy()))));
        }
        // Use the loc of the `export_methods` call so `include` errors goes to the right place.
        classDef->rhs.emplace_back(ast::MK::ClassOrModule(
            exportMethodsLoc, core::Loc(ctx.file, exportMethodsLoc),
            name2Expr(core::Names::Constants::PackageMethods(),
                      name2Expr(this->info->name.mangledName, name2Expr(core::Names::Constants::PackageRegistry()))),
            {}, std::move(extendStatements), ast::ClassDef::Kind::Module));

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
        // TODO: Error if no package spec.
        if (info == nullptr) {
            return;
        }
        ENFORCE(this->packageModuleName != nullptr);

        // Build tree from bottom up, starting with all of the exported items.
        ast::ClassDef::RHS_store exportedItems;
        // Prepend to scope

        // NOTE: Don't assign locs to the code generated below. They encode offsets to __package.rb, but will be
        // transplanted into other __package.rbs and may no longer be valid.
        for (auto &klass : exported) {
            // Item = <PackageRegistry>::MangledPackageName::Path::To::Item
            exportedItems.emplace_back(
                ast::MK::Assign(core::LocOffsets::none(), name2Expr(klass->cnst),
                                name2Expr(klass->cnst, prependInternalPackageNameToScope(klass->scope->deepCopy()))));
        }

        if (!exportedMethods.empty()) {
            // Extend <PackageRegistry>::Name_of_package::<PackageMethods>
            exportedItems.emplace_back(
                ast::MK::Send1(core::LocOffsets::none(), ast::MK::Self(core::LocOffsets::none()), core::Names::extend(),
                               name2Expr(core::Names::Constants::PackageMethods(),
                                         name2Expr(this->info->name.mangledName,
                                                   name2Expr(core::Names::Constants::PackageRegistry())))));
        }

        this->info->exportModule = ast::MK::ClassOrModule(
            core::LocOffsets::none(), core::Loc(ctx.file, core::LocOffsets::none()), move(this->packageModuleName), {},
            std::move(exportedItems), ast::ClassDef::Kind::Module);
    }
};

// Sanity checks package files, mutates arguments to export / export_methods to point to item in namespace,
// builds up the expression injected into packages that import the package, and codegens the <PackagedMethods>  module.
shared_ptr<PackageInfo> getPackageInfo(core::MutableContext ctx, ast::ParsedFile &package) {
    ENFORCE(package.file.exists());
    ENFORCE(package.file.data(ctx).sourceType == core::File::Type::Package);
    // Assumption: Root of AST is <root> class.
    ENFORCE(static_cast<ast::ClassDef *>(package.tree.get()) != nullptr);
    ENFORCE(static_cast<ast::ClassDef *>(package.tree.get())->symbol == core::Symbols::root());
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

    // TODO: Don't allow duplicate imports.
    for (auto imported : package.importedPackageNames) {
        auto it = packageInfoByMangledName.find(imported.mangledName);
        if (it == packageInfoByMangledName.end()) {
            if (auto e = ctx.beginError(imported.loc, core::errors::Packager::PackageNotFound)) {
                e.setHeader("Cannot find package `{}`", absl::StrJoin(imported.fullName, "::", NameFormatter(ctx)));
            }
            continue;
        }

        const auto &found = *it;
        importedPackages.emplace_back(found.second->exportModule->deepCopy());
    }

    auto packageNamespace = ast::MK::ClassOrModule(
        core::LocOffsets::none(), core::Loc(ctx.file, core::LocOffsets::none()),
        name2Expr(package.name.mangledName, name2Expr(core::Names::Constants::PackageRegistry())), {},
        std::move(importedPackages), ast::ClassDef::Kind::Module);

    auto rootKlass = ast::cast_tree<ast::ClassDef>(file.tree);
    ENFORCE(rootKlass != nullptr);
    rootKlass->rhs.emplace_back(move(packageNamespace));
    return file;
}

ast::ParsedFile rewritePackagedFile(core::Context ctx, ast::ParsedFile file, core::NameRef packageMangledName) {
    auto rootKlass = ast::cast_tree<ast::ClassDef>(file.tree);
    if (rootKlass == nullptr) {
        // TODO: Why does this happen?
        if (auto e = ctx.beginError(file.tree->loc, core::errors::Packager::PackageNotFound)) {
            e.setHeader("What's going on?");
        }
        return file;
    }
    ENFORCE(rootKlass != nullptr);
    auto moduleWrapper =
        ast::MK::ClassOrModule(core::LocOffsets::none(), core::Loc(ctx.file, core::LocOffsets::none()),
                               name2Expr(packageMangledName, name2Expr(core::Names::Constants::PackageRegistry())), {},
                               std::move(rootKlass->rhs), ast::ClassDef::Kind::Module);
    rootKlass->rhs.clear();
    rootKlass->rhs.emplace_back(move(moduleWrapper));
    return file;
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
                auto pkg = getPackageInfo(ctx, file);
                // TODO: Warn if no package found.
                if (pkg != nullptr) {
                    auto &existing = packageInfoByMangledName[pkg->name.mangledName];
                    if (existing != nullptr) {
                        if (auto e =
                                ctx.beginError(pkg->loc.offsets(), core::errors::Packager::RedefinitionOfPackage)) {
                            auto pkgName = absl::StrJoin(pkg->name.fullName, "::", NameFormatter(ctx));
                            e.setHeader("Redefinition of package `{}`", pkgName);
                            e.addErrorLine(existing->loc, "Package `{}` originally defined here", pkgName);
                        }
                    } else {
                        existing = pkg;
                    }
                    packageInfoByFile[file.file] = pkg;
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
                auto &packageInfo = packageInfoByFile[file.file];
                if (packageInfo != nullptr) {
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
                        // TODO(jvilk): Could use a radix tree to make this lookup more efficient.
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
    auto namesUsed = gs.namesUsed();
    auto emptyWorkers = WorkerPool::create(0, gs.tracer());
    files = Packager::run(gs, *emptyWorkers, move(files));
    ENFORCE(gs.namesUsed() == namesUsed);
    return files;
}

} // namespace sorbet::packager