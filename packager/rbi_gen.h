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
    static UnorderedSet<core::ClassOrModuleRef>
    buildPackageNamespace(core::GlobalState &gs, std::vector<ast::ParsedFile> &packageFiles, WorkerPool &workers);
    static RBIOutput runOnce(const core::GlobalState &gs, core::NameRef pkg,
                             const UnorderedSet<core::ClassOrModuleRef> &packageNamespaces);

    static void run(core::GlobalState &gs, std::vector<ast::ParsedFile> packageFiles, std::string outputDir,
                    WorkerPool &workers);
};
} // namespace sorbet::packager

#endif // SORBET_PACKAGER_RBI_GEN_H
