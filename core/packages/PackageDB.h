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
    static constexpr NameRef TEST_NAMESPACE = core::Names::Constants::Test();

    MangledName enterPackage(std::unique_ptr<PackageInfo> pkg);

    // Fetch the mangled package name for a file, returning a MangledName that doesn't exist if there is no
    // associated package for the file.
    const MangledName getPackageNameForFile(FileRef file) const;

    // Use the structure of the symbol table to determine which package owns a given symbol, instead
    // of doing it based on location information.
    //
    // The file of the canonical loc() of a symbol used to be how we would figure out which package
    // owns a Symbol. This is brittle and depends heavily on things like file order and whether
    // a class symbol `!isDeclared`, among other things. Using the ownership structure of the
    // symbol table is a more reliable way to determine ownership.
    //
    // TODO(jez) Update this comment when the PackageInfo -> symbol table migration is finished.
    // TODO(jez) This should probably return something like a PackageRef in the future.
    const MangledName getPackageNameForSymbol(const GlobalState &gs, SymbolRef sym) const;

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

private:
    bool enabled_ = false;
    std::vector<std::string> extraPackageFilesDirectoryUnderscorePrefixes_;
    std::vector<std::string> extraPackageFilesDirectorySlashPrefixes_;
    std::string errorHint_;
    std::vector<std::string> skipRBIExportEnforcementDirs_;
    std::vector<MangledName> allowRelaxedPackagerChecksFor_;

    // This vector is kept in sync with the size of the file table in the global state by
    // `Packager::setPackageNameOnFiles`. A `FileRef` being out of bounds in this vector is treated as the file having
    // no associated package.
    std::vector<MangledName> packageForFile_;

    UnorderedMap<MangledName, std::unique_ptr<packages::PackageInfo>> packages_;
    UnorderedMap<std::string, MangledName> packagesByPathPrefix;
    std::vector<MangledName> mangledNames;

    bool frozen = true;
    std::thread::id writerThread;

    const MangledName getPackageNameForSymbolImpl(const GlobalState &gs, ClassOrModuleRef klass,
                                                  std::vector<NameRef> &breadcrumbs) const;
    const MangledName getPackageNameFromBreadcrumbs(const GlobalState &gs,
                                                    const std::vector<NameRef> &breadcrumbs) const;

    friend class UnfreezePackages;
};

} // namespace sorbet::core::packages
#endif // SORBET_CORE_PACKAGES_PACKAGEDB_H
