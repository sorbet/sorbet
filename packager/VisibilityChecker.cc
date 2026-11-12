#include "packager/VisibilityChecker.h"
#include "absl/synchronization/blocking_counter.h"
#include "ast/treemap/treemap.h"
#include "common/concurrency/Parallel.h"
#include "core/Context.h"

using namespace std;

namespace sorbet::packager {

namespace {

class VisibilityCheckerPass final {
public:
    const core::packages::PackageInfo &package;
    UnorderedMap<core::packages::MangledName, core::packages::PackageReferenceInfo> referencedPackages;

    // We only want to validate visibility for usages of constants, not definitions.
    // postTransformConstantLit does not discriminate, so we have to remember whether a given
    // ConstantLit was a definition.
    UnorderedSet<const void *> constantAssignmentDefinitions;

    VisibilityCheckerPass(core::Context ctx, const core::packages::PackageInfo &package) : package{package} {}

    void preTransformAssign(core::Context ctx, const ast::Assign &asgn) {
        auto lhs = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (lhs != nullptr) {
            constantAssignmentDefinitions.insert(lhs.get());
        }
    }

    void postTransformAssign(core::Context ctx, const ast::Assign &asgn) {
        auto lhs = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (lhs != nullptr) {
            constantAssignmentDefinitions.erase(lhs.get());
        }
    }

    void postTransformConstantLit(core::Context ctx, const ast::ConstantLit &lit) {
        if (constantAssignmentDefinitions.contains(&lit)) {
            return;
        }

        auto litSymbol = lit.symbol();
        if (!litSymbol.isClassOrModule() && !litSymbol.isFieldOrStaticField()) {
            return;
        }
        if (litSymbol == core::Symbols::todo()) {
            // The symbol will be `<todo sym>` when traversing to a ClassDef which omits a superclass.
            // TODO(jez) Should we change put the real superclass in the tree once GlobalPass resolves it?
            return;
        }

        // no need to check visibility for these cases
        auto otherPackage = litSymbol.enclosingClass(ctx).data(ctx)->package;
        if (!otherPackage.exists() || this->package.mangledName() == otherPackage) {
            return;
        }

        auto importError = this->package.checkReferenceAgainstImports(ctx, lit.loc(), otherPackage);
        referencedPackages[otherPackage] = importError.value_or(core::packages::PackageReferenceInfo{});
        // Resolver reports references to package-private constants.
    }

    void preTransformClassDef(core::Context ctx, const ast::ClassDef &original) {
        if (original.kind == ast::ClassDef::Kind::Class && !original.ancestors.empty()) {
            auto &superClass = original.ancestors[0];
            ast::ConstTreeWalk::apply(ctx, *this, superClass);
        }
    }

    static void run(core::GlobalState &nonConstGs, WorkerPool &workers, absl::Span<const ast::ParsedFile> filesSpan) {
        const core::GlobalState &gs = nonConstGs;
        core::packages::PackageDB &nonConstPackageDB = nonConstGs.packageDB();
        struct ThreadResult {
            core::FileRef file;
            UnorderedMap<core::packages::MangledName, core::packages::PackageReferenceInfo> referencedPackages;
        };
        auto resultq = std::make_shared<BlockingBoundedQueue<std::optional<ThreadResult>>>(filesSpan.size());
        Timer timeit(gs.tracer(), "visibility_checker.check_visibility");
        auto taskq = std::make_shared<ConcurrentBoundedQueue<size_t>>(filesSpan.size());
        for (size_t i = 0; i < filesSpan.size(); ++i) {
            taskq->push(i, 1);
        }

        // N.B.: `workers.size()` can be `0` when threads are disabled, which would result in undefined behavior for
        // `BlockingCounter`.
        absl::BlockingCounter barrier(std::max(workers.size(), 1));
        workers.multiplexJob("VisibilityChecker", [taskq, &filesSpan, &gs, resultq, &barrier]() {
            size_t idx;
            for (auto result = taskq->try_pop(idx); !result.done(); result = taskq->try_pop(idx)) {
                if (!result.gotItem()) {
                    continue;
                }
                auto &f = filesSpan[idx];
                auto package = gs.packageDB().getPackageForSourceFile(gs, f.file);
                if (package == nullptr) {
                    resultq->push(std::nullopt, 1);
                    continue;
                }
                core::Context ctx{gs, core::Symbols::root(), f.file};
                VisibilityCheckerPass pass{ctx, *package};
                ast::ConstTreeWalk::apply(ctx, pass, f.tree);
                resultq->push(ThreadResult{f.file, std::move(pass.referencedPackages)}, 1);
            }
            barrier.DecrementCount();
        });

        std::optional<ThreadResult> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem() && threadResult.has_value()) {
                auto &file = threadResult->file;
                auto pkgName = gs.packageDB().getPackageNameForFile(file);
                if (!pkgName.exists()) {
                    continue;
                }

                auto nonConstPackageInfo = nonConstPackageDB.getPackageInfoNonConst(pkgName);
                vector<pair<core::packages::MangledName, core::packages::PackageReferenceInfo>> references;
                auto &referencedPackages = threadResult->referencedPackages;
                for (auto &[packageName, packageReferenceInfo] : referencedPackages) {
                    references.emplace_back(make_pair(packageName, packageReferenceInfo));
                }
                if (gs.packageDB().genPackagesMode() == core::packages::GenPackagesMode::Disabled) {
                    // Resolver has already recorded references that it rejected at package boundaries. Merge the
                    // successfully resolved references found here into the same per-file entry.
                    for (auto &[packageName, packageReferenceInfo] : references) {
                        nonConstPackageInfo->trackPackageReference(file, packageName, packageReferenceInfo);
                    }
                } else {
                    // Resolver package checking is disabled in gen-packages mode, so this pass owns the complete set.
                    nonConstPackageInfo->trackPackageReferences(file, references);
                }
            }
        }
        barrier.Wait();
    }
};
} // namespace

void VisibilityChecker::checkReferences(core::GlobalState &gs, WorkerPool &workers,
                                        absl::Span<const ast::ParsedFile> files) {
    VisibilityCheckerPass::run(gs, workers, files);
}

} // namespace sorbet::packager
