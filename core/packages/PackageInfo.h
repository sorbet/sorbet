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

struct Import {
    MangledName name;
    ImportType type;
    core::LocOffsets loc;

    Import(core::packages::MangledName &&name, core::packages::ImportType type, core::LocOffsets loc)
        : name(std::move(name)), type(type), loc(loc) {}

    bool isTestImport() const {
        return type != core::packages::ImportType::Normal;
    }
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

// TODO(jez) Why is this struct different from the `struct VisibleTo` defined in packager.cc?
struct VisibleTo {
    MangledName packageName;
    VisibleToType visibleToType;

    VisibleTo(MangledName packageName, VisibleToType visibleToType)
        : packageName(std::move(packageName)), visibleToType(visibleToType){};
};

class PackageInfo {
public:
    virtual MangledName mangledName() const = 0;
    virtual absl::Span<const core::NameRef> fullName() const = 0;
    virtual absl::Span<const std::string> pathPrefixes() const = 0;
    virtual std::vector<VisibleTo> visibleTo() const = 0;
    virtual std::unique_ptr<PackageInfo> deepCopy() const = 0;
    virtual std::optional<std::pair<core::packages::StrictDependenciesLevel, core::LocOffsets>>
    strictDependenciesLevel() const = 0;
    virtual std::optional<std::pair<core::NameRef, core::LocOffsets>> layer() const = 0;
    virtual std::optional<std::pair<core::StrictLevel, core::LocOffsets>> min_typed_level() const = 0;
    virtual std::optional<std::pair<core::StrictLevel, core::LocOffsets>> tests_min_typed_level() const = 0;

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
    core::ClassOrModuleRef getRootSymbolForAutocorrectSearch(const core::GlobalState &gs,
                                                             core::SymbolRef suggestionScope) const;

    core::ClassOrModuleRef getPackageScope(const core::GlobalState &gs) const;
    core::ClassOrModuleRef getPackageTestScope(const core::GlobalState &gs) const;

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

    virtual bool ownsSymbol(const core::GlobalState &gs, core::SymbolRef symbol) const = 0;
    virtual bool exportAll() const = 0;
    virtual bool visibleToTests() const = 0;
};

} // namespace sorbet::core::packages
#endif
