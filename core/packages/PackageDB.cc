#include "core/packages/PackageDB.h"
#include "absl/strings/str_split.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::core::packages {

namespace {
static const PackageInfo NONE_PKG(MangledName(), Loc::none(), Loc::none());
} // namespace

UnfreezePackages::UnfreezePackages(PackageDB &db) : db(db) {
    ENFORCE(db.frozen);
    db.writerThread = this_thread::get_id();
    db.frozen = false;
}

UnfreezePackages::~UnfreezePackages() {
    ENFORCE(!db.frozen);
    db.writerThread = std::thread::id();
    db.frozen = true;
}

MangledName PackageDB::enterPackage(unique_ptr<PackageInfo> pkg) {
    ENFORCE(!frozen);
    ENFORCE(writerThread == this_thread::get_id(), "PackageDB writes are not thread safe");
    auto nr = pkg->mangledName();
    auto [it, newlyInserted] = packages_.insert({nr, nullptr});
    if (newlyInserted) {
        for (const auto &prefix : pkg->pathPrefixes()) {
            packagesByPathPrefix[prefix] = nr;
        }
        mangledNames.emplace_back(nr);
    } else {
        // Package files do not have full featured content hashing. If the contents of one changes
        // we always run slow-path and fully rebuild the set of packages. In some cases, the LSP
        // fast-path may re-run on an unchanged package file. Sanity check to ensure the loc and
        // prefixes are the same.
        ENFORCE(it->second->declLoc() == pkg->declLoc());
        ENFORCE(it->second->pathPrefixes() == pkg->pathPrefixes());
    }
    it->second = move(pkg);
    ENFORCE(mangledNames.size() == packages_.size());
    return nr;
}

const MangledName PackageDB::getPackageNameForFile(FileRef file) const {
    if (this->packageForFile_.size() <= file.id()) {
        return MangledName();
    }

    return this->packageForFile_[file.id()];
}

void PackageDB::setPackageNameForFile(FileRef file, MangledName mangledName) {
    if (this->packageForFile_.size() <= file.id()) {
        this->packageForFile_.resize(file.id() + 1, MangledName());
    }

    this->packageForFile_[file.id()] = mangledName;
}

MangledName PackageDB::findPackageByPath(const core::GlobalState &gs, core::FileRef file) const {
    ENFORCE(enabled_);

    // Note about safety: we're only using the file data for two pieces of information: the file path and the
    // sourceType. The path is present even on unloaded files, and the sourceType we're interested in is `Package`,
    // which will have been loaded by a previous step for the packageDB to be valid.
    //
    // See https://github.com/sorbet/sorbet/pull/5291 for more information.
    auto &fileData = file.dataAllowingUnsafe(gs);

    string_view path = fileData.path();
    int curPrefixPos = path.find_last_of('/');
    while (curPrefixPos > 0) {
        const auto &it = packagesByPathPrefix.find(path.substr(0, curPrefixPos + 1));
        if (it != packagesByPathPrefix.end()) {
            return it->second;
        }

        if (fileData.isPackage(gs)) {
            // When looking up a `__package.rb` file do not search parent directories
            break;
        }
        curPrefixPos = path.find_last_of('/', curPrefixPos - 1);
    }
    return MangledName();
}

const PackageInfo &PackageDB::getPackageInfo(MangledName mangledName) const {
    auto it = packages_.find(mangledName);
    if (it == packages_.end()) {
        return NONE_PKG;
    }
    return *it->second;
}

PackageInfo *PackageDB::getPackageInfoNonConst(MangledName mangledName) {
    auto it = packages_.find(mangledName);
    if (it == packages_.end()) {
        return nullptr;
    }
    return it->second.get();
}

absl::Span<const MangledName> PackageDB::packages() const {
    return absl::MakeSpan(mangledNames);
}

absl::Span<const string> PackageDB::skipRBIExportEnforcementDirs() const {
    return absl::MakeSpan(skipRBIExportEnforcementDirs_);
}

absl::Span<const core::NameRef> PackageDB::layers() const {
    return absl::MakeSpan(layers_);
}

