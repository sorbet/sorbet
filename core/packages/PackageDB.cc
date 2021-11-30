#include "core/packages/PackageDB.h"
#include "absl/strings/match.h"
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
        ENFORCE(false);
        return emptyName;
    }

    const vector<string> &pathPrefixes() const {
        ENFORCE(false);
        return prefixes;
    }

    unique_ptr<PackageInfo> deepCopy() const {
        ENFORCE(false);
        return make_unique<NonePackage>();
    }

    Loc definitionLoc() const {
        ENFORCE(false);
        return Loc::none();
    }

    vector<MissingExportMatch> findMissingExports(core::Context ctx, core::SymbolRef scope, core::NameRef name) const {
        ENFORCE(false);
        return {};
    }

    ~NonePackage() {}

private:
    const vector<string> prefixes;
    const vector<core::NameRef> emptyName;
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
    db.writerThread = std::thread::id();
    db.frozen = true;
    // Note in the future we may want to change the data structures involved in a way that requires
    // a finalization step. Controlling freeze/unfreeze with RAII gives a good hook to do that.
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
        ENFORCE(prev->second->definitionLoc() == pkg->definitionLoc());
        ENFORCE(prev->second->pathPrefixes() == pkg->pathPrefixes());
    }
    packages_[nr] = move(pkg);
    ENFORCE(mangledNames.size() == packages_.size());
    return nr;
}

NameRef PackageDB::lookupPackage(NameRef pkgMangledName) const {
    ENFORCE(pkgMangledName.exists());
    auto it = packages_.find(pkgMangledName);
    if (it == packages_.end()) {
        return NameRef::noName();
    }
    return it->first;
}

const PackageInfo &PackageDB::getPackageForFile(const core::GlobalState &gs, core::FileRef file) const {
    ENFORCE(frozen);
    auto &fileData = file.data(gs);
    string_view path = fileData.path();
    int curPrefixPos = path.find_last_of('/');
    while (curPrefixPos != string::npos) {
        const auto &it = packagesByPathPrefix.find(path.substr(0, curPrefixPos + 1));
        if (it != packagesByPathPrefix.end()) {
            const auto &pkg = getPackageInfo(it->second);
            ENFORCE(pkg.exists());
            return pkg;
        }

        if (fileData.sourceType == core::File::Type::Package) {
            // When looking up a `__package.rb` file do not search parent directories
            break;
        }
        curPrefixPos = path.find_last_of('/', curPrefixPos - 1);
    }
    return NONE_PKG;
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

const std::vector<std::string> &PackageDB::extraPackageFilesDirectoryPrefixes() const {
    return extraPackageFilesDirectoryPrefixes_;
}

bool PackageDB::isTestFile(const core::GlobalState &gs, const core::File &file) {
    // TODO: (aadi-stripe, 11/26/2021) see if these can all be changed to use getPrintablePath
    return absl::EndsWith(file.path(), ".test.rb") || absl::StartsWith(file.path(), "./test/") ||
           absl::StrContains(gs.getPrintablePath(file.path()), "/test/");
}

PackageDB PackageDB::deepCopy() const {
    ENFORCE(frozen);
    PackageDB result;
    result.packages_.reserve(this->packages_.size());
    for (auto const &[nr, pkgInfo] : this->packages_) {
        result.packages_[nr] = pkgInfo->deepCopy();
    }
    result.secondaryTestPackageNamespaceRefs_ = this->secondaryTestPackageNamespaceRefs_;
    result.extraPackageFilesDirectoryPrefixes_ = this->extraPackageFilesDirectoryPrefixes_;
    result.packagesByPathPrefix = this->packagesByPathPrefix;
    result.mangledNames = this->mangledNames;
    return result;
}

UnfreezePackages PackageDB::unfreeze() {
    return UnfreezePackages(*this);
}

} // namespace sorbet::core::packages
