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
#include <sys/stat.h>

using namespace std;

namespace sorbet::packager {
namespace {

constexpr string_view PACKAGE_FILE_NAME = "__package.rb"sv;
constexpr core::NameRef TEST_NAME = core::Names::Constants::Test();

bool isTestFile(const core::GlobalState &gs, core::File &file) {
    return absl::EndsWith(file.path(), ".test.rb") || absl::StrContains(gs.getPrintablePath(file.path()), "/test/");
}

struct FullyQualifiedName {
    vector<core::NameRef> parts;
    core::Loc loc;
    ast::ExpressionPtr toLiteral(core::LocOffsets loc) const;

    FullyQualifiedName() = default;
    FullyQualifiedName(vector<core::NameRef> parts, core::Loc loc) : parts(parts), loc(loc) {}
    explicit FullyQualifiedName(const FullyQualifiedName &) = default;
    FullyQualifiedName(FullyQualifiedName &&) = default;
    FullyQualifiedName &operator=(const FullyQualifiedName &) = delete;
    FullyQualifiedName &operator=(FullyQualifiedName &&) = default;

    FullyQualifiedName withPrefix(core::NameRef prefix) const {
        vector<core::NameRef> prefixed(parts.size() + 1);
        prefixed[0] = prefix;
        std::copy(parts.begin(), parts.end(), prefixed.begin() + 1);
        ENFORCE(prefixed.size() == parts.size() + 1);
        return {move(prefixed), loc};
    }
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
    FullyQualifiedName fullTestPkgName;

    // Pretty print the package's (user-observable) name (e.g. Foo::Bar)
    string toString(const core::GlobalState &gs) const {
        return absl::StrJoin(fullName.parts, "::", NameFormatter(gs));
    }
};

enum class ImportType {
    Normal,
    Test, // test_import
};

struct Import {
    PackageName name;
    ImportType type;

    Import(PackageName &&name, ImportType type) : name(std::move(name)), type(type) {}
};

enum class ExportType {
    Public,
    PrivateTest,
};

struct Export {
    FullyQualifiedName fqn;
    ExportType type;

    Export(FullyQualifiedName &&fqn, ExportType type) : fqn(move(fqn)), type(type) {}

    const vector<core::NameRef> &parts() const {
        return fqn.parts;
    }
};

struct PackageInfo {
    // The possible path prefixes associated with files in the package, including path separator at end.
    vector<std::string> packagePathPrefixes;
    PackageName name;
    // loc for the package definition. Used for error messages.
    core::Loc loc;
    // The names of each package imported by this package.
    vector<Import> importedPackageNames;
    // List of exported items that form the body of this package's public API.
    // These are copied into every package that imports this package.
    vector<Export> exports;
};

/**
 * Container class that facilitates thread-safe read-only access to packages.
 */
class PackageDB final {
private:
    // The only thread that is allowed write access to this class.
    const std::thread::id owner;
    UnorderedMap<std::string, shared_ptr<const PackageInfo>> packageInfoByPathPrefix;
    bool finalized = false;
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

        for (const std::string &packagePathPrefix : pkg->packagePathPrefixes) {
            packageInfoByPathPrefix[packagePathPrefix] = pkg;
        }
    }

    void finalizePackages() {
        ENFORCE(owner == this_thread::get_id());
        finalized = true;
    }

