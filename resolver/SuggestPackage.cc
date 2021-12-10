#include "resolver/SuggestPackage.h"

#include "absl/strings/str_join.h"
#include "common/formatting.h"
#include "core/core.h"

using namespace std;

namespace sorbet::resolver {
namespace {
class NameFormatter final {
    const core::GlobalState &gs;

public:
    NameFormatter(const core::GlobalState &gs) : gs(gs) {}

    void operator()(std::string *out, core::NameRef name) const {
        out->append(name.shortName(gs));
    }
};

// Add all name parts for a symbol to a vector, exclude internal names used by packager.
void symbol2NameParts(core::Context ctx, core::SymbolRef symbol, vector<core::NameRef> &out) {
    ENFORCE(out.empty());
    while (symbol.exists() && symbol != core::Symbols::root() && !symbol.name(ctx).isPackagerName(ctx)) {
        out.emplace_back(symbol.name(ctx));
        symbol = symbol.owner(ctx);
    }
    std::reverse(out.begin(), out.end());
}

struct PackageMatch {
    core::NameRef mangledName;
    std::optional<core::packages::ImportType> existingImportType;
};

class PackageContext final {
public:
    core::Context ctx;
    const core::packages::PackageInfo &currentPkg;

    PackageContext(core::Context ctx) : ctx(ctx), currentPkg(ctx.state.packageDB().getPackageForFile(ctx, ctx.file)) {}

    const core::packages::PackageDB &db() const {
        return ctx.state.packageDB();
    }

    vector<PackageMatch> findPossibleMissingImports(const ast::ConstantLit::ResolutionScopes &scopes,
                                                    core::NameRef name) const {
        vector<PackageMatch> matches;
        vector<core::NameRef> prefix;
        // Resolution scopes are always longest to shortest. Reserve additional space for the name we're
        // looking for.
        for (auto scope : scopes) {
            prefix.clear();
            symbol2NameParts(ctx, scope, prefix);
            prefix.emplace_back(name);

            findPackagesWithPrefix(prefix, matches);
        }

        // Filter out duplicate imports from suggestions:
        bool isTestFile = ctx.file.data(ctx).isPackagedTest();
        auto it = remove_if(matches.begin(), matches.end(), [&](auto &m) {
            if (m.existingImportType == nullopt) {
                return false;
            }
            switch (*m.existingImportType) {
                case core::packages::ImportType::Normal:
                    return true; // This is already imported
                case core::packages::ImportType::Test:
                    return isTestFile; // This is already test_imported
            }
        });
        matches.erase(it, matches.end());

        return matches;
    }

    void addMissingExportSuggestions(core::ErrorBuilder &e, core::packages::PackageInfo::MissingExportMatch match) {
        if (match.srcPkg == currentPkg.mangledName() && ctx.file.data(ctx).isPackagedTest()) {
            addMissingExportForTestSuggestion(e, match);
            return;
        }
        auto &srcPkg = db().getPackageInfo(match.srcPkg);

        if (auto autocorrect = srcPkg.addExport(ctx, match.symbol, /* isPrivateTestExport */ false)) {
            e.addAutocorrect(std::move(*autocorrect));
        }
        e.addErrorLine(match.symbol.loc(ctx), "Defined here");

        maybeAddErrorHint(e);
    }

    void addMissingExportForTestSuggestion(core::ErrorBuilder &e,
                                           core::packages::PackageInfo::MissingExportMatch match) {
        auto &srcPkg = db().getPackageInfo(match.srcPkg);

        if (auto autocorrect = srcPkg.addExport(ctx, match.symbol, /* isPrivateTestExport */ true)) {
            e.addAutocorrect(std::move(*autocorrect));
        }
        e.addErrorLine(match.symbol.loc(ctx), "Defined here");

        auto extraContext = "To expose a name to a package's own tests it must be exported using export_for_tests.\n";
        maybeAddErrorHint(e, extraContext);
    }

    void addMissingImportSuggestions(core::ErrorBuilder &e, PackageMatch &match) {
        vector<core::ErrorLine> lines;
        auto &otherPkg = db().getPackageInfo(match.mangledName);
        bool isTestFile = ctx.file.data(ctx).isPackagedTest();

        if (auto autocorrect = currentPkg.addImport(ctx, otherPkg, isTestFile)) {
            e.addAutocorrect(std::move(*autocorrect));
        }
        e.addErrorLine(otherPkg.declLoc(), "Defined here");

        maybeAddErrorHint(e);
    }

