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
 *   class Opus::Foo < PackageSpec
 *    import Bar
 *    import OtherPkg::X
 *
 *    export Baz
 *   end
 *
 * to:
 *
 *   module <PACKAGE_REGISTRY>::Opus_Foo_Package
 *     module Bar
 *       # Import methods exported on Bar (TODO: This seems incorrect?
 *       # _Any_ top-level package method will then be available)
 *       extend <PACKAGE_REGISTRY>::Bar_Package
 *       # Import each class exported by Bar
 *       SomeClassInBar = <PACKAGE_REGISTRY>::Bar_Package::SomeClassInBar
 *     end
 *
 *     module OtherPkg
 *       module X
 *         extend <PACKAGE_REGISTRY>::OtherPkg__X_Package::<PackageMethods>
 *         SomeClassInX = <PACKAGE_REGISTRY>::OtherPkg__X_Package::SomeClassInX
 *       end
 *     end
 *   end
 *
 *   class Opus::Foo < PackageSpec
 *    import Bar
 *    import OtherPkg::X
 *
 *    export Baz
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

    // Run packager incrementally. Note: Cannot run incrementally on package file changes.
    static std::vector<ast::ParsedFile> runIncremental(core::GlobalState &gs, std::vector<ast::ParsedFile> allPackages,
                                                       std::vector<ast::ParsedFile> changedFiles);

    Packager() = delete;
};
} // namespace sorbet::packager

#endif // SORBET_REWRITER_PACKAGE_H