    /**
     * Given a file of type PACKAGE, return its PackageInfo or nullptr if one does not exist.
     */
    const PackageInfo *getPackageByFile(core::Context ctx, core::FileRef packageFile) const {
        const std::string_view path = packageFile.data(ctx).path();
        const auto &it = packageInfoByPathPrefix.find(path.substr(0, path.find_last_of('/') + 1));
        if (it == packageInfoByPathPrefix.end()) {
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

        std::string_view path = ctx.file.data(ctx).path();
        int curPrefixPos = path.find_last_of('/');
        while (curPrefixPos != std::string::npos) {
            const auto it = packageInfoByPathPrefix.find(path.substr(0, curPrefixPos + 1));
            if (it != packageInfoByPathPrefix.end()) {
                return it->second.get();
            }

            curPrefixPos = path.find_last_of('/', curPrefixPos - 1);
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
    pName.fullTestPkgName = pName.fullName.withPrefix(TEST_NAME);

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

// Prefix a constant reference with a name: `Foo::Bar` -> `<REGISTRY>::<name>::Foo::Bar`
// Registry is either <PackageRegistry> or <PackageTests>. The latter if following the convention
// that if scope starts with `Test::`.
ast::ExpressionPtr prependPackageScope(ast::ExpressionPtr scope, core::NameRef mangledName) {
    auto *lastConstLit = &ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(scope);
    while (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(lastConstLit->scope)) {
        lastConstLit = constLit;
    }
    core::NameRef registryName = core::Names::Constants::PackageRegistry();
    if (lastConstLit->cnst == TEST_NAME) {
        registryName = core::Names::Constants::PackageTests();
    }
    lastConstLit->scope = name2Expr(mangledName, name2Expr(registryName));
    return scope;
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
    const bool isTestFile;
    vector<core::NameRef> nameParts;
    int rootConsts = 0;
    int skipPush = 0;

public:
    EnforcePackagePrefix(const PackageInfo *pkg, bool isTestFile) : pkg(pkg), isTestFile(isTestFile) {
        ENFORCE(pkg != nullptr);
    }

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root()) {
            // Ignore top-level <root>
            return tree;
        }
        const auto &pkgName = requiredNamespace();
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
            auto &pkgName = requiredNamespace();
            bool needsLitPush = nameParts.size() < pkgName.size() && rootConsts == 0;
            if (needsLitPush) {
                pushConstantLit(lhs);
            }
            if (rootConsts == 0 && !isPrefix(pkgName, nameParts)) {
                if (auto e = ctx.beginError(lhs->loc, core::errors::Packager::DefinitionPackageMismatch)) {
                    e.setHeader("Constants may not be defined outside of the enclosing package namespace `{}`",
                                fmt::map_join(pkgName.begin(), pkgName.end(),
                                              "::", [&](const auto &nr) { return nr.show(ctx); }));
                }
            }
            if (needsLitPush) {
                popConstantLit(lhs);
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

    const vector<core::NameRef> &requiredNamespace() const {
        if (isTestFile) {
            return pkg->name.fullTestPkgName.parts;
        } else {
            return pkg->name.fullName.parts;
        }
    }
};

struct PackageInfoFinder {
    unique_ptr<PackageInfo> info = nullptr;
    vector<Export> exported;

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
        if (!isSpecMethod(send)) {
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
                exported.emplace_back(getFullyQualifiedName(ctx, target), ExportType::Public);
                // Transform the constant lit to refer to the target within the mangled package namespace.
                send.args[0] = prependInternalPackageName(move(send.args[0]));
            }
        }
        if (send.fun == core::Names::export_for_test() && send.args.size() == 1) {
            // null indicates an invalid export.
            if (auto target = verifyConstant(ctx, core::Names::export_for_test(), send.args[0])) {
                auto fqn = getFullyQualifiedName(ctx, target);
                ENFORCE(fqn.parts.size() > 0);
                if (fqn.parts[0] == TEST_NAME) {
                    if (auto e = ctx.beginError(target->loc, core::errors::Packager::InvalidExportForTest)) {
                        e.setHeader("Packages may not {} names in the `{}::` namespace", send.fun.toString(ctx),
                                    TEST_NAME.show(ctx));
                    }
                } else {
                    exported.emplace_back(move(fqn), ExportType::PrivateTest);
                }
                // Transform the constant lit to refer to the target within the mangled package namespace.
                send.args[0] = prependInternalPackageName(move(send.args[0]));
            }
        }

        if ((send.fun == core::Names::import() || send.fun == core::Names::test_import()) && send.args.size() == 1) {
            // null indicates an invalid import.
            if (auto target = verifyConstant(ctx, send.fun, send.args[0])) {
                auto name = getPackageName(ctx, target);
                if (name.mangledName == info->name.mangledName) {
                    if (auto e = ctx.beginError(target->loc, core::errors::Packager::NoSelfImport)) {
                        e.setHeader("Package `{}` cannot {} itself", info->name.toString(ctx), send.fun.toString(ctx));
                    }
                }
                info->importedPackageNames.emplace_back(move(name), method2ImportType(send));
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
        return prependPackageScope(move(scope), this->info->name.mangledName);
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
        fast_sort(exported, [](const auto &a, const auto &b) -> bool { return a.parts().size() < b.parts().size(); });
        // TODO(nroman) If this is too slow could probably be sped up with lexigraphic sort.
        for (auto longer = exported.begin() + 1; longer != exported.end(); longer++) {
            for (auto shorter = exported.begin(); shorter != longer; shorter++) {
                if (std::equal(longer->parts().begin(), longer->parts().begin() + shorter->parts().size(),
                               shorter->parts().begin())) {
                    if (auto e = ctx.beginError(longer->fqn.loc.offsets(), core::errors::Packager::ExportConflict)) {
                        e.setHeader(
                            "Cannot export `{}` because another exported name `{}` is a prefix of it",
                            fmt::map_join(longer->parts(), "::", [&](const auto &nr) { return nr.show(ctx); }),
                            fmt::map_join(shorter->parts(), "::", [&](const auto &nr) { return nr.show(ctx); }));
                        e.addErrorLine(shorter->fqn.loc, "Prefix exported here");
                    }
                    break; // Only need to find the shortest conflicting export
                }
            }
        }

        ENFORCE(info->exports.empty());
        std::swap(exported, info->exports);
    }

    bool isSpecMethod(const sorbet::ast::Send &send) const {
        switch (send.fun.rawId()) {
            case core::Names::import().rawId():
            case core::Names::test_import().rawId():
            case core::Names::export_().rawId():
            case core::Names::export_for_test().rawId():
                return true;
            default:
                return false;
        }
    }

    ImportType method2ImportType(const ast::Send &send) const {
        switch (send.fun.rawId()) {
            case core::Names::import().rawId():
                return ImportType::Normal;
            case core::Names::test_import().rawId():
                return ImportType::Test;
            default:
                ENFORCE(false);
                Exception::notImplemented();
        }
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

// TODO (aadi-stripe) we can avoid syscalls if we invent an efficient way of looking up
// directories in the source tree via GlobalState. Might be tied to https://github.com/sorbet/sorbet/issues/4509
bool pathExists(const std::string &path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

// Sanity checks package files, mutates arguments to export / export_methods to point to item in namespace,
// builds up the expression injected into packages that import the package, and codegens the <PackagedMethods>  module.
unique_ptr<PackageInfo> getPackageInfo(core::MutableContext ctx, ast::ParsedFile &package,
                                       const vector<std::string> &extraPackageFilesDirectoryPrefixes) {
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
        finder.info->packagePathPrefixes.emplace_back(packageFilePath.substr(0, packageFilePath.find_last_of('/') + 1));
        const string_view shortName = finder.info->name.mangledName.shortName(ctx.state);
        const string_view dirNameFromShortName = shortName.substr(0, shortName.find("_Package"));

        for (const string &prefix : extraPackageFilesDirectoryPrefixes) {
            string additionalDirPath = absl::StrCat(prefix, dirNameFromShortName, "/");
            if (pathExists(additionalDirPath)) {
                finder.info->packagePathPrefixes.emplace_back(std::move(additionalDirPath));
            }
        }
    }
    return move(finder.info);
}

// For a given package, a tree that is the union of all constants exported by the packages it
// imports.
class ImportTree final {
    struct Source {
        core::NameRef packageMangledName;
        core::LocOffsets importLoc;
        ImportType importType;
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
    // PackageInfo package; // The package we are building an import tree for.
    core::NameRef pkgMangledName;
    ImportTree root;

public:
    ImportTreeBuilder(const PackageInfo &package) : pkgMangledName(package.name.mangledName) {}
    ImportTreeBuilder(const ImportTreeBuilder &) = delete;
    ImportTreeBuilder(ImportTreeBuilder &&) = default;
    ImportTreeBuilder &operator=(const ImportTreeBuilder &) = delete;
    ImportTreeBuilder &operator=(ImportTreeBuilder &&) = default;

    void mergeImports(const PackageInfo &importedPackage, const Import &import) {
        for (const auto &exp : importedPackage.exports) {
            if (exp.type == ExportType::Public) {
                addImport(importedPackage, import.name.loc, exp.fqn, import.type);
            }
        }
    }

    // Make add imports for the test package for all normal code exported by its corresponding
    // "normal" package.
    void mergeSelfExportsForTest(const PackageInfo &pkg) {
        for (const auto &exp : pkg.exports) {
            const auto &parts = exp.parts();
            ENFORCE(parts.size() > 0);
            if (parts[0] != TEST_NAME) { // Only add imports for non-test
                auto loc = exp.fqn.loc.offsets();
                addImport(pkg, loc, exp.fqn, ImportType::Test);
            }
        }
    }

    ast::ClassDef::RHS_store makeModule(core::Context ctx, ImportType moduleType) {
        vector<core::NameRef> parts;
        ast::ClassDef::RHS_store modRhs;
        makeModule(ctx, &root, parts, modRhs, moduleType, ImportTree::Source());
        return modRhs;
    }

private:
    void addImport(const PackageInfo &importedPackage, core::LocOffsets loc, const FullyQualifiedName &exportFqn,
                   ImportType importType) {
        ImportTree *node = &root;
        for (auto nameRef : exportFqn.parts) {
            auto &child = node->children[nameRef];
            if (!child) {
                child = make_unique<ImportTree>();
            }
            node = child.get();
        }
        node->source = {importedPackage.name.mangledName, loc, importType};
    }

    void makeModule(core::Context ctx, ImportTree *node, vector<core::NameRef> &parts, ast::ClassDef::RHS_store &modRhs,
                    ImportType moduleType, ImportTree::Source parentSrc) {
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
            // Ignore the entire `Test::*` part of import tree if we are not in a test context.
            if (moduleType != ImportType::Test && parts.empty() && nameRef == TEST_NAME) {
                continue;
            }
            parts.emplace_back(nameRef);
            makeModule(ctx, child, parts, modRhs, moduleType, newParentSrc);
            parts.pop_back();
        }

        if (node->source.exists()) {
            if (parentSrc.exists()) {
                // A conflicting import exist. Only report errors while constructing the test output
                // to avoid duplicate errors because test imports are a superset of normal imports.
                bool isSelfImport = node->source.packageMangledName == pkgMangledName;
                if (moduleType == ImportType::Test && !isSelfImport) {
                    if (auto e = ctx.beginError(node->source.importLoc, core::errors::Packager::ImportConflict)) {
                        // TODO Fix flaky ordering of errors. This is strange...not being done in parallel,
                        // and the file processing order is consistent.
                        e.setHeader("Conflicting import sources for `{}`",
                                    fmt::map_join(parts, "::", [&](const auto &nr) { return nr.show(ctx); }));
                        e.addErrorLine(core::Loc(ctx.file, parentSrc.importLoc), "Conflict from");
                    }
                }
            } else if (moduleType == ImportType::Test || node->source.importType == ImportType::Normal) {
                // Construct a module containing an assignment for an imported name:
                // For name `A::B::C::D` imported from package `A::B` construct:
                // module A::B::C
                //   D = <Mangled A::B>::A::B::C::D
                // end
                auto assignRhs = prependPackageScope(parts2literal(parts, core::LocOffsets::none()),
                                                     node->source.packageMangledName);
                auto assign = ast::MK::Assign(core::LocOffsets::none(), name2Expr(parts.back(), ast::MK::EmptyTree()),
                                              std::move(assignRhs));

                // Ensure import's do not add duplicate loc's in the test_module
                auto importLoc =
                    moduleType == node->source.importType ? node->source.importLoc : core::LocOffsets::none();

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

    ast::ExpressionPtr importModuleName(vector<core::NameRef> &parts, core::LocOffsets importLoc) const {
        ast::ExpressionPtr name = name2Expr(pkgMangledName);
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
    ast::ClassDef::RHS_store testImportedPackages;

    auto package = packageDB.getPackageByFile(ctx, file.file);
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
        for (auto &import : package->importedPackageNames) {
            auto &imported = import.name;
            auto *importedPackage = packageDB.getPackageByMangledName(imported.mangledName);
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
                treeBuilder.mergeImports(*importedPackage, import);
            }
        }

        importedPackages = treeBuilder.makeModule(ctx, ImportType::Normal);

        treeBuilder.mergeSelfExportsForTest(*package);
        testImportedPackages = treeBuilder.makeModule(ctx, ImportType::Test);
    }

    auto packageNamespace =
        ast::MK::Module(core::LocOffsets::none(), core::LocOffsets::none(),
                        name2Expr(core::Names::Constants::PackageRegistry()), {}, std::move(importedPackages));
    auto testPackageNamespace =
        ast::MK::Module(core::LocOffsets::none(), core::LocOffsets::none(),
                        name2Expr(core::Names::Constants::PackageTests()), {}, std::move(testImportedPackages));

    auto &rootKlass = ast::cast_tree_nonnull<ast::ClassDef>(file.tree);
    rootKlass.rhs.emplace_back(move(packageNamespace));
    rootKlass.rhs.emplace_back(move(testPackageNamespace));
    return file;
}

ast::ParsedFile rewritePackagedFile(core::Context ctx, ast::ParsedFile file, core::NameRef packageMangledName,
                                    const PackageInfo *pkg, bool isTestFile) {
    if (ast::isa_tree<ast::EmptyTree>(file.tree)) {
        // Nothing to wrap. This occurs when a file is marked typed: Ignore.
        return file;
    }

    auto &rootKlass = ast::cast_tree_nonnull<ast::ClassDef>(file.tree);
    EnforcePackagePrefix enforcePrefix(pkg, isTestFile);
    file.tree = ast::ShallowMap::apply(ctx, enforcePrefix, move(file.tree));

    auto wrapperName = isTestFile ? core::Names::Constants::PackageTests() : core::Names::Constants::PackageRegistry();
    auto moduleWrapper =
        ast::MK::Module(core::LocOffsets::none(), core::LocOffsets::none(),
                        name2Expr(packageMangledName, name2Expr(wrapperName)), {}, std::move(rootKlass.rhs));
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

vector<ast::ParsedFile> Packager::run(core::GlobalState &gs, WorkerPool &workers, vector<ast::ParsedFile> files,
                                      const vector<std::string> &extraPackageFilesDirectoryPrefixes) {
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
                packageDB.addPackage(ctx, getPackageInfo(ctx, file, extraPackageFilesDirectoryPrefixes));
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
                    auto &file = job.file.data(gs);
                    if (file.sourceType == core::File::Type::Normal) {
                        core::Context ctx(gs, core::Symbols::root(), job.file);
                        if (auto pkg = constPkgDB.getPackageForContext(ctx)) {
                            job = rewritePackagedFile(ctx, move(job), pkg->name.mangledName, pkg, isTestFile(gs, file));
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

vector<ast::ParsedFile> Packager::runIncremental(core::GlobalState &gs, vector<ast::ParsedFile> files,
                                                 const vector<std::string> &extraPackageFilesDirectoryPrefixes) {
    // Just run all packages w/ the changed files through Packager again. It should not define any new names.
    // TODO(jvilk): This incremental pass reprocesses every package file in the project. It should instead only process
    // the packages needed to understand file changes.
    ENFORCE(checkContainsAllPackages(gs, files));
    auto namesUsed = gs.namesUsedTotal();
    auto emptyWorkers = WorkerPool::create(0, gs.tracer());
    files = Packager::run(gs, *emptyWorkers, move(files), extraPackageFilesDirectoryPrefixes);
    ENFORCE(gs.namesUsedTotal() == namesUsed);
    return files;
}

} // namespace sorbet::packager
