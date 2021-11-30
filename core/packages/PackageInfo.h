#ifndef SORBET_CORE_PACKAGES_PACKAGEINFO_H
#define SORBET_CORE_PACKAGES_PACKAGEINFO_H

#include "core/NameRef.h"
#include "core/SymbolRef.h"
#include <vector>

namespace sorbet::core {
class Context;
class Loc;
} // namespace sorbet::core

namespace sorbet::core::packages {
class PackageInfo {
public:
    virtual core::NameRef mangledName() const = 0;
    virtual const std::vector<core::NameRef> &fullName() const = 0;
    virtual const std::vector<std::string> &pathPrefixes() const = 0;
    virtual std::unique_ptr<PackageInfo> deepCopy() const = 0;
    virtual core::Loc definitionLoc() const = 0;
    virtual bool exists() const final;

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
};
} // namespace sorbet::core::packages
#endif
