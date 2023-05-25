#include "packager/packager.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/synchronization/blocking_counter.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "common/FileOps.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/concurrency/WorkerPool.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "core/AutocorrectSuggestion.h"
#include "core/Unfreeze.h"
#include "core/errors/packager.h"
#include "core/packages/MangledName.h"
#include "core/packages/PackageInfo.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <sys/stat.h>

using namespace std;

namespace sorbet::packager {
namespace {

constexpr string_view PACKAGE_FILE_NAME = "__package.rb"sv;

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

struct PackageName {
    core::LocOffsets loc;
    core::NameRef mangledName = core::NameRef::noName();
    FullyQualifiedName fullName;
    FullyQualifiedName fullTestPkgName;

    // Pretty print the package's (user-observable) name (e.g. Foo::Bar)
    string toString(const core::GlobalState &gs) const {
        return absl::StrJoin(fullName.parts, "::", core::packages::NameFormatter(gs));
    }

    bool operator==(const PackageName &rhs) const {
        return mangledName == rhs.mangledName;
    }

    bool operator!=(const PackageName &rhs) const {
        return mangledName != rhs.mangledName;
    }
};

enum class ImportType {
    Normal,
    Test, // test_import

    // "friend-import": This represents code that is re-mapped into a package's own public->private mapping or
    // its private test namespace.
    Friend,
};

struct Import {
    PackageName name;
    ImportType type;

    Import(PackageName &&name, ImportType type) : name(std::move(name)), type(type) {}
};

struct Export {
    FullyQualifiedName fqn;

    explicit Export(FullyQualifiedName &&fqn) : fqn(move(fqn)) {}

    const vector<core::NameRef> &parts() const {
        return fqn.parts;
    }

    static bool lexCmp(const Export &a, const Export &b) {
        // Lex sort by name.
        return core::packages::PackageInfo::lexCmp(a.parts(), b.parts());
    }
};

// For a given vector of NameRefs, this represents the "next" vector that does not begin with its
// prefix (without actually constructing it). Consider the following sorted names:
//
// [A B]
// [A B C]
// [A B D E]
//    <<<< Position of LexNext([A B]) roughly equivalent to [A B <Infinity>]
// [X Y]
// [X Y Z]
class LexNext final {
    const vector<core::NameRef> &names;

public:
    LexNext(const vector<core::NameRef> &names) : names(names) {}

    bool operator<(const vector<core::NameRef> &rhs) const {
        // Lexicographic comparison:
        for (auto lhsIt = names.begin(), rhsIt = rhs.begin(); lhsIt != names.end() && rhsIt != rhs.end();
             ++lhsIt, ++rhsIt) {
            if (lhsIt->rawId() < rhsIt->rawId()) {
                return true;
            } else if (rhsIt->rawId() < lhsIt->rawId()) {
                return false;
            }
        }
        return false;
    }

    bool operator<(const Export &e) const {
        return *this < e.parts();
    }
};

class PackageInfoImpl final : public core::packages::PackageInfo {
public:
    core::NameRef mangledName() const {
        return name.mangledName;
    }

    const vector<core::NameRef> &fullName() const {
        return name.fullName.parts;
    }

    const std::vector<std::string> &pathPrefixes() const {
        return packagePathPrefixes;
    }

    core::Loc fullLoc() const {
        return loc;
    }

    core::Loc declLoc() const {
        return declLoc_;
    }

    bool legacyAutoloaderCompatibility() const {
        return legacyAutoloaderCompatibility_;
    }

    bool exportAll() const {
        return exportAll_;
    }

    // The possible path prefixes associated with files in the package, including path separator at end.
    vector<std::string> packagePathPrefixes;
    PackageName name;

    // loc for the package definition. Full loc, from class to end keyword. Used for autocorrects.
    core::Loc loc;
    // loc for the package definition. Single line (just the class def). Used for error messages.
    core::Loc declLoc_;
    // The names of each package imported by this package.
    vector<Import> importedPackageNames;
    // List of exported items that form the body of this package's public API.
    // These are copied into every package that imports this package.
    vector<Export> exports_;

    // Code in this package is _completely incompatible_ for path-based autoloading, and only works with the 'legacy'
    // Sorbet-generated autoloader.
    bool legacyAutoloaderCompatibility_;

    // Whether this package should just export everything
    bool exportAll_;

    // The other packages to which this package is visible. If this vector is empty, then it means
    // the package is fully public and can be imported by anything.
    vector<PackageName> visibleTo_;

    // PackageInfoImpl is the only implementation of PackageInfoImpl
    const static PackageInfoImpl &from(const core::packages::PackageInfo &pkg) {
        ENFORCE(pkg.exists());
        return reinterpret_cast<const PackageInfoImpl &>(pkg); // TODO is there a more idiomatic way to do this?
    }

    static PackageInfoImpl &from(core::packages::PackageInfo &pkg) {
        ENFORCE(pkg.exists());
        return reinterpret_cast<PackageInfoImpl &>(pkg); // TODO is there a more idiomatic way to do this?
    }

    unique_ptr<PackageInfo> deepCopy() const {
        return make_unique<PackageInfoImpl>(*this);
    }

    bool ownsSymbol(const core::GlobalState &gs, core::SymbolRef symbol) const {
        auto file = symbol.loc(gs).file();
        auto &pkg = gs.packageDB().getPackageForFile(gs, file);
        return this->mangledName() == pkg.mangledName();
    }

    PackageInfoImpl() = default;
    explicit PackageInfoImpl(const PackageInfoImpl &) = default;
    PackageInfoImpl &operator=(const PackageInfoImpl &) = delete;

    optional<core::AutocorrectSuggestion> addImport(const core::GlobalState &gs, const PackageInfo &pkg,
                                                    bool isTestImport) const {
        auto &info = PackageInfoImpl::from(pkg);
        for (auto &import : importedPackageNames) {
            if (import.name != info.name) {
                continue;
            }
            if (!isTestImport && import.type == ImportType::Test) {
                return convertTestImport(gs, info, core::Loc(fullLoc().file(), import.name.loc));
            }
            // we already import this, and if so, don't return an autocorrect
            return nullopt;
        }

        core::Loc insertionLoc = loc.adjust(gs, core::INVALID_POS_LOC, core::INVALID_POS_LOC);
        // first let's try adding it to the end of the imports.
        if (!importedPackageNames.empty()) {
            auto lastOffset = importedPackageNames.back().name.loc;
            insertionLoc = core::Loc{loc.file(), lastOffset.copyEndWithZeroLength()};
        } else {
            // if we don't have any imports, then we can try adding it
            // either before the first export, or if we have no
            // exports, then right before the final `end`
            uint32_t exportLoc;
            if (!exports_.empty()) {
                exportLoc = exports_.front().fqn.loc.beginPos() - "export "sv.size() - 1;
            } else {
                exportLoc = loc.endPos() - "end"sv.size() - 1;
            }
            // we want to find the end of the last non-empty line, so
            // let's do something gross: walk backward until we find non-whitespace
            const auto &file_source = loc.file().data(gs).source();
            while (isspace(file_source[exportLoc])) {
                exportLoc--;
                // this shouldn't happen in a well-formatted
                // `__package.rb` file, but just to be safe
                if (exportLoc == 0) {
                    return nullopt;
                }
            }
            insertionLoc = {loc.file(), exportLoc + 1, exportLoc + 1};
        }
        ENFORCE(insertionLoc.exists());

        // now find the appropriate place for it, specifically by
        // finding the import that directly preceeds it, if any
        core::AutocorrectSuggestion suggestion(
            fmt::format("Import `{}` in package `{}`", info.name.toString(gs), name.toString(gs)),
            {{insertionLoc,
              fmt::format("\n  {} {}", isTestImport ? "test_import" : "import", info.name.toString(gs))}});
        return {suggestion};
    }

