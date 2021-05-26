#include "packager/packager.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "common/FileOps.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/concurrency/WorkerPool.h"
#include "common/formatting.h"
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
    ast::ExpressionPtr toLiteral(core::LocOffsets loc) const;
};

class NameFormatter final {
    const core::GlobalState &gs;

public:
    NameFormatter(const core::GlobalState &gs) : gs(gs) {}

    void operator()(std::string *out, core::NameRef name) const {
        out->append(name.shortName(gs));
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
    vector<FullyQualifiedName> exports;
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
        if (absl::StrContains(constLit->cnst.shortName(ctx), "_")) {
            // By forbidding package names to have an underscore, we can trivially convert between mangled names and
            // unmangled names by replacing `_` with `::`.
            if (auto e = ctx.beginError(constLit->loc, core::errors::Packager::InvalidPackageName)) {
                e.setHeader("Package names cannot contain an underscore");
                auto replacement = absl::StrReplaceAll(constLit->cnst.shortName(ctx), {{"_", ""}});
                auto nameLoc = constLit->loc;
                // cnst is the last characters in the constant literal
                nameLoc.beginLoc = nameLoc.endLoc - constLit->cnst.shortName(ctx).size();

                e.addAutocorrect(core::AutocorrectSuggestion{
                    fmt::format("Replace `{}` with `{}`", constLit->cnst.shortName(ctx), replacement),
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
    auto utf8Name = ctx.state.enterNameUTF8(mangledName);
    auto packagerName = ctx.state.freshNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    pName.mangledName = ctx.state.enterNameConstant(packagerName);

    return pName;
}

bool isReferenceToPackageSpec(core::Context ctx, ast::ExpressionPtr &expr) {
    auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return constLit != nullptr && constLit->cnst == core::Names::Constants::PackageSpec();
}

ast::ExpressionPtr name2Expr(core::NameRef name, ast::ExpressionPtr scope = ast::MK::EmptyTree(),
                             core::LocOffsets loc = core::LocOffsets::none()) {
    return ast::MK::UnresolvedConstant(loc, move(scope), name);
}

ast::ExpressionPtr FullyQualifiedName::toLiteral(core::LocOffsets loc) const {
    ast::ExpressionPtr name = ast::MK::EmptyTree();
    for (auto part : parts) {
        name = name2Expr(part, move(name));
    }
    // Outer name should have the provided loc.
    if (auto lit = ast::cast_tree<ast::UnresolvedConstantLit>(name)) {
        name = ast::MK::UnresolvedConstant(loc, move(lit->scope), lit->cnst);
    }
    return name;
}

ast::ExpressionPtr parts2literal(const vector<core::NameRef> &parts, core::LocOffsets loc) {
    ast::ExpressionPtr name = ast::MK::EmptyTree();
    for (auto part : parts) {
        name = name2Expr(part, move(name));
    }
    // Outer name should have the provided loc.
    if (auto *lit = ast::cast_tree<ast::UnresolvedConstantLit>(name)) {
        name = ast::MK::UnresolvedConstant(loc, move(lit->scope), lit->cnst);
    }
    return name;
}

// Prefix a constant reference with a name: `Foo::Bar` -> `<name>::Foo::Bar`
ast::ExpressionPtr prependName(ast::ExpressionPtr scope, core::NameRef name) {
    // For `Bar::Baz::Bat`, `UnresolvedConstantLit` will contain `Bar`.
    auto *lastConstLit = ast::cast_tree<ast::UnresolvedConstantLit>(scope);
    if (lastConstLit != nullptr) {
        while (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(lastConstLit->scope)) {
            lastConstLit = constLit;
        }
    }

    // If `lastConstLit` is `nullptr`, then `scope` should be EmptyTree.
    ENFORCE(lastConstLit != nullptr || ast::isa_tree<ast::EmptyTree>(scope));

    auto scopeToPrepend = name2Expr(name, name2Expr(core::Names::Constants::PackageRegistry()));
    if (lastConstLit == nullptr) {
        return scopeToPrepend;
    } else {
        lastConstLit->scope = move(scopeToPrepend);
        return scope;
    }
}

ast::UnresolvedConstantLit *verifyConstant(core::MutableContext ctx, core::NameRef fun, ast::ExpressionPtr &expr) {
    auto target = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    if (target == nullptr) {
        if (auto e = ctx.beginError(expr.loc(), core::errors::Packager::InvalidImportOrExport)) {
            e.setHeader("Argument to `{}` must be a constant", fun.show(ctx));
        }
    }
    return target;
}

bool isPrefix(const vector<core::NameRef> &prefix, const vector<core::NameRef> &names) {
    size_t minSize = std::min(prefix.size(), names.size());
    ENFORCE(minSize > 0);
    return std::equal(prefix.begin(), prefix.end(), names.begin(), names.begin() + minSize);
}

bool sharesPrefix(const vector<core::NameRef> &a, const vector<core::NameRef> &b) {
    size_t minSize = std::min(a.size(), b.size());
    ENFORCE(minSize > 0);
    return std::equal(a.begin(), a.begin() + minSize, b.begin(), b.begin() + minSize);
}

// Visitor that ensures for constants defined within a package that all have the package as a
// prefix.
class EnforcePackagePrefix final {
    const PackageInfo *pkg;
    vector<core::NameRef> nameParts;
    int rootConsts = 0;
    int skipPush = 0;

public:
    EnforcePackagePrefix(const PackageInfo *pkg) : pkg(pkg) {
        ENFORCE(pkg != nullptr);
    }

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root()) {
            // Ignore top-level <root>
            return tree;
        }
        const auto &pkgName = pkg->name.fullName.parts;
        if (nameParts.size() > pkgName.size()) {
            // At this depth we can stop checking the prefixes since beyond the end of the prefix.
            skipPush++;
            return tree;
        }
        auto &constantLit = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(classDef.name);
        pushConstantLit(&constantLit);

        if (rootConsts == 0 && !sharesPrefix(pkgName, nameParts)) {
            if (auto e = ctx.beginError(constantLit.loc, core::errors::Packager::DefinitionPackageMismatch)) {
                e.setHeader(
                    "Class or method definition must match enclosing package namespace `{}`",
                    fmt::map_join(pkgName.begin(), pkgName.end(), "::", [&](const auto &nr) { return nr.show(ctx); }));
            }
        }
        return tree;
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root()) {
            // Sanity check bookkeeping
            ENFORCE(nameParts.size() == 0);
            ENFORCE(rootConsts == 0);
            ENFORCE(skipPush == 0);
            return tree;
        }
        auto *constantLit = &ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(classDef.name);
        if (skipPush > 0) {
            skipPush--;
            return tree;
        }
        popConstantLit(constantLit);
        return tree;
    }

    ast::ExpressionPtr preTransformAssign(core::Context ctx, ast::ExpressionPtr original) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(original);
        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn.lhs);
        if (lhs != nullptr) {
            auto &pkgName = pkg->name.fullName.parts;
            if (rootConsts == 0 && !isPrefix(pkgName, nameParts)) {
                if (auto e = ctx.beginError(lhs->loc, core::errors::Packager::DefinitionPackageMismatch)) {
                    e.setHeader("Constants may not be defined outside of the enclosing package namespace `{}`",
                                fmt::map_join(pkgName.begin(), pkgName.end(),
                                              "::", [&](const auto &nr) { return nr.show(ctx); }));
                }
            }
        }
        return original;
    }

private:
    void pushConstantLit(ast::UnresolvedConstantLit *lit) {
        auto oldLen = nameParts.size();
        while (lit != nullptr) {
            nameParts.emplace_back(lit->cnst);
            auto *scope = ast::cast_tree<ast::ConstantLit>(lit->scope);
            lit = ast::cast_tree<ast::UnresolvedConstantLit>(lit->scope);
            if (scope != nullptr) {
                ENFORCE(lit == nullptr);
                ENFORCE(scope->symbol == core::Symbols::root());
                rootConsts++;
            }
        }
        reverse(nameParts.begin() + oldLen, nameParts.end());
    }