    bool tryPackageSpecCorrections(core::ErrorBuilder &e, ast::UnresolvedConstantLit &unresolved) {
        if (isUnresolvedExport(unresolved)) {
            tryUnresolvedExportCorrections(e, unresolved);
            return true;
        } else {
            tryUnresolvedImportCorrections(e, unresolved);
            return true;
        }
    }

private:
    // Search all packages finding those that match a specific prefix. These are candidates for
    // includes. See following example for why prefixes and not exact matches are used:
    //
    // Consider the package `Foo::Bar::Baz` that exports the name `Foo::Bar::Baz::Thing`.
    // The following reference to that name from code in the package `Foo::Quuz`:
    //   Foo::Bar::Baz::Thing
    //        ^^^  The resolution error will be for `Bar` not resolving with a resolution scope
    //             `Foo`
    // In this case all we search with the prefix `Foo::Bar` and find that the `Foo::Bar::Baz`
    // package matches.
    void findPackagesWithPrefix(const vector<core::NameRef> &prefix, vector<PackageMatch> &matches) const {
        ENFORCE(!prefix.empty());
        // If the search prefix starts with `Test` ignore it while searching matching package names.
        auto prefixBegin = prefix.begin();
        if (*prefixBegin == core::Names::Constants::Test() && prefix.size() > 1) {
            ++prefixBegin;
        }
        auto prefixSize = prefix.end() - prefixBegin;

        for (auto name : db().packages()) {
            auto &other = db().getPackageInfo(name);
            auto &fullName = other.fullName();
            if (fullName.size() >= prefixSize && canImport(other) &&
                std::equal(fullName.begin(), fullName.begin() + prefixSize, prefixBegin)) {
                matches.emplace_back(PackageMatch{name, currentPkg.importsPackage(other)});
            }
        }
    }

    bool canImport(const core::packages::PackageInfo &other) const {
        return currentPkg.mangledName() != other.mangledName(); // Don't import yourself
    }

    bool isUnresolvedExport(ast::UnresolvedConstantLit &unresolved) {
        if (ast::isa_tree<ast::EmptyTree>(unresolved.scope)) {
            return false;
        }
        // Look for the prefix added by the packager to exports. (Imports are left as-is.)
        return core::packages::PackageInfo::isPackageModule(
            ctx, ast::cast_tree_nonnull<ast::ConstantLit>(unresolved.scope).symbol.asClassOrModuleRef());
    }

    void tryUnresolvedExportCorrections(core::ErrorBuilder &e, ast::UnresolvedConstantLit &unresolved) {
        auto scope = ast::cast_tree_nonnull<ast::ConstantLit>(unresolved.scope).symbol.asClassOrModuleRef();
        auto matches = scope.data(ctx)->findMemberFuzzyMatch(ctx, unresolved.cnst);
        {
            // Remove results defined outside of this package:
            auto it = remove_if(matches.begin(), matches.end(),
                                [&](auto &m) -> bool { return !currentPkg.ownsSymbol(ctx, m.symbol); });
            matches.erase(it, matches.end());
            if (matches.size() > 4) {
                matches.resize(4);
            }
        }
        if (!matches.empty()) {
            addReplacementSuggestions(e, unresolved, matches);
        } else {
            e.addErrorNote("To be exported it must be defined in package `{}`", formatPackageName(currentPkg));
        }
        maybeAddErrorHint(e);
    }

