#ifndef SORBET_CORE_PACKAGES_PACKAGEDB_H
#define SORBET_CORE_PACKAGES_PACKAGEDB_H

#include "common/common.h"
#include "core/Files.h"
#include "core/Names.h"
#include "core/packages/PackageInfo.h"

namespace sorbet::core::packages {

class PackageDB;

class UnfreezePackages final {
public:
    PackageDB &db;

    UnfreezePackages(PackageDB &);
    ~UnfreezePackages();
};

class PackageDB final {
public:
    NameRef enterPackage(std::unique_ptr<PackageInfo> pkg);
    NameRef lookupPackage(NameRef pkgMangledName) const;

    const PackageInfo &getPackageForFile(const core::GlobalState &gs, core::FileRef file) const;
    const PackageInfo &getPackageInfo(core::NameRef mangledName) const;

    std::unique_ptr<PackageDB> deepCopy() const;

    UnfreezePackages unfreeze();

    PackageDB() = default;
    PackageDB(const PackageDB &) = delete;
    PackageDB(PackageDB &&) = default;
    PackageDB &operator=(const PackageDB &) = delete;
    PackageDB &operator=(PackageDB &&) = default;

private:
    UnorderedMap<core::NameRef, std::unique_ptr<packages::PackageInfo>> packages;
    UnorderedMap<std::string, core::NameRef> packagesByPathPrefix;
    bool frozen = true;
    std::thread::id writerThread;

    friend class UnfreezePackages;
};

} // namespace sorbet::core::packages
#endif // SORBET_CORE_PACKAGES_PACKAGEDB_H