    optional<core::AutocorrectSuggestion> addExport(const core::GlobalState &gs,
                                                    const core::SymbolRef newExport) const {
        auto insertionLoc = core::Loc::none(loc.file());
        // first let's try adding it to the end of the imports.
        if (!exports_.empty()) {
            auto lastOffset = exports_.back().fqn.loc.offsets();
            insertionLoc = core::Loc{loc.file(), lastOffset.copyEndWithZeroLength()};
        } else {
            // if we don't have any imports, then we can try adding it
            // either before the first export, or if we have no
            // exports, then right before the final `end`
            uint32_t exportLoc = loc.endPos() - "end"sv.size() - 1;
            // we want to find the end of the last non-empty line, so
            // let's do something gross: walk backward until we find non-whitespace
            const auto &file_source = loc.file().data(gs).source();
            while (isspace(file_source[exportLoc])) {
                exportLoc--;
                // this shouldn't happen in a well-formatted
                // `__package.rb` file, but just to be safe
                if (exportLoc == 0) {
                    return nullopt;
                }
            }
            insertionLoc = {loc.file(), exportLoc + 1, exportLoc + 1};
        }
        ENFORCE(insertionLoc.exists());

        // now find the appropriate place for it, specifically by
        // finding the import that directly preceeds it, if any
        auto strName = newExport.show(gs);
        core::AutocorrectSuggestion suggestion(fmt::format("Export `{}` in package `{}`", strName, name.toString(gs)),
                                               {{insertionLoc, fmt::format("\n  export {}", strName)}});
        return {suggestion};
    }

    core::AutocorrectSuggestion convertTestImport(const core::GlobalState &gs, const PackageInfoImpl &pkg,
                                                  core::Loc importLoc) const {
        auto [lineStart, _] = importLoc.findStartOfLine(gs);
        core::Loc replaceLoc(importLoc.file(), lineStart.beginPos(), importLoc.endPos());
        return core::AutocorrectSuggestion(fmt::format("Convert `{}` to `{}`", "test_import", "import"),
                                           {{replaceLoc, fmt::format("import {}", pkg.name.toString(gs))}});
    }

    vector<vector<core::NameRef>> exports() const {
        vector<vector<core::NameRef>> rv;
        for (auto &e : exports_) {
            rv.emplace_back(e.fqn.parts);
        }
        return rv;
    }
    vector<vector<core::NameRef>> imports() const {
        vector<vector<core::NameRef>> rv;
        for (auto &i : importedPackageNames) {
            if (i.type == ImportType::Normal) {
                rv.emplace_back(i.name.fullName.parts);
            }
        }
        return rv;
    }
    vector<vector<core::NameRef>> testImports() const {
        vector<vector<core::NameRef>> rv;
        for (auto &i : importedPackageNames) {
            if (i.type == ImportType::Test) {
                rv.emplace_back(i.name.fullName.parts);
            }
        }
        return rv;
    }
    vector<vector<core::NameRef>> visibleTo() const {
        vector<vector<core::NameRef>> rv;
        for (auto &v : visibleTo_) {
            rv.emplace_back(v.fullName.parts);
        }
        return rv;
    }

    std::optional<core::packages::ImportType> importsPackage(core::NameRef mangledName) const {
        if (!mangledName.exists()) {
            return std::nullopt;
        }

        auto imp =
            absl::c_find_if(importedPackageNames, [mangledName](auto &i) { return i.name.mangledName == mangledName; });
        if (imp == importedPackageNames.end()) {
            return nullopt;
        }

        switch (imp->type) {
            case ImportType::Normal:
                return core::packages::ImportType::Normal;
            case ImportType::Test:
                return core::packages::ImportType::Test;
            case ImportType::Friend:
                ENFORCE(false, "Should not happen");
                return nullopt;
        }
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
                    {core::AutocorrectSuggestion::Edit{ctx.locAt(nameLoc), replacement}}});
            }
        }
        constLit = ast::cast_tree<ast::UnresolvedConstantLit>(constLit->scope);
    }
}

FullyQualifiedName getFullyQualifiedName(core::Context ctx, ast::UnresolvedConstantLit *constantLit) {
    FullyQualifiedName fqn;
    fqn.loc = ctx.locAt(constantLit->loc);
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
    pName.mangledName = core::packages::MangledName::mangledNameFromParts(ctx.state, pName.fullName.parts);

    return pName;
}

bool isReferenceToPackageSpec(core::Context ctx, ast::ExpressionPtr &expr) {
    auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return constLit != nullptr && constLit->cnst == core::Names::Constants::PackageSpec();
}

ast::ExpressionPtr prependName(ast::ExpressionPtr scope, core::NameRef prefix) {
    auto lastConstLit = ast::cast_tree<ast::UnresolvedConstantLit>(scope);
    ENFORCE(lastConstLit != nullptr);
    while (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(lastConstLit->scope)) {
        lastConstLit = constLit;
    }
    lastConstLit->scope =
        ast::MK::Constant(lastConstLit->scope.loc().copyWithZeroLength(), core::Symbols::PackageSpecRegistry());
    return scope;
}

ast::ExpressionPtr prependRoot(ast::ExpressionPtr scope) {
    auto *lastConstLit = &ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(scope);
    while (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(lastConstLit->scope)) {
        lastConstLit = constLit;
    }
    auto loc = lastConstLit->scope.loc();
    lastConstLit->scope = ast::MK::Constant(loc, core::Symbols::root());
    return scope;
}

ast::UnresolvedConstantLit *verifyConstant(core::Context ctx, core::NameRef fun, ast::ExpressionPtr &expr) {
    auto target = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    if (target == nullptr) {
        if (auto e = ctx.beginError(expr.loc(), core::errors::Packager::InvalidConfiguration)) {
            e.setHeader("Argument to `{}` must be a constant", fun.show(ctx));
        }
    }
    return target;
}

// Binary search to find a packages index in the global packages list
uint16_t findPackageIndex(core::Context ctx, const PackageInfoImpl &pkg) {
    auto &packages = ctx.state.packageDB().packages();
    return std::lower_bound(packages.begin(), packages.end(), pkg.fullName(),
                            [ctx](auto pkgName, auto &curFileFullName) {
                                auto &pkg = ctx.state.packageDB().getPackageInfo(pkgName);
                                return core::packages::PackageInfo::lexCmp(pkg.fullName(), curFileFullName);
                            }) -
           packages.begin();
}

