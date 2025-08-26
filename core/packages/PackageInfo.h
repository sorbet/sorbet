#ifndef SORBET_CORE_PACKAGES_PACKAGEINFO_H
#define SORBET_CORE_PACKAGES_PACKAGEINFO_H

#include "absl/types/span.h"

#include "core/Loc.h"
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

enum class ImportType : uint8_t {
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

std::string_view strictDependenciesLevelToString(StrictDependenciesLevel level);

struct FullyQualifiedName {
    std::vector<core::NameRef> parts;

    FullyQualifiedName() = default;
    FullyQualifiedName(std::vector<core::NameRef> parts) : parts(parts) {}
    explicit FullyQualifiedName(const FullyQualifiedName &) = default;
    FullyQualifiedName(FullyQualifiedName &&) = default;
    FullyQualifiedName &operator=(const FullyQualifiedName &) = delete;
    FullyQualifiedName &operator=(FullyQualifiedName &&) = default;

    std::string show(const core::GlobalState &gs) const;
};

class PackageInfo;

struct Import {
    MangledName mangledName;
    ImportType type;
    bool isPrelude_;
    core::LocOffsets loc;

    Import(MangledName mangledName, ImportType type, core::LocOffsets loc)
        : mangledName(mangledName), type(type), isPrelude_{false}, loc(loc) {}

    // Generate a prelude import, with the given origin.
    // NOTE: the origin must point to something to hang any errors related to the implicit import off of. Currently this
    // is the declLoc_ of the package that is getting the implicit import, and because of this it's important to not use
    // these locs when computing locations for autocorrects.
    static Import prelude(MangledName mangledName, core::LocOffsets implicitLoc);

    Import(Import &&other) = default;
    Import(const Import &other) = default;
    Import &operator=(Import &&other) = default;
    Import &operator=(const Import &other) = default;

    bool isTestImport() const {
        return type != ImportType::Normal;
    }

    // Prelude imports don't correspond to an import definition in a package source, and as such their location
    // should not be relied on for autocorrects.
    bool isPrelude() const {
        return this->isPrelude_;
    }
};

struct Export {
    FullyQualifiedName fqn;
    core::LocOffsets loc;

    explicit Export(FullyQualifiedName &&fqn, core::LocOffsets loc) : fqn(std::move(fqn)), loc(loc) {}

    const std::vector<core::NameRef> &parts() const {
        return fqn.parts;
    }

    // Lex sort by name.
    static bool lexCmp(const Export &a, const Export &b);
};

struct VisibleTo {
    MangledName mangledName;
    VisibleToType type;
    core::LocOffsets loc;

    VisibleTo(MangledName mangledName, VisibleToType type, core::LocOffsets loc)
        : mangledName(mangledName), type(type), loc(loc){};
};

struct PackageReferenceInfo {
    bool importNeeded;
    bool causesModularityError;
};

class PackageInfo {
public:
    MangledName mangledName() const {
        return mangledName_;
    }

    absl::Span<const std::string> pathPrefixes() const {
        ENFORCE(exists());
        return absl::MakeSpan(packagePathPrefixes);
    }

    core::Loc fullLoc() const {
        ENFORCE(exists());
        return loc;
    }

    core::Loc declLoc() const {
        ENFORCE(exists());
        return declLoc_;
    }

    bool exists() const {
        return mangledName().exists();
    }

    std::string show(const core::GlobalState &gs) const;
    bool operator==(const PackageInfo &rhs) const;

    bool exportAll() const {
        ENFORCE(exists());
        return exportAll_;
    }

    bool visibleToTests() const {
        ENFORCE(exists());
        return visibleToTests_;
    }

    MangledName mangledName_;

    // loc for the package definition. Full loc, from class to end keyword. Used for autocorrects.
    core::Loc loc;
    // loc for the package definition. Single line (just the class def). Used for error messages.
    core::Loc declLoc_;
    // The possible path prefixes associated with files in the package, including path separator at end.
    std::vector<std::string> packagePathPrefixes = {};
    // The names of each package imported by this package.
    std::vector<Import> importedPackageNames = {};
    // List of exported items that form the body of this package's public API.
    // These are copied into every package that imports this package.
    std::vector<Export> exports_ = {};

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
    std::vector<VisibleTo> visibleTo_ = {};

    // Whether `visible_to` directives should be ignored for test code
    bool visibleToTests_ = false;

