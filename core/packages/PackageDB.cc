#include "core/packages/PackageDB.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "common/sort.h"
#include "core/AutocorrectSuggestion.h"
#include "core/GlobalState.h"
#include "core/Loc.h"

using namespace std;

namespace sorbet::core::packages {

namespace {
class NonePackage final : public PackageInfo {
public:
    core::NameRef mangledName() const {
        return NameRef::noName();
    }

    const vector<core::NameRef> &fullName() const {
        notImplemented();
        return emptyName;
    }

    const vector<string> &pathPrefixes() const {
        notImplemented();
        return prefixes;
    }

    unique_ptr<PackageInfo> deepCopy() const {
        notImplemented();
        return make_unique<NonePackage>();
    }

    Loc fullLoc() const {
        notImplemented();
        return Loc::none();
    }

    Loc declLoc() const {
        notImplemented();
        return Loc::none();
    }

    std::optional<core::AutocorrectSuggestion> addImport(const core::GlobalState &gs, const PackageInfo &pkg,
                                                         bool isTestImport) const {
        notImplemented();
        return nullopt;
    }

    std::optional<core::AutocorrectSuggestion> addExport(const core::GlobalState &gs,
                                                         const core::SymbolRef name) const {
        return {};
    }

    bool ownsSymbol(const core::GlobalState &gs, core::SymbolRef symbol) const {
        notImplemented();
        return false;
    }

    bool strictAutoloaderCompatibility() const {
        notImplemented();
        return false;
    }

    std::vector<std::vector<core::NameRef>> exports() const {
        return vector<vector<core::NameRef>>();
    }
    std::vector<std::vector<core::NameRef>> imports() const {
        return vector<vector<core::NameRef>>();
    }
    std::vector<std::vector<core::NameRef>> testImports() const {
        return vector<vector<core::NameRef>>();
    }

    std::optional<ImportType> importsPackage(core::NameRef mangledName) const {
        notImplemented();
        return nullopt;
    }

    ~NonePackage() {}

private:
    const vector<string> prefixes;
    const vector<core::NameRef> emptyName;

    void notImplemented() const {
        ENFORCE(false, "Not implemented for NonePackage");
    }
};
static const NonePackage NONE_PKG;
} // namespace

UnfreezePackages::UnfreezePackages(PackageDB &db) : db(db) {
    ENFORCE(db.frozen);
    db.writerThread = this_thread::get_id();
    db.frozen = false;
}

UnfreezePackages::~UnfreezePackages() {
    ENFORCE(!db.frozen);
    fast_sort(db.mangledNames, [&db = this->db](auto a, auto b) -> bool {
        return PackageInfo::lexCmp(db.getPackageInfo(a).fullName(), db.getPackageInfo(b).fullName());
    });

    db.writerThread = std::thread::id();
    db.frozen = true;
}

NameRef PackageDB::enterPackage(unique_ptr<PackageInfo> pkg) {
    ENFORCE(!frozen);
    ENFORCE(writerThread == this_thread::get_id(), "PackageDB writes are not thread safe");
    auto nr = pkg->mangledName();
    auto prev = packages_.find(nr);
    if (prev == packages_.end()) {
        for (const auto &prefix : pkg->pathPrefixes()) {
            packagesByPathPrefix[prefix] = nr;
        }
        mangledNames.emplace_back(nr);
    } else {
        // Package files do not have full featured content hashing. If the contents of one changes
        // we always run slow-path and fully rebuild the set of packages. In some cases, the LSP
        // fast-path may re-run on an unchanged package file. Sanity check to ensure the loc and
        // prefixes are the same.
        ENFORCE(prev->second->declLoc() == pkg->declLoc());
        ENFORCE(prev->second->pathPrefixes() == pkg->pathPrefixes());
    }
    packages_[nr] = move(pkg);
    ENFORCE(mangledNames.size() == packages_.size());
    return nr;
}

const NameRef PackageDB::getPackageNameForFile(FileRef file) const {
    if (this->packageForFile_.size() <= file.id()) {
        return NameRef::noName();
    }

    return this->packageForFile_[file.id()];
}

void PackageDB::setPackageNameForFile(FileRef file, NameRef mangledName) {
    if (this->packageForFile_.size() <= file.id()) {
        this->packageForFile_.resize(file.id() + 1, NameRef::noName());
    }

    this->packageForFile_[file.id()] = mangledName;
}

const PackageInfo &PackageDB::getPackageForFile(const core::GlobalState &gs, core::FileRef file) const {
    ENFORCE(frozen);

    // If we already have the package name cached, we can skip the slow path below. As this function is const, we cannot
    // update the vector if we fall back on the slow path.
    auto name = this->getPackageNameForFile(file);
    if (name.exists()) {
        return this->getPackageInfo(name);
    }

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
            const auto &pkg = getPackageInfo(it->second);
            ENFORCE(pkg.exists());
            return pkg;
        }

