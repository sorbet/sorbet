#ifndef SORBET_CORE_PACKAGES_PACKAGEINFO_H
#define SORBET_CORE_PACKAGES_PACKAGEINFO_H

#include "absl/types/span.h"

#include "core/NameRef.h"
#include "core/StrictLevel.h"
#include "core/SymbolRef.h"
#include "core/packages/MangledName.h"
#include <optional>
#include <vector>

namespace sorbet::core {
struct AutocorrectSuggestion;
class NameRef;
class Loc;
class GlobalState;
class Context;
} // namespace sorbet::core

namespace sorbet::core::packages {

class PackageDB;

enum class ImportType {
    Normal,
    TestHelper,
    TestUnit,
};

enum class VisibleToType {
    Normal,
    Wildcard,
};

enum class StrictDependenciesLevel {
    False,
    Layered,
    LayeredDag,
    Dag,
};

std::string_view strictDependenciesLevelToString(core::packages::StrictDependenciesLevel level);

struct FullyQualifiedName {
    vector<core::NameRef> parts;

    FullyQualifiedName() = default;
    FullyQualifiedName(vector<core::NameRef> parts) : parts(parts) {}
    explicit FullyQualifiedName(const FullyQualifiedName &) = default;
    FullyQualifiedName(FullyQualifiedName &&) = default;
    FullyQualifiedName &operator=(const FullyQualifiedName &) = delete;
    FullyQualifiedName &operator=(FullyQualifiedName &&) = default;

    string show(const core::GlobalState &gs) const {
        return absl::StrJoin(parts, "::", core::packages::NameFormatter(gs));
    }
};

struct Import {
    MangledName mangledName;
    ImportType type;
    core::LocOffsets loc;

    Import(MangledName mangledName, ImportType type, core::LocOffsets loc)
        : mangledName(mangledName), type(type), loc(loc) {}

    bool isTestImport() const {
        return type != ImportType::Normal;
    }
};

struct Export {
    FullyQualifiedName fqn;
    core::LocOffsets loc;

    explicit Export(FullyQualifiedName &&fqn, core::LocOffsets loc) : fqn(move(fqn)), loc(loc) {}

    const vector<core::NameRef> &parts() const {
        return fqn.parts;
    }

    // Lex sort by name.
    static bool lexCmp(const Export &a, const Export &b) {
        return absl::c_lexicographical_compare(a.parts(), b.parts(),
                                               [](auto a, auto b) -> bool { return a.rawId() < b.rawId(); });
    }
};

struct VisibleTo {
    MangledName mangledName;
    VisibleToType type;

    VisibleTo(MangledName mangledName, VisibleToType type) : mangledName(mangledName), type(type){};
};

class PackageInfo {
public:
    virtual MangledName mangledName() const = 0;
    virtual absl::Span<const std::string> pathPrefixes() const = 0;
    virtual std::vector<VisibleTo> visibleTo() const = 0;
    virtual std::unique_ptr<PackageInfo> deepCopy() const = 0;
    virtual std::optional<std::pair<core::packages::StrictDependenciesLevel, core::LocOffsets>>
    strictDependenciesLevel() const = 0;
    virtual std::optional<std::pair<core::NameRef, core::LocOffsets>> layer() const = 0;

    // The id of the SCC that this package's normal imports belong to.
    //
    // WARNING: Modifying the contents of the package DB after this operation will cause this id to go out of
    // date.
    virtual std::optional<int> sccID() const = 0;

    // The ID of the SCC that this package's tests belong to. This ID is only useful in the context of the package graph
    // condensation graph.
    //
    // WARNING: Modifying the contents of the package DB after this operation will cause this id to go out of
    // date.
    virtual std::optional<int> testSccID() const = 0;

    virtual core::Loc fullLoc() const = 0;
    virtual core::Loc declLoc() const = 0;
    virtual bool exists() const final;
    std::string show(const core::GlobalState &gs) const;

    virtual std::optional<ImportType> importsPackage(MangledName mangledName) const = 0;

    // Is it a layering violation to import otherPkg from this package?
    virtual bool causesLayeringViolation(const core::packages::PackageDB &packageDB,
                                         const PackageInfo &otherPkg) const = 0;
    // What is the minimum strict dependencies level that this package's imports must have?
    virtual core::packages::StrictDependenciesLevel minimumStrictDependenciesLevel() const = 0;
    // Returns a string representing the path to the given package from this package, if it exists. Note: this only
    // looks at non-test imports.
    virtual std::optional<std::string> pathTo(const core::GlobalState &gs,
                                              const core::packages::MangledName dest) const = 0;

