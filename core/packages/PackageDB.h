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

    // Fetch the mangled package name for a file, returning a core::NameRef::noName() that doesn't exist if there is no
    // associated packge for the file.
    const NameRef getPackageNameForFile(FileRef file) const;

    // Set the associated package for the file.
    void setPackageNameForFile(FileRef file, NameRef mangledName);

    const PackageInfo &getPackageForFile(const core::GlobalState &gs, core::FileRef file) const;
    const PackageInfo &getPackageInfo(core::NameRef mangledName) const;

    // Lookup `PackageInfo` from the string representation of the un-mangled package name.
    const PackageInfo &getPackageInfo(const core::GlobalState &gs, std::string_view str) const;

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
    const std::vector<std::string> &extraPackageFilesDirectoryUnderscorePrefixes() const;
    const std::vector<std::string> &extraPackageFilesDirectorySlashPrefixes() const;
    const std::vector<std::string> &skipRBIExportEnforcementDirs() const;

    const std::string_view errorHint() const;
    bool skipImportVisibilityCheckFor(const core::NameRef mangledName) const;

private:
    std::vector<NameRef> secondaryTestPackageNamespaceRefs_;
    std::vector<std::string> extraPackageFilesDirectoryUnderscorePrefixes_;
    std::vector<std::string> extraPackageFilesDirectorySlashPrefixes_;
    std::string errorHint_;
    std::vector<std::string> skipRBIExportEnforcementDirs_;
    std::vector<NameRef> skipImportVisibilityCheckFor_;

    // This vector is kept in sync with the size of the file table in the global state by
    // `Packager::setPackageNameOnFiles`. A `FileRef` being out of bounds in this vector is treated as the file having
    // no associated package.
    std::vector<NameRef> packageForFile_;

    UnorderedMap<core::NameRef, std::unique_ptr<packages::PackageInfo>> packages_;
    UnorderedMap<std::string, core::NameRef> packagesByPathPrefix;
    std::vector<NameRef> mangledNames;

    bool frozen = true;
    std::thread::id writerThread;

    friend class UnfreezePackages;
};

} // namespace sorbet::core::packages
#endif // SORBET_CORE_PACKAGES_PACKAGEDB_H
