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
    friend class core::GlobalState;

public:
    NameRef enterPackage(std::unique_ptr<PackageInfo> pkg);
    NameRef lookupPackage(NameRef pkgMangledName) const;

    const PackageInfo &getPackageForFile(const core::GlobalState &gs, core::FileRef file) const;
    const PackageInfo &getPackageInfo(core::NameRef mangledName) const;

    bool empty() const;
    // Get mangled names for all packages.
    // Packages are ordered lexicographically with respect to the NameRef's that make up their
    // namespaces.
    const std::vector<core::NameRef> &packages() const;

    PackageDB deepCopy() const;

    UnfreezePackages unfreeze();

    PackageDB() = default;
    PackageDB(const PackageDB &) = delete;
    PackageDB(PackageDB &&) = default;
    PackageDB &operator=(const PackageDB &) = delete;
    PackageDB &operator=(PackageDB &&) = default;

    const std::vector<core::NameRef> &secondaryTestPackageNamespaceRefs() const;
    const std::vector<std::string> &extraPackageFilesDirectoryPrefixes() const;

    const std::string_view errorHint() const;

    const std::vector<NameRef> &layerNames() const;

private:
    std::vector<NameRef> secondaryTestPackageNamespaceRefs_;
    std::vector<std::string> extraPackageFilesDirectoryPrefixes_;
    std::vector<NameRef> layerNames_;
    std::string errorHint_;

    UnorderedMap<core::NameRef, std::unique_ptr<packages::PackageInfo>> packages_;
    UnorderedMap<std::string, core::NameRef> packagesByPathPrefix;
    std::vector<NameRef> mangledNames;

    bool frozen = true;
    std::thread::id writerThread;

    friend class UnfreezePackages;
    friend class Layer;
};

} // namespace sorbet::core::packages
#endif // SORBET_CORE_PACKAGES_PACKAGEDB_H