const int PackageDB::layerIndex(core::NameRef layer) const {
    auto findResult = absl::c_find(layers_, layer);
    ENFORCE(findResult != layers_.end());
    return std::distance(layers_.begin(), findResult);
}

const bool PackageDB::enforceLayering() const {
    return !layers_.empty();
}

absl::Span<const string> PackageDB::extraPackageFilesDirectoryUnderscorePrefixes() const {
    return absl::MakeSpan(extraPackageFilesDirectoryUnderscorePrefixes_);
}

absl::Span<const string> PackageDB::extraPackageFilesDirectorySlashDeprecatedPrefixes() const {
    return absl::MakeSpan(extraPackageFilesDirectorySlashDeprecatedPrefixes_);
}

absl::Span<const string> PackageDB::extraPackageFilesDirectorySlashPrefixes() const {
    return absl::MakeSpan(extraPackageFilesDirectorySlashPrefixes_);
}

const string_view PackageDB::errorHint() const {
    return errorHint_;
}

void PackageDB::resolvePackagesWithRelaxedChecks(GlobalState &gs) {
    UnorderedSet<MangledName> packagesWithRelaxedChecks;
    for (const auto &pkgName : allowRelaxedPackagerChecksFor_) {
        auto pkgNameParts = absl::StrSplit(pkgName, "::");
        auto mangledName = MangledName::lookupMangledName(gs, pkgNameParts);
        packagesWithRelaxedChecks.emplace(mangledName);
    }
    this->packagesWithRelaxedChecks_ = move(packagesWithRelaxedChecks);
}

bool PackageDB::allowRelaxedPackagerChecksFor(MangledName mangledName) const {
    return this->packagesWithRelaxedChecks_.contains(mangledName);
}

PackageDB PackageDB::deepCopy() const {
    ENFORCE(frozen);
    PackageDB result;

    // --- data ---
    result.packages_.reserve(this->packages_.size());
    for (auto const &[nr, pkgInfo] : this->packages_) {
        result.packages_[nr] = pkgInfo->deepCopy();
    }
    result.packagesByPathPrefix = this->packagesByPathPrefix;
    // This assumes that the GlobalState this PackageDB is getting copied into also has these
    // interned mangledName NameRefs at the same IDs as the current PackageDB.
    result.mangledNames = this->mangledNames;

    // --- options ---
    result.enabled_ = this->enabled_;
    result.genPackages_ = this->genPackages_;
    result.deleteUnusedImports_ = this->deleteUnusedImports_;
    result.extraPackageFilesDirectoryUnderscorePrefixes_ = this->extraPackageFilesDirectoryUnderscorePrefixes_;
    result.extraPackageFilesDirectorySlashDeprecatedPrefixes_ =
        this->extraPackageFilesDirectorySlashDeprecatedPrefixes_;
    result.extraPackageFilesDirectorySlashPrefixes_ = this->extraPackageFilesDirectorySlashPrefixes_;
    result.skipRBIExportEnforcementDirs_ = this->skipRBIExportEnforcementDirs_;
    // This assumes that the GlobalState this PackageDB is getting copied into also has these
    // interned layer NameRefs at the same IDs as the current PackageDB.
    result.layers_ = this->layers_;
    result.allowRelaxedPackagerChecksFor_ = this->allowRelaxedPackagerChecksFor_;
    // Likewise, this assumes that we've entered MangledNames in the target GlobalState, but ALSO
    // that we've copied symbols, because MangledNames store ClassOrModuleRef's now
    result.packagesWithRelaxedChecks_ = this->packagesWithRelaxedChecks_;
    result.errorHint_ = this->errorHint_;

    return result;
}

UnfreezePackages PackageDB::unfreeze() {
    return UnfreezePackages(*this);
}

void PackageDB::setCondensation(Condensation &&condensation) {
    this->condensation_ = std::move(condensation);
}

const Condensation &PackageDB::condensation() const {
    return this->condensation_;
}

} // namespace sorbet::core::packages
