#ifndef SORBET_REWRITER_PACKAGE_H
#define SORBET_REWRITER_PACKAGE_H
#include "ast/ast.h"
#include "packager/VisibilityChecker.h"

namespace sorbet {
class WorkerPool;
}

namespace sorbet::packager {
/**
 * This pass transforms package files (`foo/__package.rb`) from
 *
 *   class Project::Foo < PackageSpec
 *     import Project::Bar
 *
 *     export Baz
 *     export_methods FooClassWithMethods
 *   end
 *
 * given
 *
 *   class Project::Bar < PackageSpec
 *     export SomeClassInBar
 *     export Inner::SomeOtherClassInBar
 *   end
 *
 * to:
 *
 *   class <PackageSpecRegistry>::Project::Foo < PackageSpec
 *    import <PackageSpecRegistry>::Project::Bar
 *
 *    export Package::Baz
 *   end
 *
 * Note that packages cannot have `_` in their names, so the above name mangling is 1:1.
 *
 * This pass intentionally runs outside of rewriter so that its output does not get cached.
 */
class Packager final {
public:
    // Run packager incrementally. Note: `files` must contain all packages files. Does not support package changes.
    static std::vector<ast::ParsedFile> runIncremental(const core::GlobalState &gs, std::vector<ast::ParsedFile> files,
                                                       WorkerPool &workers);

    // Build the packageDB only. This requires that the `files` span only contains `__package.rb` files that have been
    // through the indexer and namer.
    static void buildPackageDB(core::GlobalState &gs, WorkerPool &workers, absl::Span<ast::ParsedFile> files,
                               absl::Span<core::FileRef> nonPackageFiles);

    // Validate packaged files. This requires that the `files` span does not contain any `__package.rb` files.
    static void validatePackagedFiles(const core::GlobalState &gs, WorkerPool &workers,
                                      absl::Span<ast::ParsedFile> files);

    static core::SymbolRef getEnumClassForEnumValue(const core::GlobalState &gs, core::SymbolRef sym);

    // Remove the `test_export` sends from the package definition in `file`, as we process the package definition with
    // both the application and test sources, which will occur in different strata.
    //
    // NOTE: this can be removed if we ever switch to keeping tests in a completely separate package.
    static ast::ParsedFile copyPackageWithoutTestExports(const core::GlobalState &gs, const ast::ParsedFile &file);

    Packager() = delete;
};
} // namespace sorbet::packager

#endif // SORBET_REWRITER_PACKAGE_H