    std::optional<std::pair<StrictDependenciesLevel, core::LocOffsets>> strictDependenciesLevel_;
    std::optional<std::pair<core::NameRef, core::LocOffsets>> layer_;
    std::vector<core::LocOffsets> extraDirectives_;
    std::optional<
        std::pair<std::pair<core::StrictLevel, core::LocOffsets>, std::pair<core::StrictLevel, core::LocOffsets>>>
        min_typed_level_;

    // Map from pkg -> {list of files in this package that reference pkg, whether this package is missing an import
    // for pkg}
    UnorderedMap<core::packages::MangledName, std::pair<UnorderedSet<core::FileRef>, PackageReferenceInfo>>
        referencedPackages = {};

    std::optional<std::pair<StrictDependenciesLevel, core::LocOffsets>> strictDependenciesLevel() const {
        ENFORCE(exists());
        return strictDependenciesLevel_;
    }

    std::optional<std::pair<core::NameRef, core::LocOffsets>> layer() const {
        ENFORCE(exists());
        return layer_;
    }

    // ID of the strongly-connected component that this package is in, according to its graph of import dependencies
    std::optional<int> sccID_;

    // The id of the SCC that this package's normal imports belong to.
    //
    // WARNING: Modifying the contents of the package DB after this operation will cause this id to go out of
    // date.
    std::optional<int> sccID() const {
        ENFORCE(exists());
        return sccID_;
    }

    // ID of the strongly-connected component that this package's tests are in, according to its graph of import
    // dependencies
    std::optional<int> testSccID_;

    // The ID of the SCC that this package's tests belong to. This ID is only useful in the context of the package graph
    // condensation graph.
    //
    // WARNING: Modifying the contents of the package DB after this operation will cause this id to go out of
    // date.
    std::optional<int> testSccID() const {
        ENFORCE(exists());
        return testSccID_;
    }

    static PackageInfo &from(core::GlobalState &gs, MangledName pkg);

    static const PackageInfo &from(const core::GlobalState &gs, MangledName pkg);

    std::unique_ptr<PackageInfo> deepCopy() const;

    PackageInfo(MangledName mangledName, core::Loc loc, core::Loc declLoc_)
        : mangledName_(mangledName), loc(loc), declLoc_(declLoc_) {}
    explicit PackageInfo(const PackageInfo &) = default;
    PackageInfo &operator=(const PackageInfo &) = delete;

    int orderImports(const core::GlobalState &gs, const PackageInfo &a, bool aIsTestImport, const PackageInfo &b,
                     bool bIsTestImport) const;

    int orderByStrictness(const PackageDB &packageDB, const PackageInfo &a, const PackageInfo &b) const;

    int orderByAlphabetical(const core::GlobalState &gs, const PackageInfo &a, const PackageInfo &b) const;

    // autocorrects

    std::optional<core::AutocorrectSuggestion> addImport(const core::GlobalState &gs, const PackageInfo &info,
                                                         ImportType importType) const;

    std::optional<core::AutocorrectSuggestion> addExport(const core::GlobalState &gs,
                                                         const core::SymbolRef newExport) const;

    std::vector<VisibleTo> visibleTo() const {
        return visibleTo_;
    }

    std::optional<ImportType> importsPackage(MangledName mangledName) const;

    // Is it a layering violation to import otherPkg from this package?
    bool causesLayeringViolation(const PackageDB &packageDB, const PackageInfo &otherPkg) const;

    bool causesLayeringViolation(const PackageDB &packageDB, core::NameRef otherPkgLayer) const;

    // What is the minimum strict dependencies level that this package's imports must have?
    StrictDependenciesLevel minimumStrictDependenciesLevel() const;

    // Returns a string representing the path to the given package from this package, if it exists. Note: this only
    // looks at non-test imports.
    std::optional<std::string> pathTo(const core::GlobalState &gs, const MangledName dest) const;

    bool isPreludePackage_ = false;

    // True when the package is marked with a `prelude_package` annotation. This requires that the package only import
    // other prelude packages and markes it as an implicit dependency of all non-prelude packages.
    bool isPreludePackage() const {
        return this->isPreludePackage_;
    }

    // Track that this package references `package` in `file`
    void trackPackageReference(const core::FileRef file, const core::packages::MangledName package,
                               const PackageReferenceInfo packageReferenceInfo);

    // Remove knowledge of what this package is in `file`.
    // We do this so that when VisibilityChecker is re-run over `file`, we can delete stale information.
    void untrackPackageReferencesFor(const core::FileRef file);
};

} // namespace sorbet::core::packages
#endif
