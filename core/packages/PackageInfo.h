#ifndef SORBET_CORE_PACKAGES_PACKAGEINFO_H
#define SORBET_CORE_PACKAGES_PACKAGEINFO_H

#include "absl/types/span.h"

#include "core/NameRef.h"
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
    Test,
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

struct VisibleTo {
    ClassOrModuleRef sym;
    VisibleToType visibleToType;

    VisibleTo(ClassOrModuleRef sym, VisibleToType visibleToType) : sym(sym), visibleToType(visibleToType){};
};

class PackageInfo {
public:
    virtual MangledName mangledName() const = 0;
    virtual absl::Span<const core::NameRef> fullName() const = 0;
    virtual absl::Span<const std::string> pathPrefixes() const = 0;
    virtual absl::Span<const VisibleTo> visibleTo() const = 0;
    virtual std::unique_ptr<PackageInfo> deepCopy() const = 0;
    virtual std::optional<std::pair<core::packages::StrictDependenciesLevel, core::LocOffsets>>
    strictDependenciesLevel() const = 0;
    virtual std::optional<std::pair<core::NameRef, core::LocOffsets>> layer() const = 0;
    virtual std::optional<int> sccID() const = 0;
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
                                                                 bool isTestImport) const = 0;
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

    // Utilities:

    static bool lexCmp(absl::Span<const core::NameRef> lhs, absl::Span<const core::NameRef> rhs);
};

} // namespace sorbet::core::packages
#endif
