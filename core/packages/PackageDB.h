#ifndef SORBET_CORE_PACKAGES_PACKAGEDB_H
#define SORBET_CORE_PACKAGES_PACKAGEDB_H

#include "common/common.h"
#include "core/AutocorrectSuggestion.h"
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
    static constexpr NameRef TEST_NAMESPACE = core::Names::Constants::Test();

    MangledName enterPackage(std::unique_ptr<PackageInfo> pkg);

    // Fetch the mangled package name for a file, returning a MangledName that doesn't exist if there is no
    // associated package for the file.
    const MangledName getPackageNameForFile(FileRef file) const;

    // Set the associated package for the file.
    void setPackageNameForFile(FileRef file, MangledName mangledName);

    const PackageInfo &getPackageForFile(const core::GlobalState &gs, core::FileRef file) const;
    const PackageInfo &getPackageInfo(MangledName mangledName) const;

    // Lookup `PackageInfo` from the string representation of the un-mangled package name.
    const PackageInfo &getPackageInfo(const core::GlobalState &gs, std::string_view str) const;

    // Get mangled names for all packages.
    // Packages are ordered lexicographically with respect to the NameRef's that make up their
    // namespaces.
    const std::vector<MangledName> &packages() const;

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

    const std::vector<std::string> &extraPackageFilesDirectoryUnderscorePrefixes() const;
    const std::vector<std::string> &extraPackageFilesDirectorySlashPrefixes() const;
    const std::vector<std::string> &skipRBIExportEnforcementDirs() const;

    const std::string_view errorHint() const;
    bool allowRelaxedPackagerChecksFor(const MangledName mangledName) const;

    const std::vector<core::SymbolRef> extraExportsFor(packages::MangledName package) const;
    void registerExtraAutocorrectFor(const packages::MangledName package, const core::SymbolRef toExport,
                                     core::FileRef fref);
    void removeExtraAutocorrectFor(const core::FileRef fref);

private:
    bool enabled_ = false;
    std::vector<std::string> extraPackageFilesDirectoryUnderscorePrefixes_;
    std::vector<std::string> extraPackageFilesDirectorySlashPrefixes_;
    std::string errorHint_;
    std::vector<std::string> skipRBIExportEnforcementDirs_;
    std::vector<MangledName> allowRelaxedPackagerChecksFor_;

    // package where we *report* errors => [(File where error has happened, Symbol for a package to export)]
    UnorderedMap<packages::MangledName, UnorderedSet<std::pair<core::FileRef, core::SymbolRef>>> requiresExport_;

    // maps files where error *occur* to the package files where the error is repoted
    // it's used to invalidate `requiresExport_`
    UnorderedMap<core::FileRef, std::vector<packages::MangledName>> errorSources_;

    // This vector is kept in sync with the size of the file table in the global state by
    // `Packager::setPackageNameOnFiles`. A `FileRef` being out of bounds in this vector is treated as the file having
    // no associated package.
    std::vector<MangledName> packageForFile_;

    UnorderedMap<MangledName, std::unique_ptr<packages::PackageInfo>> packages_;
    UnorderedMap<std::string, MangledName> packagesByPathPrefix;
    std::vector<MangledName> mangledNames;

    bool frozen = true;
    std::thread::id writerThread;

    friend class UnfreezePackages;
};

} // namespace sorbet::core::packages
#endif // SORBET_CORE_PACKAGES_PACKAGEDB_H