    // autocorrects
    virtual std::optional<core::AutocorrectSuggestion> addImport(const core::GlobalState &gs, const PackageInfo &pkg,
                                                                 ImportType importType) const = 0;
    virtual std::optional<core::AutocorrectSuggestion> addExport(const core::GlobalState &gs,
                                                                 const core::SymbolRef name) const = 0;

    bool operator==(const PackageInfo &rhs) const;

    virtual ~PackageInfo() = 0;
    PackageInfo() = default;
    PackageInfo(PackageInfo &) = delete;
    explicit PackageInfo(const PackageInfo &) = default;
    PackageInfo &operator=(PackageInfo &&) = delete;
    PackageInfo &operator=(const PackageInfo &) = delete;

    struct MissingExportMatch {
        core::SymbolRef symbol;
        core::NameRef srcPkg;
    };

    virtual bool exportAll() const = 0;
    virtual bool visibleToTests() const = 0;
};

class PackageInfoImpl final : public core::packages::PackageInfo {
public:
    core::packages::MangledName mangledName() const {
        return mangledName_;
    }

    absl::Span<const string> pathPrefixes() const {
        return absl::MakeSpan(packagePathPrefixes);
    }

    core::Loc fullLoc() const {
        return loc;
    }

    core::Loc declLoc() const {
        return declLoc_;
    }

    bool exportAll() const {
        return exportAll_;
    }

    bool visibleToTests() const {
        return visibleToTests_;
    }

    core::packages::MangledName mangledName_;

    // loc for the package definition. Full loc, from class to end keyword. Used for autocorrects.
    core::Loc loc;
    // loc for the package definition. Single line (just the class def). Used for error messages.
    core::Loc declLoc_;
    // The possible path prefixes associated with files in the package, including path separator at end.
    vector<string> packagePathPrefixes = {};
    // The names of each package imported by this package.
    vector<core::packages::Import> importedPackageNames = {};
    // List of exported items that form the body of this package's public API.
    // These are copied into every package that imports this package.
    vector<Export> exports_ = {};

    // Whether this package should just export everything
    bool exportAll_ = false;

    // The other packages to which this package is visible. If this vector is empty, then it means
    // the package is fully public and can be imported by anything.
    //
    // The `VisibleToType` here represents whether to treat this line as a "wildcard". `Wildcard` means the
    // `visible_to` line allows this package to be imported not just by the referenced package name
    // but also any package name underneath it. `Normal` means the package can be imported
    // by the referenced package name but not any child packages (unless they have a separate
    // `visible_to` line of their own.)
    vector<core::packages::VisibleTo> visibleTo_ = {};

    // Whether `visible_to` directives should be ignored for test code
    bool visibleToTests_ = false;

    optional<pair<core::packages::StrictDependenciesLevel, core::LocOffsets>> strictDependenciesLevel_ = nullopt;
    optional<pair<core::NameRef, core::LocOffsets>> layer_ = nullopt;
    vector<core::LocOffsets> extraDirectives_ = {};
    std::optional<
        std::pair<std::pair<core::StrictLevel, core::LocOffsets>, std::pair<core::StrictLevel, core::LocOffsets>>>
        min_typed_level_ = nullopt;

    optional<pair<core::packages::StrictDependenciesLevel, core::LocOffsets>> strictDependenciesLevel() const {
        return strictDependenciesLevel_;
    }

    optional<pair<core::NameRef, core::LocOffsets>> layer() const {
        return layer_;
    }

    // ID of the strongly-connected component that this package is in, according to its graph of import dependencies
    optional<int> sccID_ = nullopt;

    optional<int> sccID() const {
        return sccID_;
    }

    // ID of the strongly-connected component that this package's tests are in, according to its graph of import
    // dependencies
    optional<int> testSccID_ = nullopt;

    optional<int> testSccID() const {
        return testSccID_;
    }

    // PackageInfoImpl is the only implementation of PackageInfo
    static PackageInfoImpl &from(core::GlobalState &gs, core::packages::MangledName pkg) {
        ENFORCE(pkg.exists());
        return from(*gs.packageDB().getPackageInfoNonConst(pkg));
    }

    const static PackageInfoImpl &from(const core::GlobalState &gs, core::packages::MangledName pkg) {
        ENFORCE(pkg.exists());
        return from(gs.packageDB().getPackageInfo(pkg));
    }

