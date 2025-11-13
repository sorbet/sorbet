#ifndef SORBET_CORE_PACKAGES_PACKAGEDB_H
#define SORBET_CORE_PACKAGES_PACKAGEDB_H

#include "absl/types/span.h"

#include "common/common.h"
#include "core/Files.h"
#include "core/Names.h"
#include "core/packages/Condensation.h"
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
    static constexpr NameRef TEST_NAMESPACE = core::Names::Constants::Test();

    MangledName enterPackage(std::unique_ptr<PackageInfo> pkg);

    // Fetch the mangled package name for a file, returning a MangledName that doesn't exist if there is no
    // associated package for the file.
    const MangledName getPackageNameForFile(FileRef file) const;

    // Set the associated package for the file.
    void setPackageNameForFile(FileRef file, MangledName mangledName);

    MangledName findPackageByPath(const core::GlobalState &gs, core::FileRef file) const;
    const PackageInfo &getPackageInfo(MangledName mangledName) const;
    PackageInfo *getPackageInfoNonConst(MangledName mangledName);

    // Get mangled names for all packages.
    // Packages are ordered lexicographically with respect to the NameRef's that make up their
    // namespaces.
    absl::Span<const MangledName> packages() const;

    PackageDB deepCopy() const;

    UnfreezePackages unfreeze();

    PackageDB() = default;
    PackageDB(const PackageDB &) = delete;
    PackageDB(PackageDB &&) = default;
    PackageDB &operator=(const PackageDB &) = delete;
    PackageDB &operator=(PackageDB &&) = default;

    // Whether the --stripe-packages mode is active.
    bool enabled() const {
        return this->enabled_;
    }

    // Whether the --gen-packages mode is active.
    bool genPackages() const {
        return this->genPackages_;
    }

    absl::Span<const std::string> extraPackageFilesDirectoryUnderscorePrefixes() const;
    absl::Span<const std::string> extraPackageFilesDirectorySlashDeprecatedPrefixes() const;
    absl::Span<const std::string> extraPackageFilesDirectorySlashPrefixes() const;
    absl::Span<const std::string> skipRBIExportEnforcementDirs() const;
    // Possible layers for packages to be in. The layers are ordered lowest to highest.
    // Ie. {'util', 'app'} means that code in `app` can call code in `util`, but code in `util` cannot call code in
    // `app`.
    absl::Span<const core::NameRef> layers() const;
    const int layerIndex(core::NameRef layer) const;
    // TODO(neil): this is more than just layering, also checks for strict dependencies. Maybe rename?
    const bool enforceLayering() const;

    const std::string_view errorHint() const;

    // Expects to be called after packages have been defined, so that string package names provided
    // at the command line can be resolved to actual packages.
    void resolvePackagesWithRelaxedChecks(GlobalState &gs);

    bool allowRelaxedPackagerChecksFor(const MangledName mangledName) const;

    // Overwrite the condensation graph for the current package set. This method is only meant to be used from
    // `ComputePackageSCCs::run`.
    //
    // WARNING: Modifying the contents of the package DB after this operation will cause the condensation to go out of
    // date.
    void setCondensation(Condensation &&condensation);

    // Fetch the condensation graph for queries.
    const Condensation &condensation() const;

private:
    bool enabled_ = false;
    bool genPackages_ = false;
    std::vector<std::string> extraPackageFilesDirectoryUnderscorePrefixes_;
    std::vector<std::string> extraPackageFilesDirectorySlashDeprecatedPrefixes_;
    std::vector<std::string> extraPackageFilesDirectorySlashPrefixes_;
    std::string errorHint_;
    std::vector<std::string> skipRBIExportEnforcementDirs_;
    std::vector<std::string> allowRelaxedPackagerChecksFor_;
    UnorderedSet<MangledName> packagesWithRelaxedChecks_;
    std::vector<core::NameRef> layers_;

    // This vector is kept in sync with the size of the file table in the global state by `setPackageNameForFile`.
    // A `FileRef` being out of bounds in this vector is treated as the file having no associated package.
    std::vector<MangledName> packageForFile_;

    UnorderedMap<MangledName, std::unique_ptr<packages::PackageInfo>> packages_;
    UnorderedMap<std::string, MangledName> packagesByPathPrefix;
    std::vector<MangledName> mangledNames;

    bool frozen = true;
    std::thread::id writerThread;

    Condensation condensation_;

    friend class UnfreezePackages;
};

} // namespace sorbet::core::packages
#endif // SORBET_CORE_PACKAGES_PACKAGEDB_H
