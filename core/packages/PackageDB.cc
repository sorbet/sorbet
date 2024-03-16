#include "core/packages/PackageDB.h"
#include "absl/strings/match.h"
#include "common/sort/sort.h"
#include "core/AutocorrectSuggestion.h"
#include "core/GlobalState.h"
#include "core/Loc.h"

using namespace std;

namespace sorbet::core::packages {

namespace {
class NonePackage final : public PackageInfo {
public:
    MangledName mangledName() const {
        return MangledName();
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

    bool exportAll() const {
        notImplemented();
        return false;
    }

    bool visibleToTests() const {
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
    std::vector<VisibleTo> visibleTo() const {
        return vector<VisibleTo>();
    }

    std::optional<ImportType> importsPackage(MangledName mangledName) const {
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

MangledName PackageDB::enterPackage(unique_ptr<PackageInfo> pkg) {
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

const MangledName PackageDB::getPackageNameForFile(FileRef file) const {
    if (this->packageForFile_.size() <= file.id()) {
        return MangledName();
    }

    return this->packageForFile_[file.id()];
}

// It's unfortunate that we have to implement this the way we do.
//
// It would have been nicer if we could get away with packages non-packages sharing the same subtree
// in the symbol table. We can't because:
//
// 1.  test code lives inside the `Test::` namespace, and (2) LSP support for
// 2.  LSP support for definition/references in __package.rb files depends heavily on locations for
//     package symbols and package namespace symbols being distinct from normal symbols.
const MangledName PackageDB::getPackageNameFromBreadcrumbs(const GlobalState &gs,
                                                           const vector<NameRef> &breadcrumbs) const {
    auto cur = Symbols::PackageSpecRegistry();
    SymbolRef bestPackageSpec;
    for (auto breadcrumb = breadcrumbs.rbegin(); breadcrumb != breadcrumbs.rend(); ++breadcrumb) {
        auto curSym = cur.data(gs)->findMember(gs, *breadcrumb);
        if (!curSym.exists()) {
            // We've reached the leaves of the package hierarchy despite there still
            // being breadcrumbs left, so we've found the most specific package.
            break;
        }

        ENFORCE(curSym.isClassOrModule(), "Non-ClassOrModule in package hierarchy");

        cur = curSym.asClassOrModuleRef();
        auto newPackageSpec = cur.data(gs)->findMember(gs, Names::Constants::PackageSpec_Storage());
        if (newPackageSpec.exists()) {
            bestPackageSpec = newPackageSpec;
        }
    }

    if (bestPackageSpec.exists()) {
        return getPackageNameForFile(bestPackageSpec.loc(gs).file());
    } else {
        return MangledName();
    }
}

const MangledName PackageDB::getPackageNameForSymbolImpl(const GlobalState &gs, ClassOrModuleRef klass,
                                                         vector<NameRef> &breadcrumbs) const {
    if (klass == Symbols::root() || klass == Symbols::PackageSpecRegistry()) {
        return getPackageNameFromBreadcrumbs(gs, breadcrumbs);
    }

    auto data = klass.data(gs);
    auto owner = data->owner;
    if (owner == Symbols::root() && data->name == TEST_NAMESPACE) {
        return getPackageNameFromBreadcrumbs(gs, breadcrumbs);
    }

    auto attachedClass = data->attachedClass(gs);
    if (attachedClass.exists()) {
        // Don't add name to breadcrumbs, because we will never find a <PackageSpec> under a singleton class.
        return getPackageNameForSymbolImpl(gs, attachedClass, breadcrumbs);
    }

    // Function is tail recursive, so mutating the vector instead of making a copy is fine.
    breadcrumbs.emplace_back(data->name);
    return getPackageNameForSymbolImpl(gs, owner, breadcrumbs);
}

const MangledName PackageDB::getPackageNameForSymbol(const GlobalState &gs, SymbolRef sym) const {
    if (!sym.isClassOrModule()) {
        return getPackageNameForSymbol(gs, sym.owner(gs));
    }

    vector<NameRef> breadcrumbs;
    return getPackageNameForSymbolImpl(gs, sym.asClassOrModuleRef(), breadcrumbs);
}

void PackageDB::setPackageNameForFile(FileRef file, MangledName mangledName) {
    if (this->packageForFile_.size() <= file.id()) {
        this->packageForFile_.resize(file.id() + 1, MangledName());
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
    auto cnst = core::packages::MangledName::mangledNameFromHuman(gs, nameStr);
    if (!cnst.exists()) {
        return NONE_PKG;
    }
    return getPackageInfo(cnst);
}

const PackageInfo &PackageDB::getPackageInfo(MangledName mangledName) const {
    auto it = packages_.find(mangledName);
    if (it == packages_.end()) {
        return NONE_PKG;
    }
    return *it->second;
}

const vector<MangledName> &PackageDB::packages() const {
    return mangledNames;
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

bool PackageDB::allowRelaxedPackagerChecksFor(MangledName mangledName) const {
    return absl::c_find(allowRelaxedPackagerChecksFor_, mangledName) != allowRelaxedPackagerChecksFor_.end();
}

PackageDB PackageDB::deepCopy() const {
    ENFORCE(frozen);
    PackageDB result;
    result.packages_.reserve(this->packages_.size());
    for (auto const &[nr, pkgInfo] : this->packages_) {
        result.packages_[nr] = pkgInfo->deepCopy();
    }
    result.enabled_ = this->enabled_;
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
