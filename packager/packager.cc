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
#include "core/packages/PackageInfo.h"
#include <sys/stat.h>

using namespace std;

namespace sorbet::packager {
namespace {

constexpr string_view PACKAGE_FILE_NAME = "__package.rb"sv;
constexpr core::NameRef TEST_NAME = core::Names::Constants::Test();

bool isTestFile(const core::GlobalState &gs, core::File &file) {
    // TODO: (aadi-stripe, 11/26/2021) see if these can all be changed to use getPrintablePath
    return absl::EndsWith(file.path(), ".test.rb") || absl::StartsWith(file.path(), "./test/") ||
           absl::StrContains(gs.getPrintablePath(file.path()), "/test/");
}

bool isPrimaryTestNamespace(const core::NameRef ns) {
    return ns == TEST_NAME;
}

bool isSecondaryTestNamespace(const core::GlobalState &gs, const core::NameRef ns) {
    const vector<core::NameRef> &secondaryTestPackageNamespaceRefs = gs.packageDB().secondaryTestPackageNamespaceRefs();
    return absl::c_find(secondaryTestPackageNamespaceRefs, ns) != secondaryTestPackageNamespaceRefs.end();
}

bool isTestNamespace(const core::GlobalState &gs, const core::NameRef ns) {
    return isPrimaryTestNamespace(ns) || isSecondaryTestNamespace(gs, ns);
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

    bool isSuffix(const FullyQualifiedName &prefix) const {
        if (prefix.parts.size() >= parts.size()) {
            return false;
        }

        return std::equal(prefix.parts.begin(), prefix.parts.end(), parts.begin());
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

    // "self-import": This represents code that is exported by a package, and is therefore "imported" (i.e. remapped)
    // into its private test namespace and its public->private mapping.
    Self,
};

// There are 4 kinds of virtual modules inserted into the AST.
// Normal: suffixed with _Package_Private, it maps external imports of a package into its namespace.
// Test: suffixed with _Package_Private, it maps external imports, exports and self-test exports of a package into its
//   test namespace.
// ExportMappingNormal: suffixed with _Package, it maps exports of a package into its publicly available namespace (this
//   is used by other packages when they import this package).
// ExportMappingTest: suffixed with _Package, it maps exports of a package that are in the "Test::" namespace into its
//   publicly available test namespace (this is used by other packages when they import this package).
//
// A package cannot access an external package's _Package_Private module, but a package's private module can access
// an external package's public interface (which is the public _Package module).
enum class ModuleType { Normal, Test, ExportMappingNormal, ExportMappingTest };

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

class PackageInfoImpl final : public sorbet::core::packages::PackageInfo {
public:
    core::NameRef mangledName() const {
        return name.mangledName;
    }

    const std::vector<std::string> &pathPrefixes() const {
        return packagePathPrefixes;
    }

    core::Loc definitionLoc() const {
        return loc;
    }

    // The possible path prefixes associated with files in the package, including path separator at end.
    vector<std::string> packagePathPrefixes;
    PackageName name;

    // Private namespace of the package. We do not put this in the PackageName struct above because the former
    // is used to not only hold package name information in PackageInfoImpl, but also created for every imported
    // package during import enumeration. In that context, the private name is unnecessary and would
    // take up an extra reference, which would contribute O(packages * imports) in space. Putting
    // it here ensures only O(packages) space complexity.
    core::NameRef privateMangledName;

    // loc for the package definition. Used for error messages.
    core::Loc loc;
    // The names of each package imported by this package.
    vector<Import> importedPackageNames;
    // List of exported items that form the body of this package's public API.
    // These are copied into every package that imports this package.
    vector<Export> exports;

    // PackageInfoImpl is the only implementation of PackageInfoImpl
    const static PackageInfoImpl &from(const sorbet::core::packages::PackageInfo &pkg) {
        ENFORCE(pkg.exists());
        return reinterpret_cast<const PackageInfoImpl &>(pkg); // TODO is there a more idiomatic way to do this?
    }

    static PackageInfoImpl &from(sorbet::core::packages::PackageInfo &pkg) {
        ENFORCE(pkg.exists());
        return reinterpret_cast<PackageInfoImpl &>(pkg); // TODO is there a more idiomatic way to do this?
    }

    unique_ptr<PackageInfo> deepCopy() const {
        return make_unique<PackageInfoImpl>(*this);
    }

    PackageInfoImpl() = default;
    explicit PackageInfoImpl(const PackageInfoImpl &) = default;
    PackageInfoImpl &operator=(const PackageInfoImpl &) = delete;
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
    auto mangledName = absl::StrCat(absl::StrJoin(pName.fullName.parts, "_", NameFormatter(ctx)), core::PACKAGE_SUFFIX);
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
ast::ExpressionPtr prependPackageScope(const core::GlobalState &gs, ast::ExpressionPtr scope,
                                       core::NameRef mangledName) {
    auto *lastConstLit = &ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(scope);
    while (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(lastConstLit->scope)) {
        lastConstLit = constLit;
    }
    core::NameRef registryName = core::Names::Constants::PackageRegistry();
    if (isTestNamespace(gs, lastConstLit->cnst)) {
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
    const PackageInfoImpl &pkg;
    const bool isTestFile;
    vector<core::NameRef> nameParts;
    int rootConsts = 0;
    int skipPush = 0;

public:
    EnforcePackagePrefix(const PackageInfoImpl &pkg, bool isTestFile) : pkg(pkg), isTestFile(isTestFile) {
        ENFORCE(pkg.exists());
    }

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root()) {
            // Ignore top-level <root>
            return tree;
        }

        if (nameParts.size() > 0 && nameParts.size() > requiredNamespace(ctx.state).size()) {
            // At this depth we can stop checking the prefixes since beyond the end of the prefix.
            skipPush++;
            return tree;
        }

        auto &constantLit = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(classDef.name);
        pushConstantLit(&constantLit);
        auto &pkgName = requiredNamespace(ctx.state);

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

        if (lhs != nullptr && rootConsts == 0) {
            if (nameParts.size() == 0 || nameParts.size() < requiredNamespace(ctx.state).size()) {
                pushConstantLit(lhs);
                auto &pkgName = requiredNamespace(ctx.state);

                if (rootConsts == 0 && !isPrefix(pkgName, nameParts)) {
                    if (auto e = ctx.beginError(lhs->loc, core::errors::Packager::DefinitionPackageMismatch)) {
                        e.setHeader("Constants may not be defined outside of the enclosing package namespace `{}`",
                                    fmt::map_join(pkgName.begin(), pkgName.end(),
                                                  "::", [&](const auto &nr) { return nr.show(ctx); }));
                    }
                }

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

    const vector<core::NameRef> &requiredNamespace(const core::GlobalState &gs) const {
        if (isTestFile) {
            if (!isPrimaryTestNamespace(nameParts[0]) && isSecondaryTestNamespace(gs, pkg.name.fullName.parts[0])) {
                return pkg.name.fullName.parts;
            }
            return pkg.name.fullTestPkgName.parts;
        } else {
            return pkg.name.fullName.parts;
        }
    }
};

struct PackageInfoFinder {
    unique_ptr<PackageInfoImpl> info = nullptr;
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
                send.args[0] = prependInternalPackageName(ctx, move(send.args[0]));
            }
        }
        if (send.fun == core::Names::export_for_test() && send.args.size() == 1) {
            // null indicates an invalid export.
            if (auto target = verifyConstant(ctx, core::Names::export_for_test(), send.args[0])) {
                auto fqn = getFullyQualifiedName(ctx, target);
                ENFORCE(fqn.parts.size() > 0);
                if (isTestNamespace(ctx.state, fqn.parts[0])) {
                    if (auto e = ctx.beginError(target->loc, core::errors::Packager::InvalidExportForTest)) {
                        e.setHeader("Packages may not {} names in the `{}::` namespace", send.fun.toString(ctx),
                                    fqn.parts[0].show(ctx));
                    }
                } else {
                    exported.emplace_back(move(fqn), ExportType::PrivateTest);
                }
                // Transform the constant lit to refer to the target within the mangled package namespace.
                send.args[0] = prependInternalPackageName(ctx, move(send.args[0]));
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
            info = make_unique<PackageInfoImpl>();
            checkPackageName(ctx, nameTree);
            info->name = getPackageName(ctx, nameTree);

            // Append _Private at the end of the mangled package name to get the name of its private namespace.
            auto utf8PrivateName =
                ctx.state.enterNameUTF8(absl::StrCat(info->name.mangledName.show(ctx.state), "_Private"));
            auto packagerPrivateName =
                ctx.state.freshNameUnique(core::UniqueNameKind::PackagerPrivate, utf8PrivateName, 1);
            info->privateMangledName = ctx.state.enterNameConstant(packagerPrivateName);

            info->loc = core::Loc(ctx.file, classDef.loc);
        } else {
            if (auto e = ctx.beginError(classDef.loc, core::errors::Packager::MultiplePackagesInOneFile)) {
                e.setHeader("Package files can only declare one package");
                e.addErrorLine(info->loc, "Previous package declaration found here");
            }
        }

        return tree;
    }

    // Bar::Baz => <PackageRegistry>::Foo_Package_Private::Bar::Baz
    ast::ExpressionPtr prependInternalPackageName(const core::GlobalState &gs, ast::ExpressionPtr scope) {
        return prependPackageScope(gs, move(scope), this->info->privateMangledName);
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
                if (std::equal(shorter->parts().begin(), shorter->parts().end(), longer->parts().begin()) &&
                    !allowedExportPrefix(ctx, *shorter, *longer)) {
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

    bool allowedExportPrefix(core::Context ctx, const Export &shorter, const Export &longer) {
        // Permits:
        //   export Foo::Bar::Baz
        //   export_for_test Foo::Bar
        return shorter.type == ExportType::PrivateTest && longer.type == ExportType::Public &&
               shorter.fqn.loc.file() == longer.fqn.loc.file() && ctx.file == shorter.fqn.loc.file();
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

// Sanity checks package files, mutates arguments to export / export_methods to point to item in namespace,
// builds up the expression injected into packages that import the package, and codegens the <PackagedMethods>  module.
unique_ptr<PackageInfoImpl> getPackageInfo(core::MutableContext ctx, ast::ParsedFile &package,
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
        const string_view dirNameFromShortName = shortName.substr(0, shortName.find(core::PACKAGE_SUFFIX));

        for (const string &prefix : extraPackageFilesDirectoryPrefixes) {
            string additionalDirPath = absl::StrCat(prefix, dirNameFromShortName, "/");
            finder.info->packagePathPrefixes.emplace_back(std::move(additionalDirPath));
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

        // This bit is set to true if the import is added as part of a fully-enumerated set
        // of exports from a package.
        bool isEnumeratedImport;

        bool exists() {
            return importLoc.exists();
        }

        bool isSelfImport() {
            return importType == ImportType::Self;
        }

        bool isTestImport() {
            return importType == ImportType::Test;
        }

        bool isNormalImport() {
            return importType == ImportType::Normal;
        }

        bool isSelfOrTestImport() {
            return importType == ImportType::Self || importType == ImportType::Test;
        }

        bool skipBuildMappingFor(ModuleType moduleType) {
            // Don't build mappings in the export mapping modules for external imports.
            if (!isSelfImport() &&
                (moduleType == ModuleType::ExportMappingNormal || moduleType == ModuleType::ExportMappingTest)) {
                return true;
            }

            // Don't build mappings in the main (normal) package module for internal imports.
            if (isSelfImport() && moduleType == ModuleType::Normal) {
                return true;
            }

            return false;
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
    // PackageInfoImpl package; // The package we are building an import tree for.
    core::NameRef pkgMangledName;
    core::NameRef privatePkgMangledName;
    ImportTree root;

public:
    ImportTreeBuilder(const PackageInfoImpl &package)
        : pkgMangledName(package.name.mangledName), privatePkgMangledName(package.privateMangledName) {}
    ImportTreeBuilder(const ImportTreeBuilder &) = delete;
    ImportTreeBuilder(ImportTreeBuilder &&) = default;
    ImportTreeBuilder &operator=(const ImportTreeBuilder &) = delete;
    ImportTreeBuilder &operator=(ImportTreeBuilder &&) = default;

    // Add the imports of a package into the import tree. These are used for building the Normal and Test modules
    // in the package's internal namespace.
    void mergeImports(core::Context ctx, const PackageInfoImpl &package) {
        const auto &packageDB = ctx.state.packageDB();
        UnorderedMap<core::NameRef, core::LocOffsets> importedNames;
        for (auto &import : package.importedPackageNames) {
            auto &imported = import.name;
            auto &importedPackage = packageDB.getPackageInfo(imported.mangledName);
            if (!importedPackage.exists()) {
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

                continue;
            }

            importedNames[imported.mangledName] = imported.loc;

            // TODO (aadi-stripe): fix this so it's not quadratic (if it ends up causing runtime bloat). Currently
            // (11/8/2021) timing analysis suggests that it doesn't.
            //
            // Determine whether the current package either imports a suffix of the imported name, or itself is a
            // suffix of the imported name. Based on this, we determine whether to enumerate and alias all exports
            // from an imported package or only alias the top-level export.
            //
            // This approach saves memory by adding package aliases in the common case, falling back on naming a whole
            // package tree only when a package and subpackage of it are both imported.

            const bool isOrImportsSubpackage =
                absl::c_any_of(package.importedPackageNames,
                               [&](const auto &otherImport) -> bool {
                                   return otherImport.name.fullName.isSuffix(imported.fullName);
                               }) ||
                package.name.fullName.isSuffix(imported.fullName);
            if (isOrImportsSubpackage) {
                mergeAllExportsFromImportedPackage(ctx, PackageInfoImpl::from(importedPackage), import);
            } else {
                mergeTopLevelExportFromImportedPackage(ctx, PackageInfoImpl::from(importedPackage), import);
            }
        }
    }

    // Add the exports of a package as "self-imports" into the import tree. These are used for building the package's
    // Test, ExportMappingNormal and ExportMappingTest modules.
    void mergeSelfExports(core::Context ctx, const PackageInfoImpl &pkg, ExportType type) {
        for (const auto &exp : pkg.exports) {
            if (exp.type != type) {
                continue;
            }

            const auto &parts = exp.parts();
            ENFORCE(parts.size() > 0);
            auto loc = exp.fqn.loc.offsets();
            addImport(ctx, pkg, loc, exp.fqn, ImportType::Self, true);
        }
    }

    ast::ClassDef::RHS_store makeModule(core::Context ctx, ModuleType moduleType) {
        vector<core::NameRef> parts;
        ast::ClassDef::RHS_store modRhs;
        makeModule(ctx, &root, parts, modRhs, moduleType, ImportTree::Source());
        return modRhs;
    }

private:
    const bool isNonTestModule(ModuleType moduleType) const {
        return moduleType == ModuleType::ExportMappingNormal || moduleType == ModuleType::Normal;
    }

    const bool isExportMappingModule(ModuleType moduleType) const {
        return moduleType == ModuleType::ExportMappingNormal || moduleType == ModuleType::ExportMappingTest;
    }

    // Enumerate and add all exported names from an imported package into the import tree
    void mergeAllExportsFromImportedPackage(core::Context ctx, const PackageInfoImpl &importedPackage,
                                            const Import &import) {
        for (const auto &exp : importedPackage.exports) {
            if (exp.type == ExportType::Public) {
                addImport(ctx, importedPackage, import.name.loc, exp.fqn, import.type, true);
            }
        }
    }

    // Add the entire top-level exported namespaces of an imported package into the import tree
    void mergeTopLevelExportFromImportedPackage(core::Context ctx, const PackageInfoImpl &importedPackage,
                                                const Import &import) {
        const bool exportsTestConstant = absl::c_any_of(importedPackage.exports, [&](const auto &exp) -> bool {
            return exp.type == ExportType::Public && isPrimaryTestNamespace(exp.fqn.parts[0]);
        });
        const bool exportsRealConstant = absl::c_any_of(importedPackage.exports, [&](const auto &exp) -> bool {
            return exp.type == ExportType::Public && !isPrimaryTestNamespace(exp.fqn.parts[0]);
        });

        // If a Test:: constant is publicly exported, we add the top-level test package namespace.
        if (exportsTestConstant) {
            addImport(ctx, importedPackage, import.name.loc, import.name.fullTestPkgName, import.type, false);
        }

        // If a non-test constant is publicly exported, we add the top-level package namespace.
        if (exportsRealConstant) {
            addImport(ctx, importedPackage, import.name.loc, import.name.fullName, import.type, false);
        }
    }

    // Add an individual imported name into the import tree.
    void addImport(core::Context ctx, const PackageInfoImpl &importedPackage, core::LocOffsets loc,
                   const FullyQualifiedName &exportFqn, ImportType importType, bool isEnumeratedImport) {
        ImportTree *node = &root;
        for (auto nameRef : exportFqn.parts) {
            auto &child = node->children[nameRef];
            if (!child) {
                child = make_unique<ImportTree>();
            }
            node = child.get();
        }

        // If the node already has an import source, this is a conflicting import.
        // See test/cli/package-import-conflicts/ for an example.
        if (node->source.exists() && importType != ImportType::Self) {
            addImportConflictError(ctx, loc, node->source.importLoc, exportFqn.parts);
        }
        node->source = ImportTree::Source{importedPackage.name.mangledName, loc, importType, isEnumeratedImport};
    }

    // Method that makes a wrapper module for an import, of the form:
    // module PkgMangledName::PkgName
    //   Import = ImportedPkgMangledName::ImportedPkgName::Export
    // end
    //
    // This method in fact creates a wrapper module recursively for the entire import tree of a package.
    void makeModule(core::Context ctx, ImportTree *node, vector<core::NameRef> &parts, ast::ClassDef::RHS_store &modRhs,
                    ModuleType moduleType, ImportTree::Source parentSrc) {
        auto newParentSrc = parentSrc;
        auto &source = node->source;
        if (source.exists() && !parentSrc.exists()) {
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
            if (parts.empty()) {
                // Ignore the entire `Test::*` part of import tree if we are not in a test context.
                if (isNonTestModule(moduleType) && isTestNamespace(ctx, nameRef)) {
                    continue;
                }

                // Ignore non-test constants for the test export mapping module.
                if (moduleType == ModuleType::ExportMappingTest && !isTestNamespace(ctx, nameRef)) {
                    continue;
                }
            }

            parts.emplace_back(nameRef);
            makeModule(ctx, child, parts, modRhs, moduleType, newParentSrc);
            parts.pop_back();
        }

        if (source.exists()) {
            // If we do not need to map the given import for the given module type, return
            if (source.skipBuildMappingFor(moduleType)) {
                return;
            }

            const bool isSelfImport = source.isSelfImport();
            // For the test module, we do not need to map self-imports (i.e. exports from the current package) that
            // begin with "Test::".
            if (moduleType == ModuleType::Test && isSelfImport && isTestNamespace(ctx, parts[0])) {
                return;
            }

            if (parentSrc.exists()) {
                // A conflicting import exists. Only report errors while constructing the test output
                // to avoid duplicate errors because test imports are a superset of normal imports.
                if (moduleType == ModuleType::Test && !isSelfImport) {
                    addImportConflictError(ctx, source.importLoc, parentSrc.importLoc, parts);
                }

                return;
            }

            if (moduleType != ModuleType::Normal || source.isNormalImport()) {
                // Construct a module containing an assignment for an imported name:
                // For name `A::B::C::D` imported from package `A::B` construct:
                // module A::B::C
                //   D = <Mangled A::B>::A::B::C::D
                // end
                const auto &sourceMangledName = isSelfImport ? privatePkgMangledName : source.packageMangledName;
                auto assignRhs =
                    prependPackageScope(ctx, parts2literal(parts, core::LocOffsets::none()), sourceMangledName);

                auto assign = ast::MK::Assign(core::LocOffsets::none(), name2Expr(parts.back(), ast::MK::EmptyTree()),
                                              std::move(assignRhs));

                ast::ClassDef::RHS_store rhs;
                rhs.emplace_back(std::move(assign));

                // Use the loc from the import in the module name and declaration to get the
                // following jump to definition behavior in the case of enumerated imports:
                // imported constant: `Foo::Bar::Baz` from package `Foo::Bar`
                //                     ^^^^^^^^       jump to the import statement
                //                               ^^^  jump to actual definition of `Baz` class
                //
                // In the case of un-enumerated imports, we don't use the loc at the import site,
                // but effectively use the one from the export site (due to the export mapping module).
                // This results in the following behavior:
                // imported constant: `Foo::Bar::Baz` from package `Foo::Bar`
                //                     ^^^^^^^^       jump to package file of `Foo::Bar`
                //                               ^^^  jump to actual definition of `Baz` class

                // Ensure import's do not add duplicate loc-s in the test_module
                const bool useImportLoc =
                    source.isEnumeratedImport && (moduleType != ModuleType::Test || source.isSelfOrTestImport());
                const auto &importLoc = useImportLoc ? source.importLoc : core::LocOffsets::none();

                auto mod = ast::MK::Module(core::LocOffsets::none(), importLoc,
                                           importModuleName(parts, importLoc, moduleType), {}, std::move(rhs));
                modRhs.emplace_back(std::move(mod));
            }
        }
    }

    // Create an error that represents an import conflict; namely a package importing two names where one is a
    // prefix of another. This is disallowed, as it would (rightly) cause a redefinition error in the namer pass.
    void addImportConflictError(core::Context ctx, const core::LocOffsets &loc, const core::LocOffsets &otherLoc,
                                const vector<core::NameRef> &nameParts) {
        if (auto e = ctx.beginError(loc, core::errors::Packager::ImportConflict)) {
            // TODO Fix flaky ordering of errors. This is strange...not being done in parallel,
            // and the file processing order is consistent.
            e.setHeader("Conflicting import sources for `{}`",
                        fmt::map_join(nameParts, "::", [&](const auto &nr) { return nr.show(ctx); }));
            e.addErrorLine(core::Loc(ctx.file, otherLoc), "Conflict from");
        }
    }

    // Name of the wrapper module for a given import, prefixed with the mangled name of the package.
    ast::ExpressionPtr importModuleName(vector<core::NameRef> &parts, core::LocOffsets importLoc,
                                        ModuleType moduleType) const {
        // Export mapping modules are built in the public (_Package) namespace, whereas other modules are built in
        // the private (_Package_Private) namespace.
        ast::ExpressionPtr name =
            isExportMappingModule(moduleType) ? name2Expr(pkgMangledName) : name2Expr(privatePkgMangledName);
        for (auto part = parts.begin(); part < parts.end() - 1; part++) {
            name = name2Expr(*part, move(name));
        }
        // Put the loc on the outer name:
        auto &lit = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(name);
        return ast::MK::UnresolvedConstant(importLoc, move(lit.scope), lit.cnst);
    }
};

// Given a packaged file, wrap it in the _Package_Private mangled namespace of its package.
ast::ParsedFile wrapFileInPackageModule(core::Context ctx, ast::ParsedFile file, core::NameRef packageMangledName,
                                        const PackageInfoImpl &pkg, bool isTestFile) {
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

// Add:
//    module <PackageRegistry>::Mangled_Name_Package_Private
//      module A::B::C
//        D = Mangled_Imported_Package::A::B::C::D
//      end
//      ...
//    end
//
//    for external imports, and
//
//    module <PackageRegistry>::Mangled_Name_Package
//      module F
//        G = Mangled_Name_Package_Private::G
//      end
//      ...
//    end
//
// ...for self-mapping, to __package.rb files to set up the package namespace.
ast::ParsedFile rewritePackage(core::Context ctx, ast::ParsedFile file) {
    ast::ClassDef::RHS_store importedPackages;
    ast::ClassDef::RHS_store testImportedPackages;
    ast::ClassDef::RHS_store exportMappingNormal;
    ast::ClassDef::RHS_store exportMappingTest;

    const auto &packageDB = ctx.state.packageDB();
    auto &absPkg = packageDB.getPackageForFile(ctx, file.file);
    if (!absPkg.exists()) {
        // We already produced an error on this package when producing its package info.
        // The correct course of action is to abort the transform.
        return file;
    }
    auto &package = PackageInfoImpl::from(absPkg);

    // Sanity check: __package.rb files _must_ be typed: strict
    if (file.file.data(ctx).originalSigil < core::StrictLevel::Strict) {
        if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::PackageFileMustBeStrict)) {
            e.setHeader("Package files must be at least `{}`", "# typed: strict");
        }
    }

    {
        ImportTreeBuilder treeBuilder(package);

        // Merge public exports of this package into tree builder in order to "self-map" them
        // from the package's private module to its public-facing module
        treeBuilder.mergeSelfExports(ctx, package, ExportType::Public);
        exportMappingNormal = treeBuilder.makeModule(ctx, ModuleType::ExportMappingNormal);
        exportMappingTest = treeBuilder.makeModule(ctx, ModuleType::ExportMappingTest);

        // Merge imports of package into tree builder in order to map external package modules
        // into this package's private module
        treeBuilder.mergeImports(ctx, package);
        importedPackages = treeBuilder.makeModule(ctx, ModuleType::Normal);

        // Merge self-test exports package into tree builder in order to "self-map" them (in addition
        // to the public exports and imports of this package) into the package's private test module
        treeBuilder.mergeSelfExports(ctx, package, ExportType::PrivateTest);
        testImportedPackages = treeBuilder.makeModule(ctx, ModuleType::Test);
    }

    // Create wrapper modules
    auto packageNamespace =
        ast::MK::Module(core::LocOffsets::none(), core::LocOffsets::none(),
                        name2Expr(core::Names::Constants::PackageRegistry()), {}, std::move(importedPackages));
    auto testPackageNamespace =
        ast::MK::Module(core::LocOffsets::none(), core::LocOffsets::none(),
                        name2Expr(core::Names::Constants::PackageTests()), {}, std::move(testImportedPackages));
    auto exportMappingNormalNamespace =
        ast::MK::Module(core::LocOffsets::none(), core::LocOffsets::none(),
                        name2Expr(core::Names::Constants::PackageRegistry()), {}, std::move(exportMappingNormal));
    auto exportMappingTestNamespace =
        ast::MK::Module(core::LocOffsets::none(), core::LocOffsets::none(),
                        name2Expr(core::Names::Constants::PackageTests()), {}, std::move(exportMappingTest));

    // Add wrapper modules to root of the tree
    auto &rootKlass = ast::cast_tree_nonnull<ast::ClassDef>(file.tree);
    rootKlass.rhs.emplace_back(move(packageNamespace));
    rootKlass.rhs.emplace_back(move(testPackageNamespace));
    rootKlass.rhs.emplace_back(move(exportMappingNormalNamespace));
    rootKlass.rhs.emplace_back(move(exportMappingTestNamespace));

    return file;
}

ast::ParsedFile rewritePackagedFile(core::GlobalState &gs, ast::ParsedFile parsedFile) {
    auto &file = parsedFile.file.data(gs);
    ENFORCE(file.sourceType != core::File::Type::Package);
    core::Context ctx(gs, core::Symbols::root(), parsedFile.file);
    auto &pkg = gs.packageDB().getPackageForFile(ctx, ctx.file);
    if (pkg.exists()) {
        auto &pkgImpl = PackageInfoImpl::from(pkg);

        // Wrap the file in a package module (ending with _Package_Private) to put it by default in the package's
        // private namespace (private-by-default paradigm).
        parsedFile =
            wrapFileInPackageModule(ctx, move(parsedFile), pkgImpl.privateMangledName, pkgImpl, isTestFile(gs, file));
    } else {
        // Don't transform, but raise an error on the first line.
        if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::UnpackagedFile)) {
            e.setHeader("File `{}` does not belong to a package; add a `{}` file to one "
                        "of its parent directories",
                        ctx.file.data(gs).path(), "__package.rb");
        }
    }
    return parsedFile;
}

// Re-write source files to be in packages. This is only called if no package definitions were
// changed.
vector<ast::ParsedFile> rewritePackagedFilesFast(core::GlobalState &gs, vector<ast::ParsedFile> files) {
    Timer timeit(gs.tracer(), "packager.rewritePackagedFilesFast");
    for (auto i = 0; i < files.size(); i++) {
        files[i] = rewritePackagedFile(gs, move(files[i]));
    }
    return files;
}

vector<ast::ParsedFile> Packager::run(core::GlobalState &gs, WorkerPool &workers, vector<ast::ParsedFile> files) {
    Timer timeit(gs.tracer(), "packager");
    // Ensure files are in canonical order.
    fast_sort(files, [](const auto &a, const auto &b) -> bool { return a.file < b.file; });

    // Step 1: Find packages and determine their imports/exports.
    {
        Timer timeit(gs.tracer(), "packager.findPackages");
        core::UnfreezeNameTable unfreeze(gs);
        core::packages::UnfreezePackages packages = gs.unfreezePackages();
        for (auto &file : files) {
            if (FileOps::getFileName(file.file.data(gs).path()) == PACKAGE_FILE_NAME) {
                file.file.data(gs).sourceType = core::File::Type::Package;
                core::MutableContext ctx(gs, core::Symbols::root(), file.file);
                auto pkg = getPackageInfo(ctx, file, gs.packageDB().extraPackageFilesDirectoryPrefixes());
                if (pkg == nullptr) {
                    // There was an error creating a PackageInfoImpl for this file, and getPackageInfo has already
                    // surfaced that error to the user. Nothing to do here.
                    continue;
                }
                auto &prevPgk = gs.packageDB().getPackageInfo(pkg->mangledName());
                if (prevPgk.exists() && prevPgk.definitionLoc() != pkg->definitionLoc()) {
                    if (auto e = ctx.beginError(pkg->loc.offsets(), core::errors::Packager::RedefinitionOfPackage)) {
                        auto pkgName = pkg->name.toString(ctx);
                        e.setHeader("Redefinition of package `{}`", pkgName);
                        auto &prevPkg = gs.packageDB().getPackageInfo(pkg->mangledName());
                        e.addErrorLine(prevPkg.definitionLoc(), "Package `{}` originally defined here", pkgName);
                    }
                } else {
                    packages.db.enterPackage(move(pkg));
                }
            }
        }
    }

    {
        Timer timeit(gs.tracer(), "packager.rewritePackages");
        // Step 2: Rewrite packages. Can be done in parallel (and w/ step 3) if this becomes a bottleneck.
        for (auto &file : files) {
            if (file.file.data(gs).sourceType == core::File::Type::Package) {
                core::Context ctx(gs, core::Symbols::root(), file.file);
                file = rewritePackage(ctx, move(file));
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

        workers.multiplexJob("rewritePackagedFiles", [&gs, fileq, resultq]() {
            Timer timeit(gs.tracer(), "packager.rewritePackagedFilesWorker");
            vector<ast::ParsedFile> results;
            u4 filesProcessed = 0;
            ast::ParsedFile job;
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    filesProcessed++;
                    auto &file = job.file.data(gs);
                    if (file.sourceType == core::File::Type::Normal) {
                        job = rewritePackagedFile(gs, move(job));
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
    // However, if only source files have changed the existing PackageDB can be used to re-process
    // the changed files only.
    // TODO(nroman-stripe) This could be further incrementalized to avoid processing all packages by
    // building in an understanding of the dependencies between packages.
    auto namesUsed = gs.namesUsedTotal();
    bool packageDefChanged = absl::c_any_of(
        files, [&gs](const auto &pf) -> bool { return pf.file.data(gs).sourceType == core::File::Type::Package; });
    if (packageDefChanged) {
        ENFORCE(checkContainsAllPackages(gs, files));
        auto emptyWorkers = WorkerPool::create(0, gs.tracer());
        files = Packager::run(gs, *emptyWorkers, move(files));
    } else {
        files = rewritePackagedFilesFast(gs, move(files));
    }
    ENFORCE(gs.namesUsedTotal() == namesUsed);
    return files;
}

} // namespace sorbet::packager
