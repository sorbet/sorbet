#ifndef SORBET_PACKAGER_RBI_GEN_H
#define SORBET_PACKAGER_RBI_GEN_H
#include "ast/ast.h"
#include "core/packages/PackageInfo.h"

namespace sorbet::packager {
/**
 * Generates an RBI file for the given package's exports.
 */
class RBIGenerator final {
public:
    struct RBIOutput {
        std::string baseFilePath;
        std::string rbi;
        std::string testRBI;
        // each is a json array of mangled package names
        std::string rbiPackageDependencies;
        std::string testRBIPackageDependencies;
    };

    // Exposed for testing.
    static UnorderedSet<core::ClassOrModuleRef> buildPackageNamespace(core::GlobalState &gs, WorkerPool &workers);
    static RBIOutput runOnce(const core::GlobalState &gs, core::NameRef pkg,
                             const UnorderedSet<core::ClassOrModuleRef> &packageNamespaces);

    // Generate RBIs for all packages present in the package database of `gs`.
    static void run(core::GlobalState &gs, const UnorderedSet<core::ClassOrModuleRef> &packageNamespaces,
                    std::string outputDir, WorkerPool &workers);

    // Generate RBIs for a single package, provided as the mangled package name `package`.
    static void runSinglePackage(core::GlobalState &gs, const UnorderedSet<core::ClassOrModuleRef> &packageNamespaces,
                                 core::NameRef package, std::string outputDir, WorkerPool &workers);

    struct SinglePackageInfo {
        // The mangled name of the package we're generating an interface for.
        core::NameRef packageName;

        // The mangled names of all packages that lie in the parent namespace of `packageName` above. For example, if
        // the package we're generating an interface for is `Foo::Bar` and `Foo` is also a package, the mangled name of
        // `Foo` will be the only element in this vector.
        std::vector<core::NameRef> parents;
    };

    static SinglePackageInfo findSinglePackage(core::GlobalState &gs, std::string packageName);
};
} // namespace sorbet::packager

#endif // SORBET_PACKAGER_RBI_GEN_H