    // PackageInfoImpl is the only implementation of PackageInfo
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

    PackageInfoImpl(core::packages::MangledName mangledName, core::Loc loc, core::Loc declLoc_)
        : mangledName_(mangledName), loc(loc), declLoc_(declLoc_) {}
    explicit PackageInfoImpl(const PackageInfoImpl &) = default;
    PackageInfoImpl &operator=(const PackageInfoImpl &) = delete;

    // What order should these packages be in the import list?
    // Returns -1 if a should come before b, 0 if they are equivalent, and 1 if a should come after b.
    //
    // This method replicates the logic used at Stripe to order packages and thus has all of the associated "quirks".
    // In particular, the ordering in a given package is a function of the strictness level of the package, and it is
    // not a simple "false < layered < layered_dag < dag" ordering. The ordering is as follows:
    // For strictDependenciesLevel::False:
    // - layering violations
    // - imports to 'false' packages
    // - imports to 'layered' or stricter packages
    // - test imports
    // For strictDependenciesLevel::Layered and LayeredDag:
    // - layering violations
    // - imports to 'false' packages
    // - imports to 'layered' or 'layered_dag' packages
    // - imports to 'dag' packages
    // - test imports
    // For strictDependenciesLevel::Dag:
    // - layering violations
    // - imports to 'false', 'layered', or 'layered_dag' packages
    // - imports to 'dag' packages
    // - test imports
    // TODO(neil): explain the rationale behind this ordering (ie. why is not the simple "false < layered < layered_dag
    // < dag" ordering)
    int orderImports(const core::GlobalState &gs, const PackageInfo &a, bool aIsTestImport, const PackageInfo &b,
                     bool bIsTestImport) const {
        // Test imports always come last, and aren't sorted by `strict_dependencies`
        if (aIsTestImport && bIsTestImport) {
            return orderByAlphabetical(gs, a, b);
        } else if (aIsTestImport && !bIsTestImport) {
            return 1;
        } else if (!aIsTestImport && bIsTestImport) {
            return -1;
        } // Neither is a test import

        auto strictnessCompareResult = orderByStrictness(gs.packageDB(), a, b);
        if (strictnessCompareResult == 0) {
            return orderByAlphabetical(gs, a, b);
        }
        return strictnessCompareResult;
    }

    int orderByStrictness(const core::packages::PackageDB &packageDB, const PackageInfo &a,
                          const PackageInfo &b) const {
        if (!packageDB.enforceLayering() || !strictDependenciesLevel().has_value() ||
            !a.strictDependenciesLevel().has_value() || !b.strictDependenciesLevel().has_value() ||
            !a.layer().has_value() || !b.layer().has_value()) {
            return 0;
        }

        // Layering violations always come first
        auto aCausesLayeringViolation = causesLayeringViolation(packageDB, a.layer().value().first);
        auto bCausesLayeringViolation = causesLayeringViolation(packageDB, b.layer().value().first);
        if (aCausesLayeringViolation && bCausesLayeringViolation) {
            return 0;
        } else if (aCausesLayeringViolation && !bCausesLayeringViolation) {
            return -1;
        } else if (!aCausesLayeringViolation && bCausesLayeringViolation) {
            return 1;
        }

        auto aStrictDependenciesLevel = a.strictDependenciesLevel().value().first;
        auto bStrictDependenciesLevel = b.strictDependenciesLevel().value().first;
        switch (strictDependenciesLevel().value().first) {
            case core::packages::StrictDependenciesLevel::False: {
                // Sort order: Layering violations, false, layered or stricter
                switch (aStrictDependenciesLevel) {
                    case core::packages::StrictDependenciesLevel::False:
                        return bStrictDependenciesLevel == core::packages::StrictDependenciesLevel::False ? 0 : -1;
                    case core::packages::StrictDependenciesLevel::Layered:
                    case core::packages::StrictDependenciesLevel::LayeredDag:
                    case core::packages::StrictDependenciesLevel::Dag:
                        return bStrictDependenciesLevel == core::packages::StrictDependenciesLevel::False ? 1 : 0;
                }
            }
            case core::packages::StrictDependenciesLevel::Layered:
            case core::packages::StrictDependenciesLevel::LayeredDag: {
                // Sort order: Layering violations, false, layered or layered_dag, dag
                switch (aStrictDependenciesLevel) {
                    case core::packages::StrictDependenciesLevel::False:
                        return bStrictDependenciesLevel == core::packages::StrictDependenciesLevel::False ? 0 : -1;
                    case core::packages::StrictDependenciesLevel::Layered:
                    case core::packages::StrictDependenciesLevel::LayeredDag:
                        switch (bStrictDependenciesLevel) {
                            case core::packages::StrictDependenciesLevel::False:
                                return 1;
                            case core::packages::StrictDependenciesLevel::Layered:
                            case core::packages::StrictDependenciesLevel::LayeredDag:
                                return 0;
                            case core::packages::StrictDependenciesLevel::Dag:
                                return -1;
                        }
                    case core::packages::StrictDependenciesLevel::Dag:
                        return bStrictDependenciesLevel == core::packages::StrictDependenciesLevel::Dag ? 0 : 1;
                }
            }
            case core::packages::StrictDependenciesLevel::Dag: {
                // Sort order: Layering violations, false or layered or layered_dag, dag
                switch (aStrictDependenciesLevel) {
                    case core::packages::StrictDependenciesLevel::False:
                    case core::packages::StrictDependenciesLevel::Layered:
                    case core::packages::StrictDependenciesLevel::LayeredDag:
                        return bStrictDependenciesLevel == core::packages::StrictDependenciesLevel::Dag ? -1 : 0;
                    case core::packages::StrictDependenciesLevel::Dag:
                        return bStrictDependenciesLevel == core::packages::StrictDependenciesLevel::Dag ? 0 : 1;
                }
            }
        }
    }

