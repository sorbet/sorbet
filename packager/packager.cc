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

struct ImportedPackage {
    core::LocOffsets loc;
    core::NameRef mangledPackageName;
};

struct PackageInfo {
    // The path prefix before every file in the package, including path separator at end.
    std::string packagePathPrefix;
    // The mangled package name. Used to define the (internal) module name for its implementation.
    core::NameRef mangledName = core::NameRef::noName();
    // loc for the package definition. Used for error messages.
    core::Loc loc;
    // The mangled names of each package imported by this package.
    vector<ImportedPackage> importedPackageNames;
    // An AST expression that contains a module definition containing the exported items from this package.
    // Is copied into every package that imports this package.
    unique_ptr<ast::Expression> exportModule = ast::MK::EmptyTree();
};

class NameFormatter {
    const core::GlobalState &gs;

public:
    NameFormatter(const core::GlobalState &gs) : gs(gs) {}

    void operator()(std::string *out, core::NameRef name) const {
        out->append(name.data(gs)->shortName(gs));
    }
};

// Foo::Bar => Foo_Bar_Package
// Returns a nonexistant name if `name` is not a valid name.
core::NameRef manglePackageName(core::MutableContext ctx, std::unique_ptr<ast::Expression> &name) {
    vector<core::NameRef> rv;

    auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(name.get());
    while (constLit != nullptr) {
        rv.emplace_back(constLit->cnst);
        constLit = ast::cast_tree<ast::UnresolvedConstantLit>(constLit->scope.get());
    }
    reverse(rv.begin(), rv.end());
    if (!rv.empty()) {
        auto mangledName = absl::StrCat(absl::StrJoin(rv, "_", NameFormatter(ctx)), "_Package");
        return ctx.state.enterNameConstant(mangledName);
    } else {
        return core::NameRef::noName();
    }
}

bool isReferenceToPackageSpec(core::Context ctx, std::unique_ptr<ast::Expression> &expr) {
    auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(expr.get());
    return constLit != nullptr && constLit->cnst == core::Names::Constants::PackageSpec();
}

unique_ptr<ast::UnresolvedConstantLit> copyConstantLit(ast::UnresolvedConstantLit *lit) {
    auto copy = lit->deepCopy();
    // Cast from Expression to UnresolvedConstantLit.
    auto copyUcl = ast::cast_tree<ast::UnresolvedConstantLit>(copy.get());
    ENFORCE(copyUcl != nullptr);
    unique_ptr<ast::UnresolvedConstantLit> rv(copyUcl);
    copy.release();
    return rv;
}

ast::UnresolvedConstantLit *verifyConstantLit(core::Context ctx, core::NameRef fun, unique_ptr<ast::Expression> &expr) {
    auto target = ast::cast_tree<ast::UnresolvedConstantLit>(expr.get());
    if (target == nullptr) {
        if (auto e = ctx.beginError(expr->loc, core::errors::Packager::InvalidImportOrExport)) {
            e.setHeader("Argument passed to `{}` must be a constant literal", fun.data(ctx)->shortName(ctx));
        }
        return nullptr;
    }

    return target;
}

unique_ptr<ast::UnresolvedConstantLit> name2Expr(core::NameRef name,
                                                 unique_ptr<ast::Expression> scope = ast::MK::EmptyTree()) {
    return ast::MK::UnresolvedConstant(core::LocOffsets::none(), move(scope), name);
}

struct PackageInfoFinder {
    shared_ptr<PackageInfo> info = make_shared<PackageInfo>();
    unique_ptr<ast::Expression> packageModuleName;
    vector<unique_ptr<ast::UnresolvedConstantLit>> exported;
    vector<unique_ptr<ast::UnresolvedConstantLit>> exportedMethods;