// Interface for traversing the tree of package namespaces.
// Compactly represent current position in this tree with begin/end offsets. This relies on
// lexicographic sorting of the packages vector. Push operations take advantage of binary search
// to be O(log(end - begin)).
//
// For example with names=[Foo, Bar]
// 0 Foo
// 1 Foo::Bar::Baz    <-- begin
// 2 Foo::Bar::Blub
// 3 Foo::Buzz        <-- end
// 4 Quuz::Bang
// 5 Yaz
// 6 <end of list>
class PackageNamespaces final {
    using Bound = pair<uint16_t, uint16_t>;

    const vector<core::NameRef> &packages; // Mangled names sorted lexicographically
    const PackageInfoImpl &filePkg;        // Package for current file
    // Current bounds:
    uint16_t begin;
    uint16_t end;

    const bool isTestFile;
    const uint16_t filePkgIdx;

    // Count of pushes once we have narrowed down to one possible package:
    int skips = 0;

    vector<Bound> bounds;
    vector<core::NameRef> nameParts;
    vector<core::LocOffsets> namePartsLocs;
    vector<pair<core::NameRef, uint16_t>> curPkg;
    core::NameRef foundTestNS = core::NameRef::noName();
    core::LocOffsets foundTestNSLoc;

    static constexpr uint16_t SKIP_BOUND_VAL = 0;

public:
    PackageNamespaces(core::Context ctx, const PackageInfoImpl &filePkg, bool isTestFile)
        : packages(ctx.state.packageDB().packages()), filePkg(filePkg), begin(0), end(packages.size()),
          isTestFile(isTestFile), filePkgIdx(findPackageIndex(ctx, filePkg)) {
        ENFORCE(packages.size() < numeric_limits<uint16_t>::max());
    }

    int depth() const {
        ENFORCE(nameParts.size() == namePartsLocs.size());
        return nameParts.size();
    }

    const vector<pair<core::NameRef, core::LocOffsets>> currentConstantName() const {
        vector<pair<core::NameRef, core::LocOffsets>> res;

        if (foundTestNS.exists()) {
            res.emplace_back(foundTestNS, foundTestNSLoc);
        }

        ENFORCE(nameParts.size() == namePartsLocs.size());
        for (size_t i = 0; i < nameParts.size(); ++i) {
            res.emplace_back(nameParts[i], namePartsLocs[i]);
        }
        return res;
    }

    core::NameRef packageForNamespace() const {
        if (curPkg.empty()) {
            return core::NameRef::noName();
        }
        return curPkg.back().first;
    }

    bool onPackagePath(core::Context ctx) {
        if (begin <= filePkgIdx && filePkgIdx < end) {
            return true;
        }
        if (!curPkg.empty() && curPkg.back().first == filePkg.mangledName()) {
            return true;
        }
        return false;
    }

    void pushName(core::Context ctx, core::NameRef name, core::LocOffsets loc) {
        if (skips > 0) {
            skips++;
            return;
        }
        bool boundsEmpty = bounds.empty();

        if (isTestFile && boundsEmpty && !foundTestNS.exists()) {
            if (isPrimaryTestNamespace(name)) {
                foundTestNS = name;
                foundTestNSLoc = loc;
                return;
            } else if (!isTestNamespace(ctx, name)) {
                // Inside a test file, but not inside a test namespace. Set bounds such that
                // begin == end, stopping any subsequent search.
                bounds.emplace_back(begin, end);
                nameParts.emplace_back(name);
                namePartsLocs.emplace_back(loc);
                begin = end = 0;
                return;
            }
        }

        if (!boundsEmpty && end - begin == 1 && packages[begin] == filePkg.mangledName() &&
            nameParts.size() >= ctx.state.packageDB().getPackageInfo(packages[begin]).fullName().size()) {
            // We have descended into a package with no sub-packages. At this point it is safe to
            // skip tracking of deeper constants.
            curPkg.emplace_back(packages[begin], SKIP_BOUND_VAL);
            skips++;
            return;
        }

        bounds.emplace_back(begin, end);
        nameParts.emplace_back(name);
        namePartsLocs.emplace_back(loc);

        auto lb = std::lower_bound(packages.begin() + begin, packages.begin() + end, nameParts,
                                   [ctx](auto pkgNr, auto &nameParts) -> bool {
                                       return core::packages::PackageInfo::lexCmp(
                                           ctx.state.packageDB().getPackageInfo(pkgNr).fullName(), nameParts);
                                   });
        auto ub =
            std::upper_bound(lb, packages.begin() + end, LexNext(nameParts), [ctx](auto &next, auto pkgNr) -> bool {
                return next < ctx.state.packageDB().getPackageInfo(pkgNr).fullName();
            });

        begin = lb - packages.begin();
        end = ub - packages.begin();

        if (begin != end) {
            auto &pkgInfo = ctx.state.packageDB().getPackageInfo(*lb);
            ENFORCE(pkgInfo.exists());
            if (nameParts.size() == pkgInfo.fullName().size()) {
                curPkg.emplace_back(*lb, bounds.size());
            }
        }
    }

    void popName() {
        auto prevSkips = skips;
        if (skips > 0) {
            skips--;
            if (skips > 0) {
                return;
            }
        }

        if (isTestFile && bounds.size() == 0 && foundTestNS.exists()) {
            ENFORCE(nameParts.empty());
            foundTestNS = core::NameRef::noName();
            foundTestNSLoc = core::LocOffsets::none();
            return;
        }

        if (prevSkips == 1) {
            ENFORCE(curPkg.back().second == SKIP_BOUND_VAL);
            curPkg.pop_back();
            return;
        }

        if (begin != end && !curPkg.empty()) {
            ENFORCE(!curPkg.empty());
            auto back = curPkg.back();
            if (bounds.size() == back.second) {
                curPkg.pop_back();
            }
        }
        ENFORCE(!bounds.empty());
        begin = bounds.back().first;
        end = bounds.back().second;
        bounds.pop_back();
        nameParts.pop_back();
        namePartsLocs.pop_back();
    }

    ~PackageNamespaces() {
        // Book-keeping sanity checks
        ENFORCE(bounds.empty());
        ENFORCE(nameParts.empty());
        ENFORCE(namePartsLocs.empty());
        ENFORCE(begin == 0);
        ENFORCE(end = packages.size());
        ENFORCE(curPkg.empty());
        ENFORCE(!foundTestNS.exists());
        ENFORCE(skips == 0);
    }
};

