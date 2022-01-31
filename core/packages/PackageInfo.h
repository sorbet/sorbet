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
class PackageInfo {
public:
    virtual core::NameRef mangledName() const = 0;
    virtual const std::vector<core::NameRef> &fullName() const = 0;
    virtual const std::vector<std::string> &pathPrefixes() const = 0;
    virtual std::vector<std::vector<core::NameRef>> exports() const = 0;
    virtual std::vector<std::vector<core::NameRef>> testExports() const = 0;
    virtual std::vector<std::vector<core::NameRef>> imports() const = 0;
    virtual std::vector<std::vector<core::NameRef>> testImports() const = 0;
    virtual std::unique_ptr<PackageInfo> deepCopy() const = 0;
    virtual core::Loc definitionLoc() const = 0;
    virtual bool exists() const final;

    // autocorrects
    virtual std::optional<core::AutocorrectSuggestion> addImport(const core::GlobalState &gs, const PackageInfo &pkg,
                                                                 bool isTestImport) const = 0;
    virtual std::optional<core::AutocorrectSuggestion>
    addExport(const core::GlobalState &gs, const core::SymbolRef name, bool isPrivateTestExport) const = 0;

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
    virtual std::vector<MissingExportMatch> findMissingExports(core::Context ctx, core::SymbolRef scope,
                                                               core::NameRef name) const = 0;
    virtual bool ownsSymbol(const core::GlobalState &gs, core::SymbolRef symbol) const = 0;

    // Utilities:

    static bool isPackageModule(const core::GlobalState &gs, core::ClassOrModuleRef klass);

    static bool lexCmp(const std::vector<core::NameRef> &lhs, const std::vector<core::NameRef> &rhs);
};
} // namespace sorbet::core::packages
#endif
