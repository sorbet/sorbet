#include "core/packages/PackageDB.h"
#include "core/Loc.h"

using namespace std;

namespace sorbet::core::packages {

namespace {
class NonePackage final : public PackageInfo {
public:
    core::NameRef mangledName() const {
        return NameRef::noName();
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

    ~NonePackage() {}

private:
    const vector<string> prefixes;
};
static const NonePackage NONE_PKG;
} // namespace

UnfreezePackages::UnfreezePackages(PackageDB &db) : db(db) {
    ENFORCE(db.frozen);
    db.frozen = false;
    db.writerThread = this_thread::get_id();
}

UnfreezePackages::~UnfreezePackages() {
    ENFORCE(!db.frozen);
    db.frozen = true;
    db.writerThread = std::thread::id();
    // Note in the future we may want to change the data structures involved in a way that requires
    // a finalization step. Controlling freeze/unfreeze with RAII gives a good hook to do that.
}

NameRef PackageDB::enterPackage(unique_ptr<PackageInfo> pkg) {
    ENFORCE(!frozen);
    ENFORCE(writerThread == this_thread::get_id(), "PackageDB writes are not thread safe");
    // TODO enforce packaging enabled
    auto nr = pkg->mangledName();
    auto prev = packages.find(nr);
    if (prev == packages.end()) {
        for (const auto &prefix : pkg->pathPrefixes()) {
            // TODO can we not copy
            packagesByPathPrefix[prefix] = nr;
        }
    } else {
        // Package files do not have full featured content hashing. If the contents of one changes
        // we always run slow-path and fully rebuild the set of packages. In some cases, the LSP
        // fast-path may re-run on an unchanged package file. Sanity check to ensure the loc and
        // prefixes are the same.
        ENFORCE(prev->second->definitionLoc() == pkg->definitionLoc());
        ENFORCE(prev->second->pathPrefixes() == pkg->pathPrefixes());
    }
    packages[nr] = move(pkg);
    return nr;
}

NameRef PackageDB::lookupPackage(NameRef pkgMangledName) const {
    ENFORCE(pkgMangledName.exists());
    auto it = packages.find(pkgMangledName);
    if (it == packages.end()) {
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
    auto it = packages.find(mangledName);
    if (it == packages.end()) {
        return NONE_PKG;
    }
    return *it->second;
}

unique_ptr<PackageDB> PackageDB::deepCopy() const {
    ENFORCE(frozen);
    auto result = make_unique<PackageDB>();
    result->packages.reserve(this->packages.size());
    for (auto const &[nr, pkgInfo] : this->packages) {
        result->packages[nr] = pkgInfo->deepCopy();
    }
    result->packagesByPathPrefix = this->packagesByPathPrefix;
    return result;
}

UnfreezePackages PackageDB::unfreeze() {
    return UnfreezePackages(*this);
}

} // namespace sorbet::core::packages