// Visitor that ensures for constants defined within a package that all have the package as a
// prefix.
class EnforcePackagePrefix final {
    const PackageInfoImpl &pkg;
    const bool isTestFile;
    PackageNamespaces namespaces;
    // Counter to avoid duplicate errors:
    // - Only emit errors when depth is 0
    // - Upon emitting an error increment
    // - Once greater than 0, all preTransform* increment, postTransform* decrement
    int errorDepth = 0;
    int rootConsts = 0;
    bool useTestNamespace = false;
    vector<std::pair<core::NameRef, core::LocOffsets>> tmpNameParts;

public:
    EnforcePackagePrefix(core::Context ctx, const PackageInfoImpl &pkg, bool isTestFile)
        : pkg(pkg), isTestFile(isTestFile), namespaces(ctx, pkg, isTestFile) {
        ENFORCE(pkg.exists());
    }

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root()) {
            // Ignore top-level <root>
            return;
        }
        if (errorDepth > 0) {
            errorDepth++;
            return;
        }

        ast::UnresolvedConstantLit *constantLit = ast::cast_tree<ast::UnresolvedConstantLit>(classDef.name);
        if (constantLit == nullptr) {
            return;
        }

        pushConstantLit(ctx, constantLit);

        if (rootConsts == 0) {
            if (hasParentClass(classDef)) {
                // A class definition that includes a parent `class Foo::Bar < Baz`
                // must be made in that package
                checkBehaviorLoc(ctx, classDef.declLoc);
            } else if (!namespaces.onPackagePath(ctx)) {
                ENFORCE(errorDepth == 0);
                errorDepth++;
                if (auto e = ctx.beginError(constantLit->loc, core::errors::Packager::DefinitionPackageMismatch)) {
                    definitionPackageMismatch(ctx, e);
                }
            }
        }
    }

    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root()) {
            // Sanity check bookkeeping
            ENFORCE(rootConsts == 0);
            ENFORCE(errorDepth == 0);
            return;
        }

        if (errorDepth > 0) {
            errorDepth--;
            // only continue if this was the first occurrence of the error
            if (errorDepth > 0) {
                return;
            }
        }

        ast::UnresolvedConstantLit *constantLit = ast::cast_tree<ast::UnresolvedConstantLit>(classDef.name);
        if (constantLit == nullptr) {
            return;
        }

        popConstantLit(constantLit);
    }

    void preTransformAssign(core::Context ctx, ast::ExpressionPtr &original) {
        if (errorDepth > 0) {
            errorDepth++;
            return;
        }
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(original);
        auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn.lhs);

        if (lhs != nullptr && rootConsts == 0) {
            pushConstantLit(ctx, lhs);

            if (rootConsts == 0 && namespaces.packageForNamespace() != pkg.mangledName()) {
                ENFORCE(errorDepth == 0);
                errorDepth++;
                if (auto e = ctx.beginError(lhs->loc, core::errors::Packager::DefinitionPackageMismatch)) {
                    definitionPackageMismatch(ctx, e);
                }
            }

            popConstantLit(lhs);
        }
    }

    void postTransformAssign(core::Context ctx, ast::ExpressionPtr &original) {
        if (errorDepth > 0) {
            errorDepth--;
        }
    }

    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &original) {
        if (errorDepth > 0) {
            errorDepth++;
            return;
        }
        auto &def = ast::cast_tree_nonnull<ast::MethodDef>(original);
        checkBehaviorLoc(ctx, def.declLoc);
    }

    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &original) {
        if (errorDepth > 0) {
            errorDepth--;
        }
    }

    void preTransformSend(core::Context ctx, ast::ExpressionPtr &original) {
        if (errorDepth > 0) {
            errorDepth++;
            return;
        }
        checkBehaviorLoc(ctx, original.loc());
    }

    void postTransformSend(core::Context ctx, ast::ExpressionPtr &original) {
        if (errorDepth > 0) {
            errorDepth--;
        }
    }

    void checkBehaviorLoc(core::Context ctx, core::LocOffsets loc) {
        ENFORCE(errorDepth == 0);
        if (rootConsts > 0 || namespaces.depth() == 0) {
            return;
        }
        auto &pkgName = requiredNamespace(ctx);
        auto packageForNamespace = namespaces.packageForNamespace();
        if (packageForNamespace != pkg.mangledName()) {
            ENFORCE(errorDepth == 0);
            errorDepth++;
            if (auto e = ctx.beginError(loc, core::errors::Packager::DefinitionPackageMismatch)) {
                e.setHeader("This file must only define behavior in enclosing package `{}`",
                            fmt::map_join(pkgName, "::", [&](const auto &nr) { return nr.show(ctx); }));
                const auto &constantName = namespaces.currentConstantName();
                e.addErrorLine(ctx.locAt(constantName.back().second), "Defining behavior in `{}` instead:",
                               fmt::map_join(constantName, "::", [&](const auto &nr) { return nr.first.show(ctx); }));
                e.addErrorLine(pkg.declLoc(), "Enclosing package `{}` declared here",
                               fmt::map_join(pkgName, "::", [&](const auto &nr) { return nr.show(ctx); }));
                if (packageForNamespace.exists()) {
                    auto &packageInfo = ctx.state.packageDB().getPackageInfo(packageForNamespace);
                    e.addErrorLine(packageInfo.declLoc(), "Package `{}` declared here",
                                   constantName.back().first.show(ctx));
                }
            }
        }
    }

private:
    void pushConstantLit(core::Context ctx, ast::UnresolvedConstantLit *lit) {
        ENFORCE(tmpNameParts.empty());
        auto prevDepth = namespaces.depth();
        while (lit != nullptr) {
            tmpNameParts.emplace_back(lit->cnst, lit->loc);
            auto *scope = ast::cast_tree<ast::ConstantLit>(lit->scope);
            lit = ast::cast_tree<ast::UnresolvedConstantLit>(lit->scope);
            if (scope != nullptr) {
                ENFORCE(lit == nullptr);
                ENFORCE(scope->symbol == core::Symbols::root());
                rootConsts++;
            }
        }
        if (rootConsts == 0) {
            for (auto it = tmpNameParts.rbegin(); it != tmpNameParts.rend(); ++it) {
                namespaces.pushName(ctx, it->first, it->second);
            }
        }

        if (prevDepth == 0 && isTestFile && namespaces.depth() > 0) {
            useTestNamespace = isPrimaryTestNamespace(tmpNameParts.back().first) ||
                               !isSecondaryTestNamespace(ctx, pkg.name.fullName.parts[0]);
        }

        tmpNameParts.clear();
    }

    void popConstantLit(ast::UnresolvedConstantLit *lit) {
        while (lit != nullptr) {
            if (rootConsts == 0) {
                namespaces.popName();
            }
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
        return useTestNamespace ? pkg.name.fullTestPkgName.parts : pkg.name.fullName.parts;
    }

    bool hasParentClass(const ast::ClassDef &def) const {
        return def.kind == ast::ClassDef::Kind::Class && !def.ancestors.empty() &&
               ast::isa_tree<ast::UnresolvedConstantLit>(def.ancestors[0]);
    }

    void definitionPackageMismatch(const core::GlobalState &gs, core::ErrorBuilder &e) const {
        auto requiredName =
            fmt::format("{}", fmt::map_join(requiredNamespace(gs), "::", [&](const auto nr) { return nr.show(gs); }));

        if (useTestNamespace) {
            e.setHeader("Tests in the `{}` package must define tests in the `{}` namespace", pkg.show(gs),
                        requiredName);
            // TODO: If the only thing missing is a `Test::` prefix (e.g., if this were not a test
            // file there would not have been an error), then we could suggest an autocorrect.
        } else {
            e.setHeader("File belongs to package `{}` but defines a constant that does not match this namespace",
                        requiredName);
        }

        e.addErrorLine(pkg.declLoc(), "Enclosing package declared here");

        auto reqMangledName = namespaces.packageForNamespace();
        if (reqMangledName.exists()) {
            auto &reqPkg = gs.packageDB().getPackageInfo(reqMangledName);
            auto givenNamespace =
                absl::StrJoin(namespaces.currentConstantName(), "::", core::packages::NameFormatter(gs));
            e.addErrorLine(reqPkg.declLoc(), "Must belong to this package, given constant name `{}`", givenNamespace);
        }
    }
};

