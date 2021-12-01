#ifndef SORBET_REWRITER_PACKAGE_H
#define SORBET_REWRITER_PACKAGE_H
#include "ast/ast.h"
#include "core/packages/PackageInfo.h"

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
 *   module <PackageRegistry>::Project_Foo_Package
 *     module Project::Bar
 *       # Import methods exported on Bar
 *       extend <PackageRegistry>::Bar_Package::<PackageMethods>
 *       # Import each class exported by Bar
 *       SomeClassInBar = <PackageRegistry>::Bar_Package::SomeClassInBar
 *       SomeOtherClassInBar = <PackageRegistry>::Bar_Package::SomeOtherClassInBar
 *     end
 *     module <PackageMethods>
 *       include <PackageRegistry>::Project_Foo_Package::FooClassWithMethods
 *     end
 *   end
 *
 *   class Project::Foo < PackageSpec
 *    import Project::Bar
 *
 *    export <PackageRegistry>::Project_Foo_Package::Baz
 *    export_methods <PackageRegistry>::Project_Foo_Package::FooClassWithMethods
 *   end
 *
 * It also rewrites files in the package, like `foo/baz.rb`, from:
 *
 * class SomeOtherClassInTheFile
 *   ...
 * end
 *
 * class Baz
 *   ...
 * end
 *
 * to:
 *
 * module <PackageRegistry>::Project_Foo_Package
 *   class SomeOtherClassInTheFile
 *     ...
 *   end
 *
 *   class Baz
 *     ...
 *   end
 * end
 *
 * Note that packages cannot have `_` in their names, so the above name mangling is 1:1.
 *
 * This pass intentionally runs outside of rewriter so that its output does not get cached.
 */
class Packager final {
public:
    static std::vector<ast::ParsedFile> run(core::GlobalState &gs, WorkerPool &workers,
                                            std::vector<ast::ParsedFile> files);

    // Run packager incrementally. Note: `files` must contain all packages files. Does not support package changes.
    static std::vector<ast::ParsedFile> runIncremental(core::GlobalState &gs, std::vector<ast::ParsedFile> files);

    // The structures created for `__package.rb` files from their imports are large and deep. This causes
    // performance problems with typechecking. Use  to remove these modules while retaining the PackageSpec
    // class itself during typecheck. Pair with `replacePackageModules`.
    static std::vector<ast::ExpressionPtr> removePackageModules(const core::GlobalState &gs, ast::ParsedFile &pf);
    static ast::ParsedFile replacePackageModules(const core::GlobalState &gs, ast::ParsedFile pf,
                                                 std::vector<ast::ExpressionPtr> removed);

    Packager() = delete;
};
} // namespace sorbet::packager

#endif // SORBET_REWRITER_PACKAGE_H
