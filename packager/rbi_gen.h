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

    // Generate RBIs for all packages named in `packageFiles`.
    static void run(core::GlobalState &gs, const UnorderedSet<core::ClassOrModuleRef> &packageNamespaces,
                    std::string outputDir, WorkerPool &workers);

    // Generate RBIs for a single package, whose path is given by `packageName`. The `packageFiles` vector must include
    // the file named by `packageName`, or an error will be raised.
    static void runSinglePackage(core::GlobalState &gs, const UnorderedSet<core::ClassOrModuleRef> &packageNamespaces,
                                 core::NameRef package, std::string outputDir, WorkerPool &workers);

    struct SinglePackageInfo {
        core::NameRef packageName;
        std::vector<core::NameRef> parents;
    };

    static SinglePackageInfo findSinglePackage(core::GlobalState &gs, std::string packageName);
};
} // namespace sorbet::packager

#endif // SORBET_PACKAGER_RBI_GEN_H