struct PackageInfoFinder {
    unique_ptr<PackageInfoImpl> info = nullptr;
    vector<Export> exported;

    void postTransformCast(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto &cast = ast::cast_tree_nonnull<ast::Cast>(tree);
        if (!ast::isa_tree<ast::Literal>(cast.typeExpr)) {
            if (auto e = ctx.beginError(cast.typeExpr.loc(), core::errors::Packager::InvalidPackageExpression)) {
                e.setHeader("Invalid expression in package: Arguments to functions must be literals");
            }
        }
    }

    void postTransformSend(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);

        // Ignore methods
        if (send.fun == core::Names::keepDef() || send.fun == core::Names::keepSelfDef()) {
            return;
        }

        // Disallowed methods
        if (send.fun == core::Names::extend() || send.fun == core::Names::include()) {
            if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidPackageExpression)) {
                e.setHeader("Invalid expression in package: `{}` is not allowed", send.fun.shortName(ctx));
            }
            return;
        }

        // Sanity check arguments for unrecognized methods
        if (!isSpecMethod(send)) {
            const auto numPosArgs = send.numPosArgs();
            for (auto i = 0; i < numPosArgs; ++i) {
                auto &arg = send.getPosArg(i);
                if (!ast::isa_tree<ast::Literal>(arg)) {
                    if (auto e = ctx.beginError(arg.loc(), core::errors::Packager::InvalidPackageExpression)) {
                        e.setHeader("Invalid expression in package: Arguments to functions must be literals");
                    }
                }
            }
        }

        if (info == nullptr) {
            // We haven't yet entered the package class.
            return;
        }

        if (send.fun == core::Names::export_() && send.numPosArgs() == 1) {
            // null indicates an invalid export.
            if (auto target = verifyConstant(ctx, core::Names::export_(), send.getPosArg(0))) {
                exported.emplace_back(getFullyQualifiedName(ctx, target));
                auto &arg = send.getPosArg(0);
                arg = prependRoot(std::move(arg));
            }
        }

        if ((send.fun == core::Names::import() || send.fun == core::Names::test_import()) && send.numPosArgs() == 1) {
            // null indicates an invalid import.
            if (auto target = verifyConstant(ctx, send.fun, send.getPosArg(0))) {
                auto name = getPackageName(ctx, target);
                ENFORCE(name.mangledName.exists());

                if (name.mangledName == info->name.mangledName) {
                    if (auto e = ctx.beginError(target->loc, core::errors::Packager::NoSelfImport)) {
                        e.setHeader("Package `{}` cannot {} itself", info->name.toString(ctx), send.fun.toString(ctx));
                    }
                }

                // Transform: `import Foo` -> `import <PackageSpecRegistry>::Foo`
                auto importArg = move(send.getPosArg(0));
                send.removePosArg(0);
                ENFORCE(send.numPosArgs() == 0);
                send.addPosArg(prependName(move(importArg), core::Names::Constants::PackageSpecRegistry()));

                info->importedPackageNames.emplace_back(move(name), method2ImportType(send));
            }
        }

        if (send.fun == core::Names::restrict_to_service() && send.numPosArgs() == 1) {
            // Transform: `restrict_to_service Foo` -> `restrict_to_service <PackageSpecRegistry>::Foo`
            auto importArg = move(send.getPosArg(0));
            send.removePosArg(0);
            ENFORCE(send.numPosArgs() == 0);
            send.addPosArg(prependName(move(importArg), core::Names::Constants::PackageSpecRegistry()));
        }

        if (send.fun == core::Names::exportAll() && send.numPosArgs() == 0) {
            info->exportAll_ = true;
        }

        if (send.fun == core::Names::autoloader_compatibility() && send.numPosArgs() == 1) {
            // Parse autoloader_compatibility DSL and set strict bit on PackageInfoImpl if configured
            auto *compatibilityAnnotationLit = ast::cast_tree<ast::Literal>(send.getPosArg(0));
            if (compatibilityAnnotationLit == nullptr || !compatibilityAnnotationLit->isString()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidConfiguration)) {
                    e.setHeader("Argument to `{}` must be a string literal", send.fun.show(ctx));

                    if (compatibilityAnnotationLit != nullptr && compatibilityAnnotationLit->isSymbol()) {
                        auto symbol = compatibilityAnnotationLit->asSymbol();
                        if (symbol == core::Names::strict() || symbol == core::Names::legacy()) {
                            e.replaceWith("Convert to string arg", ctx.locAt(compatibilityAnnotationLit->loc), "\"{}\"",
                                          symbol.shortName(ctx));
                        }
                    }
                }

                return;
            }

            auto compatibilityAnnotation = compatibilityAnnotationLit->asString();
            if (compatibilityAnnotation != core::Names::legacy()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidConfiguration)) {
                    if (compatibilityAnnotation == core::Names::strict()) {
                        e.setHeader("The 'strict' argument has been deprecated as an argument to `{}`",
                                    send.fun.show(ctx));
                        e.addErrorNote("If you wish to mark your "
                                       "package as strictly path-based-autoloading compatible, do not provide an "
                                       "autoloader_compatibility annotation");
                    } else {
                        e.setHeader("Argument to `{}` can only be 'legacy'", send.fun.show(ctx));
                    }
                }

                return;
            }

            if (compatibilityAnnotation == core::Names::legacy()) {
                info->legacyAutoloaderCompatibility_ = true;
            }
        }

        if (send.fun == core::Names::visible_to() && send.numPosArgs() == 1) {
            if (auto target = verifyConstant(ctx, send.fun, send.getPosArg(0))) {
                auto name = getPackageName(ctx, target);
                ENFORCE(name.mangledName.exists());

                if (name.mangledName == info->name.mangledName) {
                    if (auto e = ctx.beginError(target->loc, core::errors::Packager::NoSelfImport)) {
                        e.setHeader("Useless `{}`, because {} cannot import itself", "visible_to",
                                    info->name.toString(ctx));
                    }
                }

                auto importArg = move(send.getPosArg(0));
                send.removePosArg(0);
                ENFORCE(send.numPosArgs() == 0);
                send.addPosArg(prependName(move(importArg), core::Names::Constants::PackageSpecRegistry()));

                info->visibleTo_.emplace_back(move(name));
            }
        }
    }

    void preTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root()) {
            // Ignore top-level <root>
            return;
        }

        if (classDef.ancestors.size() != 1 || !isReferenceToPackageSpec(ctx, classDef.ancestors[0]) ||
            !ast::isa_tree<ast::UnresolvedConstantLit>(classDef.name)) {
            if (auto e = ctx.beginError(classDef.declLoc, core::errors::Packager::InvalidPackageDefinition)) {
                e.setHeader("Expected package definition of form `Foo::Bar < PackageSpec`");
            }
        } else if (info == nullptr) {
            auto nameTree = ast::cast_tree<ast::UnresolvedConstantLit>(classDef.name);
            info = make_unique<PackageInfoImpl>();
            checkPackageName(ctx, nameTree);
            auto packageName = getPackageName(ctx, nameTree);
            ENFORCE(packageName.mangledName.exists());

            info->name = move(packageName);
            info->loc = ctx.locAt(classDef.loc);
            info->declLoc_ = ctx.locAt(classDef.declLoc);

            // `class Foo < PackageSpec` -> `class <PackageSpecRegistry>::Foo < PackageSpec`
            // This removes the PackageSpec's themselves from the top-level namespace
            classDef.name = prependName(move(classDef.name), core::Names::Constants::PackageSpecRegistry());
        } else {
            if (auto e = ctx.beginError(classDef.declLoc, core::errors::Packager::MultiplePackagesInOneFile)) {
                e.setHeader("Package files can only declare one package");
                e.addErrorLine(info->loc, "Previous package declaration found here");
            }
        }
    }

    void postTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root()) {
            // Ignore top-level <root>
            return;
        }

        return;
    }

    // Generate a list of FQNs exported by this package. No export may be a prefix of another.
    void finalize(core::MutableContext ctx) {
        if (info == nullptr) {
            // HACKFIX: Tolerate completely empty packages. LSP does not support the notion of a deleted file, and
            // instead replaces deleted files with the empty string. It should really mark files as Tombstones instead.
            // Additional note: immutable incremental packager mode now depends on being allowed to set `info = nullptr`
            // for the sake of doing best-effort packager runs.
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

        if (info->exportAll()) {
            // we're only here because exports exist, which means if
            // `exportAll` is set then we've got conflicting
            // information about export; flag the exports as wrong
            for (auto it = exported.begin(); it != exported.end(); ++it) {
                if (auto e = ctx.beginError(it->fqn.loc.offsets(), core::errors::Packager::ExportConflict)) {
                    e.setHeader("Package `{}` declares `{}` and therefore should not use explicit exports",
                                info->name.toString(ctx), "export_all!");
                }
            }
        }

        fast_sort(exported, Export::lexCmp);
        vector<size_t> dupInds;
        for (auto it = exported.begin(); it != exported.end(); ++it) {
            LexNext upperBound(it->parts());
            auto longer = it + 1;
            for (; longer != exported.end() && !(upperBound < *longer); ++longer) {
                if (auto e = ctx.beginError(longer->fqn.loc.offsets(), core::errors::Packager::ExportConflict)) {
                    if (it->parts() == longer->parts()) {
                        e.setHeader("Duplicate export of `{}`",
                                    fmt::map_join(longer->parts(), "::", [&](const auto &nr) { return nr.show(ctx); }));
                    } else {
                        e.setHeader("Cannot export `{}` because another exported name `{}` is a prefix of it",
                                    fmt::map_join(longer->parts(), "::", [&](const auto &nr) { return nr.show(ctx); }),
                                    fmt::map_join(it->parts(), "::", [&](const auto &nr) { return nr.show(ctx); }));
                    }
                    e.addErrorLine(it->fqn.loc, "Prefix exported here");
                }

                dupInds.emplace_back(distance(exported.begin(), longer));
            }
        }

        // Remove duplicates we found (in reverse order)
        fast_sort(dupInds);
        dupInds.erase(unique(dupInds.begin(), dupInds.end()), dupInds.end());
        for (auto indIt = dupInds.rbegin(); indIt != dupInds.rend(); ++indIt) {
            // Yes this is quadratic, but this only happens in an error condition.
            exported.erase(exported.begin() + *indIt);
        }

        ENFORCE(info->exports_.empty());
        std::swap(exported, info->exports_);
    }

    bool isSpecMethod(const sorbet::ast::Send &send) const {
        switch (send.fun.rawId()) {
            case core::Names::import().rawId():
            case core::Names::test_import().rawId():
            case core::Names::export_().rawId():
            case core::Names::restrict_to_service().rawId():
            case core::Names::autoloader_compatibility().rawId():
            case core::Names::visible_to().rawId():
            case core::Names::exportAll().rawId():
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

    void preTransformIf(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "`if`");
    }

    void preTransformWhile(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "`while`");
    }

    void postTransformBreak(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "`break`");
    }

    void postTransformRetry(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "`retry`");
    }

    void postTransformNext(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "`next`");
    }

    void preTransformReturn(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "`return`");
    }

    void preTransformRescueCase(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "`rescue case`");
    }

    void preTransformRescue(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "`rescue`");
    }

    void preTransformAssign(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "`=`");
    }

    void preTransformHash(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "hash literals");
    }

    void preTransformArray(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "array literals");
    }

    void preTransformMethodDef(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "method definitions");
    }

    void preTransformBlock(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "blocks");
    }

    void preTransformInsSeq(core::MutableContext ctx, ast::ExpressionPtr &original) {
        illegalNode(ctx, original.loc(), "`begin` and `end`");
    }
};