    unique_ptr<ast::Send> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> send) {
        if (!info->mangledName.exists()) {
            // We haven't yet entered the package class.
            return send;
        }

        if (send->fun == core::Names::export_() && send->args.size() == 1) {
            auto target = verifyConstantLit(ctx, send->fun, send->args[0]);
            // null indicates an invalid export.
            if (target != nullptr) {
                exported.emplace_back(copyConstantLit(target));
                // Transform the constant lit to refer to the target within the mangled package namespace.
                send->args[0] = prependInternalPackageNameToScope(move(send->args[0]));
            }
        }

        if (send->fun == core::Names::import_() && send->args.size() == 1) {
            auto target = verifyConstantLit(ctx, send->fun, send->args[0]);
            // null indicates an invalid import.
            if (target != nullptr) {
                auto name = manglePackageName(ctx, send->args[0]);
                if (name.exists()) {
                    info->importedPackageNames.emplace_back(ImportedPackage{send->args[0]->loc, name});
                }
            }
        }

        if (send->fun == core::Names::exportMethods()) {
            for (auto &arg : send->args) {
                auto target = verifyConstantLit(ctx, send->fun, arg);
                if (target != nullptr) {
                    exportedMethods.emplace_back(copyConstantLit(target));
                }
                // Transform the constant lit to refer to the target within the mangled package namespace.
                arg = prependInternalPackageNameToScope(move(arg));
            }
        }

        return send;
    }

    unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> classDef) {
        if (classDef->symbol == core::Symbols::root()) {
            // Ignore top-level <root>
            return classDef;
        }

        if (classDef->ancestors.size() != 1 || !isReferenceToPackageSpec(ctx, classDef->ancestors[0])) {
            if (auto e = ctx.beginError(classDef->loc, core::errors::Packager::InvalidPackageDefinition)) {
                e.setHeader("Expected package definition of form `Foo::Bar < PackageSpec`");
            }
        } else {
            info->mangledName = manglePackageName(ctx, classDef->name);
            packageModuleName = classDef->name->deepCopy();
            info->loc = core::Loc(ctx.file, classDef->loc);
        }

        return classDef;
    }

    // Bar::Baz => <PackageRegistry>::Foo_Package::Bar::Baz
    unique_ptr<ast::Expression> prependInternalPackageNameToScope(unique_ptr<ast::Expression> scope) {
        // For `Bar::Baz::Bat`, `UnresolvedConstantLit` will contain `Bar`.
        ast::UnresolvedConstantLit *lastConstLit = ast::cast_tree<ast::UnresolvedConstantLit>(scope.get());
        if (lastConstLit != nullptr) {
            while (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(lastConstLit->scope.get())) {
                lastConstLit = constLit;
            }
        }

        // If `lastConstLit` is `nullptr`, then `scope` should be EmptyTree.
        ENFORCE(lastConstLit != nullptr || ast::cast_tree<ast::EmptyTree>(scope.get()) != nullptr);

        auto scopeToPrepend = name2Expr(this->info->mangledName, name2Expr(core::Names::Constants::PackageRegistry()));
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
    //     ExportedItem1 = <PackageRegistry>::Path::To::ExportedItem1
    //     extend <PackageRegistry>::Path::To::ExportedMethodsClass
    //   end
    void finalize(core::MutableContext ctx) {
        if (!this->info->mangledName.exists()) {
            return;
        }
        ENFORCE(this->packageModuleName != nullptr);

        // Build tree from bottom up, starting with all of the exported items.
        ast::ClassDef::RHS_store exportedItems;
        // Prepend to scope

        for (auto &klass : exported) {
            // Item = <PackageRegistry>::MangledPackageName::Path::To::Item
            exportedItems.emplace_back(
                ast::MK::Assign(core::LocOffsets::none(), name2Expr(klass->cnst),
                                name2Expr(klass->cnst, prependInternalPackageNameToScope(klass->scope->deepCopy()))));
        }

        for (auto &klass : exportedMethods) {
            // extend <PackageRegistry>::MangledPackageName::Path::To::Item
            // TODO(jvilk): What's the recv?
            exportedItems.emplace_back(
                ast::MK::Send1(core::LocOffsets::none(), ast::MK::EmptyTree(), core::Names::extend(),
                               name2Expr(klass->cnst, prependInternalPackageNameToScope(klass->scope->deepCopy()))));
        }

        this->info->exportModule = ast::MK::ClassOrModule(
            core::LocOffsets::none(), core::Loc(ctx.file, core::LocOffsets::none()), move(this->packageModuleName), {},
            std::move(exportedItems), ast::ClassDef::Kind::Module);
    }
};

