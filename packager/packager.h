#ifndef SORBET_REWRITER_PACKAGE_H
#define SORBET_REWRITER_PACKAGE_H
#include "ast/ast.h"

namespace sorbet {
class WorkerPool;
}

namespace sorbet::packager {
/**
 * This pass transforms package files (`foo/__package.rb`) from
 *
 *   class Project::Foo < PackageSpec
 *    import Project::Bar
 *
 *    export Baz
 *    export_methods FooClassWithMethods
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
 *   module <PACKAGE_REGISTRY>::Project_Foo_Package
 *     module Project::Bar
 *       # Import methods exported on Bar
 *       extend <PACKAGE_REGISTRY>::Bar_Package::SomeClassWithMethods
 *       extend <PACKAGE_REGISTRY>::Bar_Package::SomeOtherClassWithMethods
 *       # Import each class exported by Bar
 *       SomeClassInBar = <PACKAGE_REGISTRY>::Bar_Package::SomeClassInBar
 *       SomeOtherClassInBar = <PACKAGE_REGISTRY>::Bar_Package::SomeOtherClassInBar
 *     end
 *   end
 *
 *   class Opus::Foo < PackageSpec
 *    import Bar
 *    import OtherPkg::X
 *
 *    export <PACKAGE_REGISTRY>::Project_Foo_Package::Baz
 *    export_methods <PACKAGE_REGISTRY>::Project_Foo_Package::FooClassWithMethods
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
 * module <PACKAGE_REGISTRY_INTERNAL>::Opus_Foo_Package
 *   class SomeOtherClassInTheFile
 *     ...
 *   end
 *
 *   class Baz
 *     ...
 *   end
 * end
 *
 * This pass intentionally runs outside of rewriter so that its output does not get cached.
 */
class Packager final {
public:
    static std::vector<ast::ParsedFile> run(core::GlobalState &gs, WorkerPool &workers,
                                            std::vector<ast::ParsedFile> files);

    // Run packager incrementally. Note: `files` must contain all packages files that regular files in `files` belong
    // to. Does not support package changes.
    static std::vector<ast::ParsedFile> runIncremental(core::GlobalState &gs, std::vector<ast::ParsedFile> files);

    Packager() = delete;
};
} // namespace sorbet::packager

#endif // SORBET_REWRITER_PACKAGE_H