unique_ptr<PackageInfoImpl>
runPackageInfoFinder(core::MutableContext ctx, ast::ParsedFile &package,
                     const vector<std::string> &extraPackageFilesDirectoryUnderscorePrefixes,
                     const vector<std::string> &extraPackageFilesDirectorySlashPrefixes) {
    ENFORCE(package.file.exists());
    ENFORCE(package.file.data(ctx).isPackage());
    // Assumption: Root of AST is <root> class. (This won't be true
    // for `typed: ignore` files, so we should make sure to catch that
    // elsewhere.)
    ENFORCE(ast::isa_tree<ast::ClassDef>(package.tree));
    ENFORCE(ast::cast_tree_nonnull<ast::ClassDef>(package.tree).symbol == core::Symbols::root());
    auto packageFilePath = package.file.data(ctx).path();
    ENFORCE(FileOps::getFileName(packageFilePath) == PACKAGE_FILE_NAME);
    PackageInfoFinder finder;
    ast::TreeWalk::apply(ctx, finder, package.tree);
    finder.finalize(ctx);
    if (finder.info) {
        const auto numPrefixes =
            extraPackageFilesDirectoryUnderscorePrefixes.size() + extraPackageFilesDirectorySlashPrefixes.size() + 1;
        finder.info->packagePathPrefixes.reserve(numPrefixes);
        finder.info->packagePathPrefixes.emplace_back(packageFilePath.substr(0, packageFilePath.find_last_of('/') + 1));
        const string_view shortName = finder.info->name.mangledName.shortName(ctx.state);
        const string_view dirNameFromShortName = shortName.substr(0, shortName.rfind(core::PACKAGE_SUFFIX));

        for (const string &prefix : extraPackageFilesDirectoryUnderscorePrefixes) {
            // Project_FooBar -- munge with underscore
            string additionalDirPath = absl::StrCat(prefix, dirNameFromShortName, "/");
            finder.info->packagePathPrefixes.emplace_back(std::move(additionalDirPath));
        }

        for (const string &prefix : extraPackageFilesDirectorySlashPrefixes) {
            // project/Foo_bar -- convert camel-case to snake-case and munge with slash
            std::stringstream ss;
            ss << prefix;
            for (int i = 0; i < dirNameFromShortName.length(); i++) {
                if (dirNameFromShortName[i] == '_') {
                    ss << '/';
                } else if (i == 0 || dirNameFromShortName[i - 1] == '_') {
                    // Capitalizing first letter in each directory name to avoid conflicts with ignored directories,
                    // which tend to be all lower case
                    char upper = std::toupper(dirNameFromShortName[i]);
                    ss << std::move(upper);
                } else {
                    if (isupper(dirNameFromShortName[i])) {
                        ss << '_'; // snake-case munging
                    }

                    char lower = std::tolower(dirNameFromShortName[i]);
                    ss << std::move(lower);
                }
            }
            ss << '/';

            std::string additionalDirPath(ss.str());
            finder.info->packagePathPrefixes.emplace_back(std::move(additionalDirPath));
        }
    }
    return move(finder.info);
}

} // namespace

