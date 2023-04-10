#ifndef SORBET_CORE_PACKAGES_PACKAGEINFO_H
#define SORBET_CORE_PACKAGES_PACKAGEINFO_H

#include "core/NameRef.h"
#include "core/SymbolRef.h"
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

enum class ImportType {
    Normal,
    Test,
};

class PackageInfo {
public:
    virtual core::NameRef mangledName() const = 0;
    virtual const std::vector<core::NameRef> &fullName() const = 0;
    virtual const std::vector<std::string> &pathPrefixes() const = 0;
    virtual std::vector<std::vector<core::NameRef>> exports() const = 0;
    virtual std::vector<std::vector<core::NameRef>> imports() const = 0;
    virtual std::vector<std::vector<core::NameRef>> testImports() const = 0;
    virtual std::vector<std::vector<core::NameRef>> visibleTo() const = 0;
    virtual std::unique_ptr<PackageInfo> deepCopy() const = 0;
    virtual core::Loc fullLoc() const = 0;
    virtual core::Loc declLoc() const = 0;
    virtual bool exists() const final;
    std::string show(const core::GlobalState &gs) const;
    core::ClassOrModuleRef getRootSymbolForAutocorrectSearch(const core::GlobalState &gs,
                                                             core::SymbolRef suggestionScope) const;

    core::ClassOrModuleRef getPackageScope(const core::GlobalState &gs) const;
    core::ClassOrModuleRef getPackageTestScope(const core::GlobalState &gs) const;

    virtual std::optional<ImportType> importsPackage(core::NameRef mangledName) const = 0;

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
    virtual bool legacyAutoloaderCompatibility() const = 0;
    virtual bool exportAll() const = 0;

    // Utilities:

    static bool lexCmp(const std::vector<core::NameRef> &lhs, const std::vector<core::NameRef> &rhs);
};

// Information about the imports of a package. The imports are split into two categories, packages whose name falls
// within the namespace of `package`, and everything else. The reason for pre-processing the imports this way is that it
// simplifies some work when stubbing constants for rbi generation.
class ImportInfo final {
public:
    // The mangled name of the package whose imports are described.
    core::NameRef package;

    // Imported packages whose name is a prefix of `package`. For example, if the package `Foo::Bar` imports `Foo` that
    // package's name would be in `parentImports` because its name is a prefix of `Foo::Bar`.
    std::vector<core::NameRef> parentImports;

    // The mangled names of packages that are imported by this package, minus any imports that fall in the parent
    // namespace of this package.
    std::vector<core::NameRef> regularImports;

    static ImportInfo fromPackage(const core::GlobalState &gs, const PackageInfo &info);
};

} // namespace sorbet::core::packages
#endif
