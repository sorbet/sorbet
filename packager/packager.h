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
    static void findPackages(core::GlobalState &gs, absl::Span<ast::ParsedFile> files);

    static void run(core::GlobalState &gs, WorkerPool &workers, absl::Span<ast::ParsedFile> files);

    // Run packager incrementally. Note: `files` must contain all packages files. Does not support package changes.
    static std::vector<ast::ParsedFile> runIncremental(const core::GlobalState &gs, std::vector<ast::ParsedFile> files);

    static void dumpPackageInfo(const core::GlobalState &gs, std::string output);

    // For each file, set its package name.
    static void setPackageNameOnFiles(core::GlobalState &gs, absl::Span<const ast::ParsedFile> files);

    // For each file, set its package name.
    static void setPackageNameOnFiles(core::GlobalState &gs, absl::Span<const core::FileRef> files);

    static core::SymbolRef getEnumClassForEnumValue(const core::GlobalState &gs, core::SymbolRef sym);

    Packager() = delete;
};
} // namespace sorbet::packager

#endif // SORBET_REWRITER_PACKAGE_H