// Validate that the package file is marked `# typed: strict`.
ast::ParsedFile validatePackage(core::Context ctx, ast::ParsedFile file) {
    const auto &packageDB = ctx.state.packageDB();
    auto &absPkg = packageDB.getPackageForFile(ctx, file.file);
    if (!absPkg.exists()) {
        // We already produced an error on this package when producing its package info.
        // The correct course of action is to abort the transform.
        return file;
    }

    auto &pkgInfo = PackageInfoImpl::from(absPkg);
    bool skipImportVisibilityCheck = packageDB.skipImportVisibilityCheckFor(pkgInfo.mangledName());

    if (!skipImportVisibilityCheck) {
        for (auto &i : pkgInfo.importedPackageNames) {
            auto &otherPkg = packageDB.getPackageInfo(i.name.mangledName);

            // this might mean the other package doesn't exist, but that
            // should have been caught already
            if (!otherPkg.exists()) {
                continue;
            }

            const auto &visibleTo = otherPkg.visibleTo();
            if (visibleTo.empty()) {
                continue;
            }

            bool allowed = absl::c_any_of(otherPkg.visibleTo(),
                                          [&absPkg](const auto &other) { return other == absPkg.fullName(); });

            if (!allowed) {
                if (auto e = ctx.beginError(i.name.loc, core::errors::Packager::ImportNotVisible)) {
                    e.setHeader("Package `{}` includes explicit visibility modifiers and cannot be imported from `{}`",
                                otherPkg.show(ctx), absPkg.show(ctx));
                    e.addErrorNote("Please consult with the owning team before adding a `{}` line to the package `{}`",
                                   "visible_to", otherPkg.show(ctx));
                }
            }
        }
    }

    // Sanity check: __package.rb files _must_ be typed: strict
    if (file.file.data(ctx).originalSigil < core::StrictLevel::Strict) {
        if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::PackageFileMustBeStrict)) {
            e.setHeader("Package files must be at least `{}`", "# typed: strict");
        }
    }

    return file;
}

ast::ParsedFile rewritePackagedFile(core::Context ctx, ast::ParsedFile parsedFile) {
    auto &file = parsedFile.file.data(ctx);
    ENFORCE(!file.isPackage());

    if (file.isPayload()) {
        // Files in Sorbet's payload are parsed and loaded in the --store-state phase, which runs
        // outside of the packager mode. They're allowed to not belong to a package.
        //
        // Note that other RBIs that are not in Sorbet's payload follow the normal packaging rules.
        //
        // We normally skip running the packager when building in sorbet-orig mode, which computes
        // the stored state, but payload files can be retypechecked by the fast path during LSP.
        return parsedFile;
    }

    auto &pkg = ctx.state.packageDB().getPackageForFile(ctx, ctx.file);
    if (!pkg.exists()) {
        // Don't transform, but raise an error on the first line.
        if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::UnpackagedFile)) {
            e.setHeader("File `{}` does not belong to a package; add a `{}` file to one "
                        "of its parent directories",
                        ctx.file.data(ctx).path(), PACKAGE_FILE_NAME);
        }
        return parsedFile;
    }

    auto &pkgImpl = PackageInfoImpl::from(pkg);

    EnforcePackagePrefix enforcePrefix(ctx, pkgImpl, file.isPackagedTest());
    ast::ShallowWalk::apply(ctx, enforcePrefix, parsedFile.tree);

    return parsedFile;
}

// Re-write source files to be in packages. This is only called if no package definitions were changed.
vector<ast::ParsedFile> rewriteFilesFast(core::GlobalState &gs, vector<ast::ParsedFile> files) {
    Timer timeit(gs.tracer(), "packager.rewriteFilesFast");
    for (auto &file : files) {
        core::Context ctx(gs, core::Symbols::root(), file.file);
        if (file.file.data(gs).isPackage()) {
            {
                core::MutableContext ctx(gs, core::Symbols::root(), file.file);
                runPackageInfoFinder(ctx, file, gs.packageDB().extraPackageFilesDirectoryUnderscorePrefixes(),
                                     gs.packageDB().extraPackageFilesDirectorySlashPrefixes());
            }
            // Re-write imports and exports:
            file = validatePackage(ctx, move(file));
        } else {
            file = rewritePackagedFile(ctx, move(file));
        }
    }
    return files;
}

vector<ast::ParsedFile> Packager::findPackages(core::GlobalState &gs, WorkerPool &workers,
                                               vector<ast::ParsedFile> files) {
    // Ensure files are in canonical order.
    fast_sort(files, [](const auto &a, const auto &b) -> bool { return a.file < b.file; });

    // Step 1: Find packages and determine their imports/exports.
    {
        Timer timeit(gs.tracer(), "packager.findPackages");
        core::UnfreezeNameTable unfreeze(gs);
        core::packages::UnfreezePackages packages = gs.unfreezePackages();
        for (auto &file : files) {
            if (!file.file.data(gs).isPackage()) {
                continue;
            }

            if (file.file.data(gs).strictLevel == core::StrictLevel::Ignore) {
                // if the `__package.rb` file is at `typed:
                // ignore`, then we haven't even parsed it, which
                // means none of the other stuff here is going to
                // actually work (since it all assumes we've got a
                // file to actually analyze.) If we've got a
                // `typed: ignore` package, then skip it.

                // `File::isPackage` is determined by the filename, so we need to clear it explicitly at this point
                // to ensure that the file is no longer treated as a package.
                file.file.data(gs).setIsPackage(false);

                continue;
            }

            core::MutableContext ctx(gs, core::Symbols::root(), file.file);
            auto pkg = runPackageInfoFinder(ctx, file, gs.packageDB().extraPackageFilesDirectoryUnderscorePrefixes(),
                                            gs.packageDB().extraPackageFilesDirectorySlashPrefixes());
            if (pkg == nullptr) {
                // There was an error creating a PackageInfoImpl for this file, and getPackageInfo has already
                // surfaced that error to the user. Nothing to do here.
                continue;
            }
            auto &prevPkg = gs.packageDB().getPackageInfo(pkg->mangledName());
            if (prevPkg.exists() && prevPkg.declLoc() != pkg->declLoc()) {
                if (auto e = ctx.beginError(pkg->loc.offsets(), core::errors::Packager::RedefinitionOfPackage)) {
                    auto pkgName = pkg->name.toString(ctx);
                    e.setHeader("Redefinition of package `{}`", pkgName);
                    e.addErrorLine(prevPkg.declLoc(), "Package `{}` originally defined here", pkgName);
                }
            } else {
                packages.db.enterPackage(move(pkg));
            }
        }
    }

    return files;
}