    void popConstantLit(ast::UnresolvedConstantLit *lit) {
        while (lit != nullptr) {
            nameParts.pop_back();
            auto *scope = ast::cast_tree<ast::ConstantLit>(lit->scope);
            lit = ast::cast_tree<ast::UnresolvedConstantLit>(lit->scope);
            if (scope != nullptr) {
                ENFORCE(lit == nullptr);
                ENFORCE(scope->symbol == core::Symbols::root());
                rootConsts--;
            }
        }
    }
};

struct PackageInfoFinder {
    unique_ptr<PackageInfo> info = nullptr;
    vector<FullyQualifiedName> exported;

    ast::ExpressionPtr postTransformSend(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);

        // Ignore methods
        if (send.fun == core::Names::keepDef() || send.fun == core::Names::keepSelfDef()) {
            return tree;
        }

        // Disallowed methods
        if (send.fun == core::Names::extend() || send.fun == core::Names::include()) {
            if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidPackageExpression)) {
                e.setHeader("Invalid expression in package: `{}` is not allowed", send.fun.shortName(ctx));
            }
            return tree;
        }

        // Sanity check arguments for unrecognized methods
        if (send.fun != core::Names::export_() && send.fun != core::Names::import()) {
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

        return tree;
    }

    ast::ExpressionPtr preTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
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

    // Bar::Baz => <PackageRegistry>::Foo_Package::Bar::Baz
    ast::ExpressionPtr prependInternalPackageName(ast::ExpressionPtr scope) {
        return prependName(move(scope), this->info->name.mangledName);
    }

    // Generate a list of FQNs exported by this package. No export may be a prefix of another.
    void finalize(core::MutableContext ctx) {
        if (info == nullptr) {
            // HACKFIX: Tolerate completely empty packages. LSP does not support the notion of a deleted file, and
            // instead replaces deleted files with the empty string. It should really mark files as Tombstones instead.
            if (!ctx.file.data(ctx).source().empty()) {
                if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::InvalidPackageDefinition)) {
                    e.setHeader("Package file must contain a package definition of form `Foo::Bar < PackageSpec`");
                }
            }
            return;
        }

        if (exported.empty()) {
            return;
        }
        fast_sort(exported, [](const auto &a, const auto &b) -> bool { return a.parts.size() < b.parts.size(); });
        // TODO(nroman) If this is too slow could probably be sped up with lexigraphic sort.
        for (auto longer = exported.begin() + 1; longer != exported.end(); longer++) {
            for (auto shorter = exported.begin(); shorter != longer; shorter++) {
                if (std::equal(longer->parts.begin(), longer->parts.begin() + shorter->parts.size(),
                               shorter->parts.begin())) {
                    if (auto e = ctx.beginError(longer->loc.offsets(), core::errors::Packager::ImportConflict)) {
                        e.setHeader("Exported names may not be prefixes of each other");
                        e.addErrorLine(shorter->loc, "Prefix exported here");
                    }
                    break; // Only need to find the shortest conflicting export
                }
            }
        }

        ENFORCE(info->exports.empty());
        std::swap(exported, info->exports);
    }

    /* Forbid arbitrary computation in packages */

    void illegalNode(core::MutableContext ctx, core::LocOffsets loc, string_view type) {
        if (auto e = ctx.beginError(loc, core::errors::Packager::InvalidPackageExpression)) {
            e.setHeader("Invalid expression in package: {} not allowed", type);
        }
    }

    ast::ExpressionPtr preTransformIf(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "`if`");
        return original;
    }

    ast::ExpressionPtr preTransformWhile(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "`while`");
        return original;
    }

    ast::ExpressionPtr postTransformBreak(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "`break`");
        return original;
    }

    ast::ExpressionPtr postTransformRetry(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "`retry`");
        return original;
    }

    ast::ExpressionPtr postTransformNext(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "`next`");
        return original;
    }

    ast::ExpressionPtr preTransformReturn(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "`return`");
        return original;
    }

    ast::ExpressionPtr preTransformRescueCase(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "`rescue case`");
        return original;
    }

    ast::ExpressionPtr preTransformRescue(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "`rescue`");
        return original;
    }

    ast::ExpressionPtr preTransformAssign(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "`=`");
        return original;
    }

    ast::ExpressionPtr preTransformHash(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "hash literals");
        return original;
    }

    ast::ExpressionPtr preTransformArray(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "array literals");
        return original;
    }

    ast::ExpressionPtr preTransformMethodDef(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "method definitions");
        return original;
    }

    ast::ExpressionPtr preTransformBlock(core::MutableContext ctx, ast::ExpressionPtr original) {
        illegalNode(ctx, original.loc(), "blocks");
        return original;
    }

    ast::ExpressionPtr preTransformInsSeq(core::MutableContext ctx, ast::ExpressionPtr original) {
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

// For a given package, a tree that is the union of all constants exported by the packages it
// imports.
class ImportTree final {
    struct Source {
        core::NameRef packageMangledName;
        core::LocOffsets importLoc;
        bool exists() {
            return importLoc.exists();
        }
    };

    // To avoid conflicts, a node should either be a leaf (source exists, no children) OR have children
    // and an non-existent source. This is validated in `makeModule`.
    UnorderedMap<core::NameRef, std::unique_ptr<ImportTree>> children;
    Source source;

public:
    ImportTree() = default;
    ImportTree(const ImportTree &) = delete;
    ImportTree(ImportTree &&) = default;
    ImportTree &operator=(const ImportTree &) = delete;
    ImportTree &operator=(ImportTree &&) = default;

    friend class ImportTreeBuilder;
};

class ImportTreeBuilder final {
    PackageInfo package; // The package we are building an import tree for.
    ImportTree root;

public:
    ImportTreeBuilder(PackageInfo package) : package(package) {}
    ImportTreeBuilder(const ImportTreeBuilder &) = delete;
    ImportTreeBuilder(ImportTreeBuilder &&) = default;
    ImportTreeBuilder &operator=(const ImportTreeBuilder &) = delete;
    ImportTreeBuilder &operator=(ImportTreeBuilder &&) = default;

    void mergeImports(const PackageInfo &importedPackage, core::LocOffsets loc) {
        for (const auto &exportedFqn : importedPackage.exports) {
            addImport(importedPackage, loc, exportedFqn);
        }
    }

    ast::ClassDef::RHS_store makeModule(core::Context ctx) {
        vector<core::NameRef> parts;
        ast::ClassDef::RHS_store modRhs;
        makeModule(ctx, &root, parts, modRhs, ImportTree::Source());
        return modRhs;
    }

private:
    void addImport(const PackageInfo &importedPackage, core::LocOffsets loc, const FullyQualifiedName &exportFqn) {
        ImportTree *node = &root;
        for (auto nameRef : exportFqn.parts) {
            auto &child = node->children[nameRef];
            if (!child) {
                child = make_unique<ImportTree>();
            }
            node = child.get();
        }
        node->source = {importedPackage.name.mangledName, loc};
    }

    void makeModule(core::Context ctx, ImportTree *node, vector<core::NameRef> &parts, ast::ClassDef::RHS_store &modRhs,
                    ImportTree::Source parentSrc) {
        auto newParentSrc = parentSrc;
        if (node->source.exists() && !parentSrc.exists()) {
            newParentSrc = node->source;
        }

        // Sort by name for stability
        vector<pair<core::NameRef, ImportTree *>> childPairs;
        std::transform(node->children.begin(), node->children.end(), back_inserter(childPairs),
                       [](const auto &pair) { return make_pair(pair.first, pair.second.get()); });
        fast_sort(childPairs, [&ctx](const auto &lhs, const auto &rhs) -> bool {
            return lhs.first.show(ctx) < rhs.first.show(ctx);
        });
        for (auto const &[nameRef, child] : childPairs) {
            parts.emplace_back(nameRef);
            makeModule(ctx, child, parts, modRhs, newParentSrc);
            parts.pop_back();
        }

        if (node->source.exists()) {
            if (parentSrc.exists()) {
                if (auto e = ctx.beginError(node->source.importLoc, core::errors::Packager::ImportConflict)) {
                    // TODO Fix flaky ordering of errors. This is strange...not being done in parallel,
                    // and the file processing order is consistent.
                    e.setHeader("Conflicting import sources for `{}`",
                                fmt::map_join(parts, "::", [&](const auto &nr) { return nr.show(ctx); }));
                    e.addErrorLine(core::Loc(ctx.file, parentSrc.importLoc), "Conflict from");
                }
            } else {
                // Construct a module containing an assignment for an imported name:
                // For name `A::B::C::D` imported from package `A::B` construct:
                // module A::B::C
                //   D = <Mangled A::B>::A::B::C::D
                // end
                auto importLoc = node->source.importLoc;
                auto assignRhs =
                    prependName(parts2literal(parts, core::LocOffsets::none()), node->source.packageMangledName);
                auto assign = ast::MK::Assign(core::LocOffsets::none(), name2Expr(parts.back(), ast::MK::EmptyTree()),
                                              std::move(assignRhs));

                ast::ClassDef::RHS_store rhs;
                rhs.emplace_back(std::move(assign));
                // Use the loc from the import in the module name and declaration to get the
                // following jump to definition behavior:
                // imported constant: `Foo::Bar::Baz` from package `Foo::Bar`
                //                     ^^^^^^^^       jump to the import statement
                //                               ^^^  jump to actual definition of `Baz` class
                auto mod = ast::MK::Module(core::LocOffsets::none(), importLoc, importModuleName(parts, importLoc), {},
                                           std::move(rhs));
                modRhs.emplace_back(std::move(mod));
            }
        }
    }

    ast::ExpressionPtr importModuleName(vector<core::NameRef> &parts, core::LocOffsets importLoc) {
        ast::ExpressionPtr name = name2Expr(package.name.mangledName);
        for (auto part = parts.begin(); part < parts.end() - 1; part++) {
            name = name2Expr(*part, move(name));
        }
        // Put the loc on the outer name:
        auto &lit = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(name);
        return ast::MK::UnresolvedConstant(importLoc, move(lit.scope), lit.cnst);
    }
};

// Add:
//    module <PackageRegistry>::Mangled_Name_Package
//      module A::B::C
//        D = Mangled_Imported_Package::A::B::C::D
//      end
//      ...
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
    if (file.file.data(ctx).originalSigil < core::StrictLevel::Strict) {
        if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::PackageFileMustBeStrict)) {
            e.setHeader("Package files must be at least `{}`", "# typed: strict");
        }
    }

    {
        UnorderedMap<core::NameRef, core::LocOffsets> importedNames;
        ImportTreeBuilder treeBuilder(*package);
        for (auto &imported : package->importedPackageNames) {
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
                treeBuilder.mergeImports(*importedPackage, imported.loc);
            }
        }
        importedPackages = treeBuilder.makeModule(ctx);
    }

    auto packageNamespace =
        ast::MK::Module(core::LocOffsets::none(), core::LocOffsets::none(),
                        name2Expr(core::Names::Constants::PackageRegistry()), {}, std::move(importedPackages));

    auto &rootKlass = ast::cast_tree_nonnull<ast::ClassDef>(file.tree);
    rootKlass.rhs.emplace_back(move(packageNamespace));
    return file;
}

ast::ParsedFile rewritePackagedFile(core::Context ctx, ast::ParsedFile file, core::NameRef packageMangledName,
                                    const PackageInfo *pkg) {
    if (ast::isa_tree<ast::EmptyTree>(file.tree)) {
        // Nothing to wrap. This occurs when a file is marked typed: Ignore.
        return file;
    }

    auto &rootKlass = ast::cast_tree_nonnull<ast::ClassDef>(file.tree);
    EnforcePackagePrefix enforcePrefix(pkg);
    file.tree = ast::ShallowMap::apply(ctx, enforcePrefix, move(file.tree));
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
                            job = rewritePackagedFile(ctx, move(job), pkg->name.mangledName, pkg);
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
    auto namesUsed = gs.namesUsedTotal();
    auto emptyWorkers = WorkerPool::create(0, gs.tracer());
    files = Packager::run(gs, *emptyWorkers, move(files));
    ENFORCE(gs.namesUsedTotal() == namesUsed);
    return files;
}

} // namespace sorbet::packager