        if (fileData.isPackage()) {
            // When looking up a `__package.rb` file do not search parent directories
            break;
        }
        curPrefixPos = path.find_last_of('/', curPrefixPos - 1);
    }
    return NONE_PKG;
}

const PackageInfo &PackageDB::getPackageInfo(const core::GlobalState &gs, std::string_view nameStr) const {
    auto mangled = absl::StrCat(absl::StrReplaceAll(nameStr, {{"::", "_"}}), core::PACKAGE_SUFFIX);
    auto utf8Name = gs.lookupNameUTF8(mangled);
    if (!utf8Name.exists()) {
        return NONE_PKG;
    }

    auto packagerName = gs.lookupNameUnique(core::UniqueNameKind::Packager, utf8Name, 1);
    if (!packagerName.exists()) {
        return NONE_PKG;
    }

    auto cnst = gs.lookupNameConstant(packagerName);
    if (!cnst.exists()) {
        return NONE_PKG;
    }
    return getPackageInfo(cnst);
}

const PackageInfo &PackageDB::getPackageInfo(core::NameRef mangledName) const {
    auto it = packages_.find(mangledName);
    if (it == packages_.end()) {
        return NONE_PKG;
    }
    return *it->second;
}

bool PackageDB::empty() const {
    return packages_.empty();
}

const vector<core::NameRef> &PackageDB::packages() const {
    return mangledNames;
}

const std::vector<core::NameRef> &PackageDB::secondaryTestPackageNamespaceRefs() const {
    return secondaryTestPackageNamespaceRefs_;
}

const std::vector<std::string> &PackageDB::skipRBIExportEnforcementDirs() const {
    return skipRBIExportEnforcementDirs_;
}

const std::vector<std::string> &PackageDB::extraPackageFilesDirectoryUnderscorePrefixes() const {
    return extraPackageFilesDirectoryUnderscorePrefixes_;
}

const std::vector<std::string> &PackageDB::extraPackageFilesDirectorySlashPrefixes() const {
    return extraPackageFilesDirectorySlashPrefixes_;
}

const std::string_view PackageDB::errorHint() const {
    return errorHint_;
}

PackageDB PackageDB::deepCopy() const {
    ENFORCE(frozen);
    PackageDB result;
    result.packages_.reserve(this->packages_.size());
    for (auto const &[nr, pkgInfo] : this->packages_) {
        result.packages_[nr] = pkgInfo->deepCopy();
    }
    result.secondaryTestPackageNamespaceRefs_ = this->secondaryTestPackageNamespaceRefs_;
    result.extraPackageFilesDirectoryUnderscorePrefixes_ = this->extraPackageFilesDirectoryUnderscorePrefixes_;
    result.extraPackageFilesDirectorySlashPrefixes_ = this->extraPackageFilesDirectorySlashPrefixes_;
    result.packagesByPathPrefix = this->packagesByPathPrefix;
    result.mangledNames = this->mangledNames;
    result.errorHint_ = this->errorHint_;
    return result;
}

UnfreezePackages PackageDB::unfreeze() {
    return UnfreezePackages(*this);
}

} // namespace sorbet::core::packages