    int orderByAlphabetical(const core::GlobalState &gs, const PackageInfo &a, const PackageInfo &b) const {
        auto aStrName = a.show(gs);
        auto bStrName = b.show(gs);
        if (aStrName == bStrName) {
            return 0;
        }
        return aStrName < bStrName ? -1 : 1;
    }

    optional<core::AutocorrectSuggestion> addImport(const core::GlobalState &gs, const PackageInfo &pkg,
                                                    core::packages::ImportType importType) const {
        auto &info = PackageInfoImpl::from(pkg);
        auto insertionLoc = core::Loc::none(loc.file());
        optional<core::AutocorrectSuggestion::Edit> deleteTestImportEdit = nullopt;
        if (!importedPackageNames.empty()) {
            core::LocOffsets importToInsertAfter;
            for (auto &import : importedPackageNames) {
                if (import.mangledName == info.mangledName()) {
                    if ((importType == core::packages::ImportType::Normal &&
                         import.type != core::packages::ImportType::Normal) ||
                        (importType == core::packages::ImportType::TestHelper &&
                         import.type == core::packages::ImportType::TestUnit)) {
                        // There's already an import for this package, so we'll "upgrade" it to the desired import.
                        // importToInsertAfter already tracks where we need to insert the import.  So we can craft an
                        // edit to delete the existing line, and then use the regular logic for adding an import to
                        // insert the `import`.
                        auto importLoc = core::Loc(fullLoc().file(), import.loc);
                        auto [lineStart, numWhitespace] = importLoc.findStartOfIndentation(gs);
                        auto beginPos =
                            lineStart.adjust(gs, -numWhitespace, 0).beginPos(); // -numWhitespace for the indentation
                        auto endPos = importLoc.endPos();
                        core::Loc replaceLoc(importLoc.file(), beginPos, endPos);

                        deleteTestImportEdit = {replaceLoc, ""};

                        // as a special-case: if we're converting a `test_import` to remove `only:`, then we want to
                        // re-insert it at this exact same point (since we sort those together.) Let's find the previous
                        // import!
                        // TODO: we should find and delete just the `, only: ...` in this case, but that's slightly
                        // tricky

                        if (importType == core::packages::ImportType::TestHelper) {
                            insertionLoc = {importLoc.file(), beginPos - 1, beginPos - 1};
                            break;
                        }
                    } else {
                        // we already import this, and if so, don't return an autocorrect
                        return nullopt;
                    }
                }

                auto &importInfo = gs.packageDB().getPackageInfo(import.mangledName);
                if (!importInfo.exists()) {
                    importToInsertAfter = import.loc;
                    continue;
                }

                auto compareResult = orderImports(gs, info, importType != core::packages::ImportType::Normal,
                                                  importInfo, import.isTestImport());
                if (compareResult == 1 || compareResult == 0) {
                    importToInsertAfter = import.loc;
                }
            }
            if (!importToInsertAfter.exists()) {
                // Insert before the first import
                core::Loc beforePackageName = {loc.file(), importedPackageNames.front().loc};
                auto [beforeImport, numWhitespace] = beforePackageName.findStartOfIndentation(gs);
                auto endOfPrevLine = beforeImport.adjust(gs, -numWhitespace - 1, 0);
                insertionLoc = endOfPrevLine.copyWithZeroLength();
            } else {
                insertionLoc = core::Loc(loc.file(), importToInsertAfter.copyEndWithZeroLength());
            }
        } else {
            // if we don't have any imports, then we can try adding it
            // either before the first export, or if we have no
            // exports, then right before the final `end`
            int64_t exportLoc;
            if (!exports_.empty()) {
                exportLoc = exports_.front().loc.beginPos() - "export "sv.size() - 1;
            } else {
                exportLoc = loc.endPos() - "end"sv.size() - 1;
            }

            string_view file_source = loc.file().data(gs).source();

            // Defensively guard against the first export loc or the package's loc being invalid.
            if (exportLoc <= 0 || exportLoc >= file_source.size()) {
                ENFORCE(false, "Failed to find a valid starting loc");
                return nullopt;
            }

            // we want to find the end of the last non-empty line, so
            // let's do something gross: walk backward until we find non-whitespace
            while (isspace(file_source[exportLoc])) {
                exportLoc--;
                // this shouldn't happen in a well-formatted
                // `__package.rb` file, but just to be safe
                if (exportLoc == 0) {
                    return nullopt;
                }
            }
            insertionLoc = core::Loc(loc.file(), exportLoc + 1, exportLoc + 1);
        }
        ENFORCE(insertionLoc.exists());

        auto packageToImport = info.mangledName_.owner.show(gs);
        string_view importTypeHuman;
        string_view importTypeMethod;
        string_view importTypeTrailing = "";
        switch (importType) {
            case core::packages::ImportType::Normal:
                importTypeHuman = "Import";
                importTypeMethod = "import";
                break;
            case core::packages::ImportType::TestUnit:
                importTypeHuman = "Test Import";
                importTypeMethod = "test_import";
                importTypeTrailing = ", only: \"test_rb\"";
                break;
            case core::packages::ImportType::TestHelper:
                importTypeHuman = "Test Import";
                importTypeMethod = "test_import";
                break;
        }
        auto suggestionTitle =
            fmt::format("{} `{}` in package `{}`", importTypeHuman, packageToImport, mangledName_.owner.show(gs));
        vector<core::AutocorrectSuggestion::Edit> edits = {
            {insertionLoc, fmt::format("\n  {} {}{}", importTypeMethod, packageToImport, importTypeTrailing)}};
        if (deleteTestImportEdit.has_value()) {
            edits.push_back(deleteTestImportEdit.value());
            suggestionTitle = fmt::format("Convert existing import to `{}`", importTypeMethod);
        }

        core::AutocorrectSuggestion suggestion(suggestionTitle, edits);
        return {suggestion};
    }

