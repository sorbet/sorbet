#ifndef SORBET_CORE_PACKAGES_PACKAGEINFO_H
#define SORBET_CORE_PACKAGES_PACKAGEINFO_H

#include <vector>

namespace sorbet::core {
class NameRef;
class Loc;
} // namespace sorbet::core

namespace sorbet::core::packages {
class PackageInfo {
public:
    virtual core::NameRef mangledName() const = 0;
    virtual const std::vector<std::string> &pathPrefixes() const = 0;
    virtual std::unique_ptr<PackageInfo> deepCopy() const = 0;
    virtual core::Loc definitionLoc() const = 0;

    virtual ~PackageInfo() = 0;
    PackageInfo() = default;
    PackageInfo(PackageInfo &) = delete;
    explicit PackageInfo(const PackageInfo &) = default;
    PackageInfo &operator=(PackageInfo &&) = delete;
    PackageInfo &operator=(const PackageInfo &) = delete;
};
} // namespace sorbet::core::packages
#endif
