#ifndef SORBET_REWRITER_PACKAGE_H
#define SORBET_REWRITER_PACKAGE_H
#include "ast/ast.h"
#include "core/packages/PackageInfo.h"
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
 *     export_methods SomeClassWithMethods, SomeOtherClassWithMethods
 *   end
 *
 * to:
 *
 *   class <PackgeRegistry>::Project::Foo < PackageSpec
 *    import <PackgeRegistry>::Project::Bar
 *
 *    export Package::Baz
 *    export_methods Package::FooClassWithMethods
 *   end
 *
 * Note that packages cannot have `_` in their names, so the above name mangling is 1:1.
 *
 * This pass intentionally runs outside of rewriter so that its output does not get cached.
 */
class Packager final {
public:
    static std::vector<ast::ParsedFile> findPackages(core::GlobalState &gs, WorkerPool &workers,
                                                     std::vector<ast::ParsedFile> files);

    static std::vector<ast::ParsedFile> run(core::GlobalState &gs, WorkerPool &workers,
                                            std::vector<ast::ParsedFile> files);

    // Run packager incrementally. Note: `files` must contain all packages files. Does not support package changes.
    static std::vector<ast::ParsedFile> runIncremental(core::GlobalState &gs, std::vector<ast::ParsedFile> files);
    // Run packager incrementally, but withouut mutating GlobalState. This is used by LSP to serve
    // certain kinds of requests using stale data. In this mode, if mutating GlobalState would have
    // been required to make progress, the corresponding part of the tree that triggered the
    // mutation is simply dropped.
    static std::vector<ast::ParsedFile> runIncrementalBestEffort(const core::GlobalState &gs,
                                                                 std::vector<ast::ParsedFile> files);

    static void dumpPackageInfo(const core::GlobalState &gs, std::string output);

    // For each file, set its package name.
    static void setPackageNameOnFiles(core::GlobalState &gs, const std::vector<ast::ParsedFile> &files);

    // For each file, set its package name.
    static void setPackageNameOnFiles(core::GlobalState &gs, const std::vector<core::FileRef> &files);

    Packager() = delete;
};
} // namespace sorbet::packager

#endif // SORBET_REWRITER_PACKAGE_H
