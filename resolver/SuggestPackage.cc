#include "resolver/SuggestPackage.h"

#include "common/formatting.h"
#include "core/core.h"

using namespace std;

namespace sorbet::resolver {
namespace {
// Add all name parts for a symbol to a vector, exclude internal names used by packager.
void symbol2NameParts(core::Context ctx, core::SymbolRef symbol, vector<core::NameRef> &out) {
    ENFORCE(out.empty());
    while (symbol.exists() && symbol != core::Symbols::root() && !symbol.data(ctx)->name.isPackagerName(ctx)) {
        out.emplace_back(symbol.data(ctx)->name);
        symbol = symbol.data(ctx)->owner;
    }
    std::reverse(out.begin(), out.end());
}

struct PackageMatch {
    core::NameRef mangledName;
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
        return matches;
    }

    void addMissingImportSuggestions(core::ErrorBuilder &e, PackageMatch &match) {
        vector<core::ErrorLine> lines;
        auto &otherPkg = db().getPackageInfo(match.mangledName);
        auto importName = core::packages::PackageDB::isTestFile(ctx, ctx.file.data(ctx)) ? core::Names::test_import()
                                                                                         : core::Names::import();
        lines.emplace_back(core::ErrorLine::from(
            otherPkg.definitionLoc(), "Do you need to `{}` package `{}`?", importName.show(ctx),
            fmt::map_join(otherPkg.fullName(), "::", [&](auto nr) -> string { return nr.show(ctx); })));
        // TODO(nroman-stripe) Add automatic fixers
        e.addErrorSection(core::ErrorSection(lines));
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
                matches.emplace_back(PackageMatch{name});
            }
        }
    }

    bool canImport(const core::packages::PackageInfo &other) const {
        return currentPkg.mangledName() != other.mangledName(); // Don't import yourself
    }
};
} // namespace

bool SuggestPackage::tryPackageCorrections(core::Context ctx, core::ErrorBuilder &e,
                                           const ast::ConstantLit::ResolutionScopes &scopes, core::NameRef name) {
    if (ctx.state.packageDB().empty()) {
        return false;
    }
    ENFORCE(!scopes.empty());
    PackageContext pkgCtx(ctx);
    if (!pkgCtx.currentPkg.exists()) {
        return false; // This error is not in packaged code. Nothing to do here.
    }

    // TODO(nroman-stripe) First search for missing exports.

    vector<PackageMatch> missingImports = pkgCtx.findPossibleMissingImports(scopes, name);
    if (missingImports.empty()) {
        return false;
    }
    if (missingImports.size() > 5) {
        missingImports.resize(5);
    }
    for (auto match : missingImports) {
        pkgCtx.addMissingImportSuggestions(e, match);
    }
    return true;
}
} // namespace sorbet::resolver