shared_ptr<PackageInfo> getPackageInfo(core::MutableContext ctx, ast::ParsedFile &package) {
    ENFORCE(package.file.exists());
    ENFORCE(package.file.data(ctx).sourceType == core::File::Type::Package);
    auto packageFilePath = package.file.data(ctx).path();
    ENFORCE(FileOps::getFileName(packageFilePath) == PACKAGE_FILE_NAME);
    PackageInfoFinder finder;
    // TODO(jvilk): Test - root level package.
    finder.info->packagePathPrefix = packageFilePath.substr(0, packageFilePath.find_last_of('/') + 1);
    package.tree = ast::TreeMap::apply(ctx, finder, move(package.tree));
    finder.finalize(ctx);
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
        auto it = packageInfoByMangledName.find(imported.mangledPackageName);
        if (it == packageInfoByMangledName.end()) {
            if (auto e = ctx.beginError(imported.loc, core::errors::Packager::PackageNotFound)) {
                // TODO: Would be nice to pretty print the imported package name.
                e.setHeader("Cannot find package {}", imported.mangledPackageName.data(ctx)->show(ctx));
            }
            continue;
        }

        const auto &found = *it;
        importedPackages.emplace_back(found.second->exportModule->deepCopy());
    }

    auto packageNamespace =
        ast::MK::ClassOrModule(core::LocOffsets::none(), core::Loc(ctx.file, core::LocOffsets::none()),
                               name2Expr(package.mangledName, name2Expr(core::Names::Constants::PackageRegistry())), {},
                               std::move(importedPackages), ast::ClassDef::Kind::Module);

    auto rootKlass = ast::cast_tree<ast::ClassDef>(file.tree.get());
    ENFORCE(rootKlass != nullptr);
    rootKlass->rhs.emplace_back(move(packageNamespace));
    return file;
}

ast::ParsedFile rewritePackagedFile(core::Context ctx, ast::ParsedFile file, core::NameRef packageMangledName) {
    auto rootKlass = ast::cast_tree<ast::ClassDef>(file.tree.get());
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
                auto &existing = packageInfoByMangledName[pkg->mangledName];
                if (existing != nullptr) {
                    if (auto e = ctx.beginError(pkg->loc.offsets(), core::errors::Packager::DuplicatePackageName)) {
                        e.setHeader("Redefinition of package `{}`", pkg->mangledName.data(gs)->show(gs));
                        e.addErrorLine(existing->loc, "Package `{}` originally defined here",
                                       existing->mangledName.data(gs)->show(gs));
                    }
                } else {
                    existing = pkg;
                }
                packageInfoByFile[file.file] = pkg;
                packages.emplace_back(move(pkg));
            }
        }
    }

    // Sort packages so that packages with the longest/most specific paths are first.
    // That way, the first package to match a file's path is the most specific package match.
    fast_sort(packages, [](const auto &a, const auto &b) -> bool {
        return a->packagePathPrefix.size() > b->packagePathPrefix.size();
    });

    if (packageInfoByFile.empty()) {
        // No packages.
        return files;
    }

    {
        Timer timeit(gs.tracer(), "packager.rewritePackages");
        // Step 2: Rewrite packages. Can be done in parallel (and w/ step 3) if this becomes a bottleneck.
        for (auto &file : files) {
            if (file.file.data(gs).sourceType == core::File::Type::Package) {
                auto &packageInfo = packageInfoByFile[file.file];
                ENFORCE(packageInfo != nullptr)

                core::Context ctx(gs, core::Symbols::root(), file.file);
                file = rewritePackage(ctx, move(file), *packageInfo, packageInfoByMangledName);
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
                    if (job.file.data(gs).sourceType != core::File::Type::Package) {
                        // Check if file path is part of a package.
                        // TODO(jvilk): Could use a radix tree to make this lookup more efficient.
                        auto path = job.file.data(gs).path();
                        for (const auto &pkg : packages) {
                            if (absl::StartsWith(path, pkg->packagePathPrefix)) {
                                core::Context ctx(gs, core::Symbols::root(), job.file);
                                job = rewritePackagedFile(ctx, move(job), pkg->mangledName);
                                break;
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