void Packager::setPackageNameOnFiles(core::GlobalState &gs, const vector<ast::ParsedFile> &files) {
    std::vector<std::pair<core::FileRef, core::NameRef>> mapping;
    mapping.reserve(files.size());

    // Step 1a, add package references to every file. This could be parallel if needed, file access will be unique and
    // no symbols will be allocated.
    {
        auto &db = gs.packageDB();
        for (auto &f : files) {
            auto &pkg = db.getPackageForFile(gs, f.file);
            if (!pkg.exists()) {
                continue;
            }

            mapping.emplace_back(f.file, pkg.mangledName());
        }
    }

    {
        auto packages = gs.unfreezePackages();
        for (auto [file, package] : mapping) {
            packages.db.setPackageNameForFile(file, package);
        }
    }

    return;
}

// NOTE: we use `dataAllowingUnsafe` here, as determining the package for a file is something that can be done from its
// path alone.
void Packager::setPackageNameOnFiles(core::GlobalState &gs, const vector<core::FileRef> &files) {
    std::vector<std::pair<core::FileRef, core::NameRef>> mapping;
    mapping.reserve(files.size());

    // Step 1a, add package references to every file. This could be parallel if needed, file access will be unique and
    // no symbols will be allocated.
    {
        auto &db = gs.packageDB();
        for (auto &f : files) {
            auto &pkg = db.getPackageForFile(gs, f);
            if (!pkg.exists()) {
                continue;
            }

            mapping.emplace_back(f, pkg.mangledName());
        }
    }

    {
        auto packages = gs.unfreezePackages();
        for (auto [file, package] : mapping) {
            packages.db.setPackageNameForFile(file, package);
        }
    }

    return;
}

vector<ast::ParsedFile> Packager::run(core::GlobalState &gs, WorkerPool &workers, vector<ast::ParsedFile> files) {
    ENFORCE(!gs.runningUnderAutogen, "Packager pass does not run in autogen");

    Timer timeit(gs.tracer(), "packager");

    files = findPackages(gs, workers, std::move(files));
    setPackageNameOnFiles(gs, files);

    // Step 2:
    // * Find package files and rewrite them into virtual AST mappings.
    // * Find files within each package and rewrite each to be wrapped by their virtual package namespace.
    {
        Timer timeit(gs.tracer(), "packager.rewritePackagesAndFiles");

        auto taskq = std::make_shared<ConcurrentBoundedQueue<size_t>>(files.size());
        absl::BlockingCounter barrier(max(workers.size(), 1));

        for (size_t i = 0; i < files.size(); ++i) {
            taskq->push(i, 1);
        }

        workers.multiplexJob("rewritePackagesAndFiles", [&gs, &files, &barrier, taskq]() {
            Timer timeit(gs.tracer(), "packager.rewritePackagesAndFilesWorker");
            size_t idx;
            for (auto result = taskq->try_pop(idx); !result.done(); result = taskq->try_pop(idx)) {
                ast::ParsedFile &job = files[idx];
                if (result.gotItem()) {
                    auto &file = job.file.data(gs);
                    core::Context ctx(gs, core::Symbols::root(), job.file);

                    if (file.isPackage()) {
                        job = validatePackage(ctx, move(job));
                    } else {
                        job = rewritePackagedFile(ctx, move(job));
                    }
                }
            }

            barrier.DecrementCount();
        });

        barrier.Wait();
    }

    return files;
}

vector<ast::ParsedFile> Packager::runIncremental(core::GlobalState &gs, vector<ast::ParsedFile> files) {
    // Note: This will only run if packages have not been changed (byte-for-byte equality).
    // TODO(nroman-stripe) This could be further incrementalized to avoid processing all packages by
    // building in an understanding of the dependencies between packages.
    auto namesUsed = gs.namesUsedTotal();
    files = rewriteFilesFast(gs, move(files));
    ENFORCE(gs.namesUsedTotal() == namesUsed);
    return files;
    Packager::setPackageNameOnFiles(gs, files);
    return files;
}

namespace {

class ImportFormatter final {
    const core::GlobalState &gs;

public:
    ImportFormatter(const core::GlobalState &gs) : gs(gs) {}

    void operator()(std::string *out, const vector<core::NameRef> &name) const {
        fmt::format_to(back_inserter(*out), "\"{}\"", absl::StrJoin(name, "::", core::packages::NameFormatter(gs)));
    }
};

class FileListFormatter final {
    const core::GlobalState &gs;

public:
    FileListFormatter(const core::GlobalState &gs) : gs(gs) {}

    void operator()(std::string *out, core::FileRef file) const {
        out->append("\"");
        out->append(file.data(gs).path());
        out->append("\"");
    }
};

struct PackageFiles {
    vector<core::FileRef> files;
    vector<core::FileRef> testFiles;
};

class PackageInfoFormatter final {
    const core::GlobalState &gs;
    const UnorderedMap<core::NameRef, PackageFiles> &packageFiles;

public:
    PackageInfoFormatter(const core::GlobalState &gs, const UnorderedMap<core::NameRef, PackageFiles> &packageFiles)
        : gs(gs), packageFiles(packageFiles) {}

    void operator()(std::string *out, core::NameRef mangledName) const {
        const auto &pkg = gs.packageDB().getPackageInfo(mangledName);
        out->append("{{");
        out->append("\"name\":");
        fmt::format_to(back_inserter(*out), "\"{}\",",
                       absl::StrJoin(pkg.fullName(), "::", core::packages::NameFormatter(gs)));
        out->append("\"imports\":[");
        fmt::format_to(back_inserter(*out), absl::StrJoin(pkg.imports(), ",", ImportFormatter(gs)));
        out->append("],\"testImports\":[");
        fmt::format_to(back_inserter(*out), absl::StrJoin(pkg.testImports(), ",", ImportFormatter(gs)));
        out->append("],\"files\":[");
        const auto it = packageFiles.find(mangledName);
        if (it != packageFiles.end()) {
            fmt::format_to(back_inserter(*out), absl::StrJoin(it->second.files, ",", FileListFormatter(gs)));
        }
        out->append("], \"testFiles\":[");
        if (it != packageFiles.end()) {
            fmt::format_to(back_inserter(*out), absl::StrJoin(it->second.testFiles, ",", FileListFormatter(gs)));
        }
        out->append("]}}");
    }
};

} // namespace

void Packager::dumpPackageInfo(const core::GlobalState &gs, std::string outputFile) {
    const auto &pkgDB = gs.packageDB();
    // package => files
    UnorderedMap<core::NameRef, PackageFiles> packageFiles;
    for (uint32_t i = 1; i < gs.filesUsed(); ++i) {
        core::FileRef file(i);
        const auto &pkg = pkgDB.getPackageForFile(gs, file);
        if (pkg.exists()) {
            if (file.data(gs).isPackagedTest()) {
                packageFiles[pkg.mangledName()].testFiles.emplace_back(file);
            } else {
                packageFiles[pkg.mangledName()].files.emplace_back(file);
            }
        }
    }

    fmt::memory_buffer out;
    fmt::format_to(back_inserter(out), "[");
    fmt::format_to(back_inserter(out), absl::StrJoin(pkgDB.packages(), ",", PackageInfoFormatter(gs, packageFiles)));
    fmt::format_to(back_inserter(out), "]");
    FileOps::write(outputFile, fmt::to_string(out));
}

} // namespace sorbet::packager
