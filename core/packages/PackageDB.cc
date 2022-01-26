#include "core/packages/PackageDB.h"
#include "absl/strings/match.h"
#include "common/sort.h"
#include "core/AutocorrectSuggestion.h"
#include "core/GlobalState.h"
#include "core/Loc.h"
#include "core/packages/Layer.h"

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

    Loc definitionLoc() const {
        notImplemented();
        return Loc::none();
    }

    sorbet::core::packages::Layer layer() const {
        notImplemented();
        return Layer(-1);
    }

    std::optional<core::AutocorrectSuggestion> addImport(const core::GlobalState &gs, const PackageInfo &pkg,
                                                         bool isTestImport) const {
        notImplemented();
        return nullopt;
    }

    vector<MissingExportMatch> findMissingExports(core::Context ctx, core::SymbolRef scope, core::NameRef name) const {
        notImplemented();
        return {};
    }

    std::optional<core::AutocorrectSuggestion> addExport(const core::GlobalState &gs, const core::SymbolRef name,
                                                         bool isPrivateTestExport) const {
        return {};
    }

    bool ownsSymbol(const core::GlobalState &gs, core::SymbolRef symbol) const {
        notImplemented();
        return false;
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
    while (curPrefixPos > 0) {
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

const std::string_view PackageDB::errorHint() const {
    return errorHint_;
}

const std::string_view PackageDB::getLayerName(uint8_t layerRank) const {
    ENFORCE(layerRank < layerNames_.size());
    return layerNames_.at(layerRank);
}

const uint8_t PackageDB::getLayerRank(std::string layerName) const {
    for (std::size_t i = 0; i < layerNames_.size(); i++) {
        if (layerNames_.at(i).compare(layerName) == 0) {
            return i;
        }
    }
    return 255;
}

const std::size_t PackageDB::getLayerCount() const {
    return layerNames_.size();
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
    result.errorHint_ = this->errorHint_;
    return result;
}

UnfreezePackages PackageDB::unfreeze() {
    return UnfreezePackages(*this);
}

} // namespace sorbet::core::packages
