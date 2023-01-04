#ifndef SORBET_REWRITER_PACKAGE_H
#define SORBET_REWRITER_PACKAGE_H
#include "ast/ast.h"
#include "core/packages/PackageInfo.h"
#include "packager/VisibilityChecker.h"

namespace sorbet {
class WorkerPool;
}

namespace sorbet::packager {
const core::NameRef TEST_NAME = core::Names::Constants::Test();

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
 *   class <PackgeSpecRegistry>::Project::Foo < PackageSpec
 *    import <PackgeSpecRegistry>::Project::Bar
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
    static std::vector<ast::ParsedFile> findPackages(core::GlobalState &gs, WorkerPool &workers,
                                                     std::vector<ast::ParsedFile> files);

    static std::vector<ast::ParsedFile> run(core::GlobalState &gs, WorkerPool &workers,
                                            std::vector<ast::ParsedFile> files);

    // Run packager incrementally. Note: `files` must contain all packages files. Does not support package changes.
    static std::vector<ast::ParsedFile> runIncremental(core::GlobalState &gs, std::vector<ast::ParsedFile> files);

    static void dumpPackageInfo(const core::GlobalState &gs, std::string output);

    // For each file, set its package name.
    static void setPackageNameOnFiles(core::GlobalState &gs, const std::vector<ast::ParsedFile> &files);

    // For each file, set its package name.
    static void setPackageNameOnFiles(core::GlobalState &gs, const std::vector<core::FileRef> &files);

    static core::SymbolRef getEnumClassForEnumValue(const core::GlobalState &gs, core::SymbolRef sym);

    Packager() = delete;
};
} // namespace sorbet::packager

#endif // SORBET_REWRITER_PACKAGE_H