    optional<core::AutocorrectSuggestion> addExport(const core::GlobalState &gs,
                                                    const core::SymbolRef newExport) const {
        auto pkgFile = loc.file();
        auto insertionLoc = core::Loc::none(pkgFile);
        if (!exports_.empty()) {
            core::LocOffsets exportToInsertAfter;
            for (auto &e : exports_) {
                if (newExport.show(gs) > e.fqn.show(gs)) {
                    exportToInsertAfter = e.loc;
                }
            }
            if (!exportToInsertAfter.exists()) {
                // Insert before the first export
                auto beforeConstantName = exports_.front().loc;
                auto [beforeExport, numWhitespace] = core::Loc(pkgFile, beforeConstantName).findStartOfIndentation(gs);
                auto endOfPrevLine = beforeExport.adjust(gs, -numWhitespace - 1, 0);
                insertionLoc = endOfPrevLine.copyWithZeroLength();
            } else {
                insertionLoc = core::Loc(pkgFile, exportToInsertAfter.copyEndWithZeroLength());
            }
        } else {
            // if we don't have any exports, then we can try adding it right before the final `end`
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

        auto strName = newExport.show(gs);
        core::AutocorrectSuggestion suggestion(
            fmt::format("Export `{}` in package `{}`", strName, mangledName_.owner.show(gs)),
            {{insertionLoc, fmt::format("\n  export {}", strName)}});
        return {suggestion};
    }

    vector<core::packages::VisibleTo> visibleTo() const {
        return visibleTo_;
    }

    optional<core::packages::ImportType> importsPackage(core::packages::MangledName mangledName) const {
        if (!mangledName.exists()) {
            return nullopt;
        }

        auto imp =
            absl::c_find_if(importedPackageNames, [mangledName](auto &i) { return i.mangledName == mangledName; });
        if (imp == importedPackageNames.end()) {
            return nullopt;
        }

        return imp->type;
    }

    // Is it a layering violation to import otherPkg from this package?
    bool causesLayeringViolation(const core::packages::PackageDB &packageDB, const PackageInfo &otherPkg) const {
        if (!otherPkg.layer().has_value()) {
            return false;
        }

        return causesLayeringViolation(packageDB, otherPkg.layer().value().first);
    }

    bool causesLayeringViolation(const core::packages::PackageDB &packageDB, core::NameRef otherPkgLayer) const {
        if (!layer().has_value()) {
            return false;
        }

        auto pkgLayer = layer().value().first;
        auto pkgLayerIndex = packageDB.layerIndex(pkgLayer);
        auto otherPkgLayerIndex = packageDB.layerIndex(otherPkgLayer);

        return pkgLayerIndex < otherPkgLayerIndex;
    }

    // What is the minimum strict dependencies level that this package's imports must have?
    core::packages::StrictDependenciesLevel minimumStrictDependenciesLevel() const {
        if (!strictDependenciesLevel().has_value()) {
            return core::packages::StrictDependenciesLevel::False;
        }

        switch (strictDependenciesLevel().value().first) {
            case core::packages::StrictDependenciesLevel::False:
                return core::packages::StrictDependenciesLevel::False;
            case core::packages::StrictDependenciesLevel::Layered:
            case core::packages::StrictDependenciesLevel::LayeredDag:
                return core::packages::StrictDependenciesLevel::Layered;
            case core::packages::StrictDependenciesLevel::Dag:
                return core::packages::StrictDependenciesLevel::Dag;
        }
    }

    string renderPath(const core::GlobalState &gs, const vector<core::packages::MangledName> &path) const {
        // TODO(neil): if the cycle has a large number of nodes (10?), show partial path (first 5, ... (n omitted), last
        // 5) to prevent error being too long
        // Note: This function iterates through path in reverse order because pathTo generates it in that order, so
        // iterating reverse gives the regular order.
        string pathMessage;
        for (int i = path.size() - 1; i >= 0; i--) {
            auto name = gs.packageDB().getPackageInfo(path[i]).show(gs);
            bool showArrow = i > 0;
            pathMessage += core::ErrorColors::format("    `{}`{}\n", name, showArrow ? " →" : "");
        }
        return pathMessage;
    }

    optional<string> pathTo(const core::GlobalState &gs, const core::packages::MangledName dest) const {
        // Note: This implements BFS.
        auto src = mangledName();
        queue<core::packages::MangledName> toVisit;
        // Maps from package to what package we came from to get to that package, used to construct the path
        // Ex. A -> B -> C means that prev[C] = B and prev[B] = A
        UnorderedMap<core::packages::MangledName, core::packages::MangledName> prev;
        UnorderedSet<core::packages::MangledName> visited;

        toVisit.push(src);
        while (!toVisit.empty()) {
            auto curr = toVisit.front();
            toVisit.pop();
            auto [_it, inserted] = visited.emplace(curr);
            if (!inserted) {
                continue;
            }

            if (curr == dest) {
                // Found the target node, walk backward through the prev map to construct the path
                vector<core::packages::MangledName> path;
                path.push_back(curr);
                while (curr != src) {
                    curr = prev[curr];
                    path.push_back(curr);
                }
                // Note: here, path will be in reverse order (ie. from dest -> src), and then renderPath iterates it in
                // reverse, to get the correct order. If you plan to use path directly, make sure to reverse it (and
                // then upate renderPath to iterate normally).
                return renderPath(gs, path);
            }

            auto &currInfo = PackageInfoImpl::from(gs.packageDB().getPackageInfo(curr));
            for (auto &import : currInfo.importedPackageNames) {
                auto &importInfo = gs.packageDB().getPackageInfo(import.mangledName);
                if (!importInfo.exists() || import.isTestImport() || visited.contains(import.mangledName)) {
                    continue;
                }
                if (!prev.contains(import.mangledName)) {
                    prev[import.mangledName] = curr;
                }
                toVisit.push(import.mangledName);
            }
        }

        // No path found
        return nullopt;
    }
};

} // namespace sorbet::core::packages
#endif