    void tryUnresolvedImportCorrections(core::ErrorBuilder &e, ast::UnresolvedConstantLit &unresolved) {
        auto &scope = unresolved.scope;
        // by default, we'll try to search for a matching package in the root scope
        core::SymbolRef searchScope = core::Symbols::root();
        // but if our parent is something that _was_ resolved, we can
        // search in that scope instead
        if (auto *cnst = ast::cast_tree<ast::ConstantLit>(scope)) {
            searchScope = cnst->symbol;
        }

        // try to find a matching constant in the resolved parent
        // scope (or root scope if that didn't exist)
        auto matches = searchScope.asClassOrModuleRef().data(ctx)->findMemberFuzzyMatch(ctx, unresolved.cnst);
        {
            // remove anything that's not a package. Since we're
            // searching from the root, that means we'll be finding
            // the things which inherit from `PackageSpec`
            // (i.e. something like `::MyPackage`, and not
            // `::<PackageRegistry>::MyPackage`), and we can retain
            // _only_ those constants which inherit from `PackageSpec`
            // and throw out other suggestions
            auto it = remove_if(matches.begin(), matches.end(), [&](auto &m) -> bool {
                if (m.symbol.isClassOrModule()) {
                    return m.symbol.asClassOrModuleRef().data(ctx)->superClass() != core::Symbols::PackageSpec();
                } else {
                    return true;
                }
            });
            matches.erase(it, matches.end());

            // only keep a tractable number of them
            if (matches.size() > 4) {
                matches.resize(4);
            }
        }
        // do the replacements
        if (!matches.empty()) {
            addReplacementSuggestions(e, unresolved, matches);
        }
        maybeAddErrorHint(e);
    }

    void addReplacementSuggestions(core::ErrorBuilder &e, ast::UnresolvedConstantLit &unresolved,
                                   const vector<sorbet::core::ClassOrModule::FuzzySearchResult> &matches) {
        vector<core::ErrorLine> lines;
        for (auto suggestion : matches) {
            const auto replacement = suggestion.symbol.show(ctx);
            lines.emplace_back(core::ErrorLine::from(suggestion.symbol.loc(ctx), "Did you mean: `{}`?", replacement));
            e.replaceWith(fmt::format("Replace with `{}`", replacement), ctx.locAt(unresolved.loc), "{}", replacement);
        }
        e.addErrorSection(core::ErrorSection(lines));
    }

    void maybeAddErrorHint(core::ErrorBuilder &e, string_view extraContext = "") {
        if (db().errorHint().empty()) {
            return;
        }
        e.addErrorNote("{}{}", extraContext, db().errorHint());
    }

    string formatPackageName(const core::packages::PackageInfo &pkg) const {
        return absl::StrJoin(pkg.fullName(), "::", NameFormatter(ctx));
    }
};
} // namespace

bool SuggestPackage::tryPackageCorrections(core::Context ctx, core::ErrorBuilder &e,
                                           const ast::ConstantLit::ResolutionScopes &scopes,
                                           ast::UnresolvedConstantLit &unresolved) {
    // Search strategy:
    // 1. Look for packages that we did not import that match the prefix of the unresolved constant
    //    literal.
    // 2. Look for un-exported names in packages *this package already imports* that defined the
    //    unresolved literal.
    // Both of above look for EXACT matches only. If neither of these find anything we fall back to
    // the resolver's default behavior (Symbol::findMemberFuzzyMatch).
    if (ctx.state.packageDB().empty()) {
        return false;
    }
    ENFORCE(!scopes.empty());
    PackageContext pkgCtx(ctx);
    if (!pkgCtx.currentPkg.exists()) {
        return false; // This error is not in packaged code. Nothing to do here.
    } else if (ctx.file.data(ctx).isPackage()) {
        return pkgCtx.tryPackageSpecCorrections(e, unresolved);
    }

    vector<PackageMatch> missingImports;
    if (auto unresScope = ast::cast_tree<ast::ConstantLit>(unresolved.scope)) {
        missingImports = pkgCtx.findPossibleMissingImports({unresScope->symbol}, unresolved.cnst);
    } else {
        missingImports = pkgCtx.findPossibleMissingImports(scopes, unresolved.cnst);
    }
    if (!missingImports.empty()) {
        if (missingImports.size() > 3) {
            missingImports.resize(3);
        }
        for (auto match : missingImports) {
            pkgCtx.addMissingImportSuggestions(e, match);
        }
        return true;
    }

    if (ast::isa_tree<ast::ConstantLit>(unresolved.scope)) {
        auto missingExports = pkgCtx.currentPkg.findMissingExports(
            ctx, ast::cast_tree_nonnull<ast::ConstantLit>(unresolved.scope).symbol, unresolved.cnst);
        if (missingExports.size() > 3) {
            missingExports.resize(3);
        }
        if (!missingExports.empty()) {
            for (auto match : missingExports) {
                pkgCtx.addMissingExportSuggestions(e, match);
            }
            return true;
        }
    }
    return false;
}
} // namespace sorbet::resolver
