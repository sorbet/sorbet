#include "packager/packager.h"
#include "absl/strings/str_join.h"
#include "ast/packager/packager.h"
#include "ast/treemap/treemap.h"
#include "common/FileOps.h"
#include "common/concurrency/Parallel.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "core/Unfreeze.h"
#include "core/errors/packager.h"
#include "core/packages/Condensation.h"
#include "core/packages/MangledName.h"
#include "core/packages/PackageInfo.h"
#include "packager/ComputePackageSCCs.h"
#include <algorithm>
#include <cctype>
#include <sys/stat.h>

using namespace std;

namespace sorbet::packager {
using namespace core::packages;
namespace {

constexpr string_view PACKAGE_FILE_NAME = "__package.rb"sv;

bool visibilityApplies(const core::GlobalState &gs, const VisibleTo &vt, MangledName name) {
    if (vt.type == VisibleToType::Wildcard) {
        // a wildcard will match if it's a proper prefix of the package name
        auto curPkg = name.owner;
        auto prefix = vt.mangledName.owner;
        do {
            if (curPkg == prefix) {
                return true;
            }
            curPkg = curPkg.data(gs)->owner;
        } while (curPkg != core::Symbols::root());
        return false;
    } else {
        // otherwise it needs to be the same
        return vt.mangledName == name;
    }
}

string buildValidLayersStr(const core::GlobalState &gs) {
    auto validLayers = gs.packageDB().layers();
    ENFORCE(validLayers.size() > 0);
    if (validLayers.size() == 1) {
        return string(validLayers.front().shortName(gs));
    }
    string result = "";
    for (int i = 0; i < validLayers.size() - 1; i++) {
        if (validLayers.size() > 2) {
            result += core::ErrorColors::format("`{}`, ", validLayers[i].shortName(gs));
        } else {
            result += core::ErrorColors::format("`{}` ", validLayers[i].shortName(gs));
        }
    }
    result += core::ErrorColors::format("or `{}`", validLayers.back().shortName(gs));
    return result;
}

// For a given vector of NameRefs, this represents the "next" vector that does not begin with its
// prefix (without actually constructing it). Consider the following sorted names:
//
// [A B]
// [A B C]
// [A B D E]
//    <<<< Position of LexNext([A B]) roughly equivalent to [A B <Infinity>]
// [X Y]
// [X Y Z]
class LexNext final {
    absl::Span<const core::NameRef> names;

public:
    LexNext(const vector<core::NameRef> &names) : names(names) {}

    bool operator<(absl::Span<const core::NameRef> rhs) const {
        // Lexicographic comparison:
        for (auto lhsIt = names.begin(), rhsIt = rhs.begin(); lhsIt != names.end() && rhsIt != rhs.end();
             ++lhsIt, ++rhsIt) {
            if (lhsIt->rawId() < rhsIt->rawId()) {
                return true;
            } else if (rhsIt->rawId() < lhsIt->rawId()) {
                return false;
            }
        }

        // This is where this implementation differs from `std::lexicographic_compare`: if one name is the prefix of
        // another they're considered equal, wheras `std::lexicographic_compare` would return `true` if the LHS
        // was shorter.
        return false;
    }

    bool operator<(const Export &e) const {
        return *this < e.parts();
    }
};

// If the __package.rb file itself is a test file, then the whole package is a test-only package.
// For example, `test/__package.rb` is a test-only package (e.g. Critic in Stripe's codebase).
bool isTestOnlyPackage(const core::GlobalState &gs, const PackageInfo &pkg) {
    return pkg.loc.file().data(gs).isPackagedTest();
}

vector<core::NameRef> fullyQualifiedNameFromMangledName(const core::GlobalState &gs, MangledName pkg) {
    auto klass = pkg.owner;
    vector<core::NameRef> fqn;
    while (klass != core::Symbols::PackageSpecRegistry()) {
        auto data = klass.data(gs);
        fqn.emplace_back(data->name);
        klass = data->owner;
    }
    absl::c_reverse(fqn);
    return fqn;
}

// TODO(jez) This function is only used in two places, and should be possible to delete:
//
// - To resolve import/visible_to names to MangledNames
// - To populate exports
//
// The constant lit resolution logic should probably not materialize the vector, and instead just
// reimplement enough of ResolveConstantsWalk to resolve the constant literals in place, returning
// the final package symbol.
//
// Exports can probably be rewritten to not need to store the fully qualified name entirely.
FullyQualifiedName getFullyQualifiedName(core::Context ctx, const ast::UnresolvedConstantLit *constantLit) {
    FullyQualifiedName fqn;
    while (constantLit != nullptr) {
        fqn.parts.emplace_back(constantLit->cnst);
        if (auto resolvedLit = ast::cast_tree<ast::ConstantLit>(constantLit->scope)) {
            constantLit = resolvedLit->original();
        } else {
            constantLit = ast::cast_tree<ast::UnresolvedConstantLit>(constantLit->scope);
        }
    }
    reverse(fqn.parts.begin(), fqn.parts.end());
    ENFORCE(!fqn.parts.empty());
    return fqn;
}

// TODO(jez) Might be nice to eagerly resolve these UnresolvedConstantLit to ConstantLit so resolver doesn't have to.
MangledName resolvePackageName(core::Context ctx, const ast::UnresolvedConstantLit *constantLit, bool allowNamespace) {
    ENFORCE(constantLit != nullptr);

    auto fullName = getFullyQualifiedName(ctx, constantLit);

    // Since packager now runs after namer, we know that these symbols are entered.
    auto owner = core::Symbols::PackageSpecRegistry();
    for (auto part : fullName.parts) {
        auto member = owner.data(ctx)->findMember(ctx, part);
        if (!member.exists() || !member.isClassOrModule()) {
            owner = core::Symbols::noClassOrModule();
            break;
        }
        owner = member.asClassOrModuleRef();
    }

    if (owner == core::Symbols::PackageSpecRegistry()) {
        // This is a weird case, because I don't think it's possible to get here, but we can handle it anyways.
        // This whole function should go away with the switch to PackageRef anyways. As in, we
        // should probably be able to pre-resolve the constants in import/visible_to/etc. lines at
        // this point, and report an eager error if those package names fail to resolve, rather than
        // resorting handling this (impossible?) edge case.
        ENFORCE(fullName.parts.empty());
        owner = core::Symbols::noClassOrModule();
    }

    if (owner.exists()) {
        if (!allowNamespace && owner.data(ctx)->superClass() != core::Symbols::PackageSpec()) {
            return MangledName();
        }
    }

    return MangledName(owner);
}

bool recursiveVerifyConstant(core::Context ctx, core::NameRef fun, const ast::ExpressionPtr &root,
                             const ast::ExpressionPtr &expr) {
    if (ast::isa_tree<ast::EmptyTree>(expr)) {
        return true;
    }

    auto target = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    if (target == nullptr) {
        if (auto e = ctx.beginError(root.loc(), core::errors::Packager::InvalidConfiguration)) {
            e.setHeader("Argument to `{}` must be a constant", fun.show(ctx));
        }
        return false;
    }

    return recursiveVerifyConstant(ctx, fun, root, target->scope);
}

const ast::UnresolvedConstantLit *verifyConstant(core::Context ctx, core::NameRef fun, const ast::ExpressionPtr &expr) {
    auto target = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    if (target == nullptr) {
        if (auto e = ctx.beginError(expr.loc(), core::errors::Packager::InvalidConfiguration)) {
            e.setHeader("Argument to `{}` must be a constant", fun.show(ctx));
        }
        return nullptr;
    }

    if (recursiveVerifyConstant(ctx, fun, expr, target->scope)) {
        return target;
    }

    return nullptr;
}

bool isRootScopedDefinition(const ast::ConstantLit *lit) {
    while (lit != nullptr && lit->original() != nullptr) {
        lit = ast::cast_tree<ast::ConstantLit>(lit->original()->scope);
        if (lit != nullptr && lit->symbol() == core::Symbols::root()) {
            return true;
        }
    }

    return false;
}

struct PackageForSymbolResult {
    // The closest package for `sym`
    MangledName bestPkg;

    // The closest owner symbol inside `<PackageSpecRegistry>`. Might not actually correspond to a
    // package if it's just a namespace, e.g. `<PSR>::Pkg1::NS` for `::Pkg1::Inner::NS::A`
    core::ClassOrModuleRef bestOwner;

    // Could be a prefix if `sym` is a `ClassOrModuleRef`
    bool couldBePrefix;
};

bool ownsPackage(const core::GlobalState &gs, const core::ClassOrModuleRef ownerForScope, MangledName pkg) {
    auto owner = pkg.owner;
    while (owner != core::Symbols::root()) {
        if (owner == ownerForScope) {
            return true;
        }

        owner = owner.data(gs)->owner;
    }

    return false;
}

// Visitor that ensures for constants defined within a package that all have the package as a
// prefix.
class EnforcePackagePrefix final {
    const PackageInfo &pkg;

    // Whether code in this file must use the `Test::` namespace.
    //
    // Obviously tests *can* use the `Test::` namespace, but tests in test-only packages don't have to.
    //
    // (This is a wart of the original implementation, not an intentional design choice. It would
    // probably be good in the future to require that runnable tests live in the `Test::` namespace
    // for the package.)
    const bool mustUseTestNamespace;

    // So that we only have to compute this once (makes certain comparisons easier)
    // Note that we don't enter this in GlobalState::initEmpty with a well-known ID,
    // because Sorbet does not always run with --stripe-packages.
    const core::SymbolRef maybeTestNamespace;

    // By contrast with `Context::owner`, this `scope` field:
    //
    // - Only tracks constant symbols (`owner` will be a MethodRef inside `{pre,post}TransformMethodDef`)
    // - `Context::owner` does not track a loc
    vector<pair<core::SymbolRef, core::LocOffsets>> scope;

    // Meant to track when we're inside something like `class ::A; class B; end; end` instead of
    // `class A; class B; end; end`. Classes that start from an absolutely qualified "cbase" with a
    // leading `::` are opted out of the EnforcePackagePrefix checks.
    //
    // TODO(jez) Document this in the public docs for the packager
    //   (at least in the error reference, but also in any eventual docs on the package system).
    //
    //   It's not clear what the long term behavior for this should be. I think that we're going to
    //   have to invent a concept of "prelude packages" and have all root-scoped stuff like this
    //   live in those prelude packages.
    //
    //   (The motivation is: if 100% of code in a repo is packaged, where do monkey patches live,
    //   because the stdlib and gems are unpackaged?)
    size_t rootConsts = 0;

    // Counter to avoid duplicate errors:
    // - Only emit errors when depth is 0
    // - Upon emitting an error increment
    // - Once greater than 0, all preTransform* increment, postTransform* decrement
    int errorDepth = 0;

public:
    EnforcePackagePrefix(core::Context ctx, const PackageInfo &pkg)
        : pkg(pkg), mustUseTestNamespace(ctx.file.data(ctx).isPackagedTest() && !isTestOnlyPackage(ctx, pkg)),
          maybeTestNamespace(core::Symbols::root().data(ctx)->findMember(ctx, PackageDB::TEST_NAMESPACE)) {
        ENFORCE(pkg.exists());
    }

    void preTransformClassDef(core::Context ctx, const ast::ClassDef &classDef) {
        if (classDef.symbol == core::Symbols::root()) {
            // Ignore top-level <root>
            return;
        }
        if (errorDepth > 0) {
            errorDepth++;
            return;
        }

        auto constantLit = ast::cast_tree<ast::ConstantLit>(classDef.name);
        if (constantLit == nullptr) {
            return;
        }

        pushScope(constantLit);

        if (rootConsts > 0) {
            // This is a root-scoped constant, like `class ::A; end`.
            // These are exempted from package prefix checking.
            return;
        }

        if (hasParentClass(classDef)) {
            // A class definition that includes a parent `class Foo::Bar < Baz`
            // must be made in that package
            checkBehaviorLoc(ctx, classDef.declLoc);
            return;
        }

        auto isOnPackagePath = onPackagePath(ctx);

        if (!isOnPackagePath) {
            ENFORCE(errorDepth == 0);
            errorDepth++;
            if (auto e = ctx.beginError(constantLit->loc(), core::errors::Packager::DefinitionPackageMismatch)) {
                definitionPackageMismatch(ctx, e, isOnPackagePath);
            }
        } else if (mustUseTestNamespace && !inTestNamespace(ctx)) {
            ENFORCE(errorDepth == 0);
            errorDepth++;
            if (auto e = ctx.beginError(constantLit->loc(), core::errors::Packager::DefinitionPackageMismatch)) {
                definitionPackageMismatch(ctx, e, isOnPackagePath);
            }
        }
    }

    void postTransformClassDef(core::Context ctx, const ast::ClassDef &classDef) {
        if (classDef.symbol == core::Symbols::root()) {
            // Sanity check bookkeeping
            ENFORCE(rootConsts == 0);
            ENFORCE(errorDepth == 0);
            return;
        }

        if (errorDepth > 0) {
            errorDepth--;
            // only continue if this was the first occurrence of the error
            if (errorDepth > 0) {
                return;
            }
        }

        auto constantLit = ast::cast_tree<ast::ConstantLit>(classDef.name);
        if (constantLit == nullptr) {
            return;
        }

        popScope(constantLit);
    }

    void preTransformAssign(core::Context ctx, const ast::Assign &asgn) {
        if (errorDepth > 0) {
            errorDepth++;
            return;
        }
        auto lhs = ast::cast_tree<ast::ConstantLit>(asgn.lhs);

        if (lhs == nullptr || rootConsts > 0) {
            return;
        }

        if (lhs->symbol().name(ctx).hasUniqueNameKind(ctx, core::UniqueNameKind::MangleRename)) {
            // Don't need to report definitionPackageMismatch if the symbol was mangle renamed
            return;
        }

        pushScope(lhs);

        if (rootConsts == 0) {
            auto isOnPackagePath = packageForNamespace(ctx) == pkg.mangledName();
            if (!isOnPackagePath) {
                ENFORCE(errorDepth == 0);
                errorDepth++;
                if (auto e = ctx.beginError(lhs->loc(), core::errors::Packager::DefinitionPackageMismatch)) {
                    definitionPackageMismatch(ctx, e, isOnPackagePath);
                }
            } else if (mustUseTestNamespace && !inTestNamespace(ctx)) {
                ENFORCE(errorDepth == 0);
                errorDepth++;
                if (auto e = ctx.beginError(lhs->loc(), core::errors::Packager::DefinitionPackageMismatch)) {
                    definitionPackageMismatch(ctx, e, isOnPackagePath);
                }
            }
        }

        popScope(lhs);
    }

    void postTransformAssign(core::Context ctx, const ast::Assign &asgn) {
        if (errorDepth > 0) {
            errorDepth--;
        }
    }

    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &def) {
        if (errorDepth > 0) {
            errorDepth++;
            return;
        }
        checkBehaviorLoc(ctx, def.declLoc);
    }

    void postTransformMethodDef(core::Context ctx, const ast::MethodDef &def) {
        if (errorDepth > 0) {
            errorDepth--;
        }
    }

    void preTransformSend(core::Context ctx, const ast::Send &send) {
        if (errorDepth > 0) {
            errorDepth++;
            return;
        }
        checkBehaviorLoc(ctx, send.loc);
    }

    void postTransformSend(core::Context ctx, const ast::Send &send) {
        if (errorDepth > 0) {
            errorDepth--;
        }
    }

    void checkBehaviorLoc(core::Context ctx, core::LocOffsets loc) {
        ENFORCE(errorDepth == 0);
        if (rootConsts > 0 || scope.empty()) {
            // Doing `class ::A; end` to monkey patch something lets you define behavior (monkey patch)
            // You can also do arbitrary behavior at the top-level outside of any definitions.
            // (Stripe's codebase enforces that the )
            return;
        }
        auto pkgForNamespace = packageForNamespace(ctx);
        if (pkgForNamespace != pkg.mangledName()) {
            ENFORCE(errorDepth == 0);
            errorDepth++;
            if (auto e = ctx.beginError(loc, core::errors::Packager::DefinitionPackageMismatch)) {
                e.setHeader("This file must only define behavior in enclosing package `{}`", requiredNamespace(ctx));
                const auto &[scopeSym, scopeLoc] = scope.back();
                e.addErrorLine(ctx.locAt(scopeLoc), "Defining behavior in `{}` instead:", scopeSym.show(ctx));
                e.addErrorLine(pkg.declLoc(), "Enclosing package `{}` declared here", pkg.mangledName_.owner.show(ctx));
                if (pkgForNamespace.exists()) {
                    auto &packageInfo = ctx.state.packageDB().getPackageInfo(pkgForNamespace);
                    e.addErrorLine(packageInfo.declLoc(), "Package `{}` declared here", scopeSym.show(ctx));
                }
            }
        }
    }

private:
    void pushScope(const ast::ConstantLit *lit) {
        scope.emplace_back(lit->symbol(), lit->loc());
        if (isRootScopedDefinition(lit)) {
            rootConsts++;
        }
    }

    void popScope(const ast::ConstantLit *lit) {
        if (isRootScopedDefinition(lit)) {
            rootConsts--;
        }
        scope.pop_back();
    }

    MangledName packageForNamespace(const core::GlobalState &gs) const {
        const auto &[scopeSym, _scopeLoc] = scope.back();
        return scopeSym.enclosingClass(gs).data(gs)->package;
    }

    bool onPackagePath(const core::GlobalState &gs) const {
        const auto &[scopeSym, _scopeLoc] = scope.back();

        core::ClassOrModuleRef klassSym;
        bool couldBePrefix = true;
        if (!scopeSym.isClassOrModule()) {
            couldBePrefix = false;
            klassSym = scopeSym.enclosingClass(gs);
        } else {
            klassSym = scopeSym.asClassOrModuleRef();
        }
        auto klassData = klassSym.data(gs);
        auto pkgForScope = klassData->package;
        auto ownerForScope = klassData->packageRegistryOwner;
        if (!ownerForScope.exists()) {
            couldBePrefix = false;
        }

        if (pkgForScope == this->pkg.mangledName()) {
            return true;
        } else if (couldBePrefix) {
            return ownsPackage(gs, ownerForScope, this->pkg.mangledName());
        } else {
            return pkgForScope == this->pkg.mangledName();
        }
    }

    bool inTestNamespace(const core::GlobalState &gs) {
        const auto &[scopeSym, _scopeLoc] = scope.back();
        auto cur = scopeSym;
        while (cur.exists() && cur != core::Symbols::root()) {
            if (cur == maybeTestNamespace) {
                return true;
            }

            cur = cur.owner(gs);
        }

        return false;
    }

    const string requiredNamespace(const core::GlobalState &gs) const {
        auto result = pkg.mangledName_.owner.show(gs);
        if (mustUseTestNamespace) {
            result = fmt::format("{}::{}", PackageDB::TEST_NAMESPACE.show(gs), result);
        }
        return result;
    }

    bool hasParentClass(const ast::ClassDef &def) const {
        return def.kind == ast::ClassDef::Kind::Class && !def.ancestors.empty() &&
               ast::isa_tree<ast::UnresolvedConstantLit>(def.ancestors[0]);
    }

    void definitionPackageMismatch(const core::GlobalState &gs, core::ErrorBuilder &e, bool isOnPackagePath) const {
        auto requiredName = requiredNamespace(gs);
        if (mustUseTestNamespace) {
            e.setHeader("Tests in the `{}` package must define tests in the `{}` namespace", pkg.show(gs),
                        requiredName);
            // TODO: If the only thing missing is a `Test::` prefix (e.g., if this were not a test
            // file there would not have been an error), then we could suggest an autocorrect.
        } else {
            e.setHeader("File belongs to package `{}` but defines a constant that does not match this namespace",
                        requiredName);
        }

        e.addErrorLine(pkg.declLoc(), "Enclosing package declared here");

        if (!isOnPackagePath) {
            auto reqMangledName = packageForNamespace(gs);
            if (reqMangledName.exists()) {
                auto &reqPkg = gs.packageDB().getPackageInfo(reqMangledName);
                if (reqPkg.exists()) {
                    const auto &[scopeSym, _scopeLoc] = scope.back();
                    e.addErrorLine(reqPkg.declLoc(), "Must belong to this package, given constant name `{}`",
                                   scopeSym.show(gs));
                }
            }
        }
    }
};

struct PackageSpecBodyWalk {
    PackageSpecBodyWalk(PackageInfo &info) : info(info) {}

    PackageInfo &info;
    vector<Export> exported;
    bool foundFirstPackageSpec = false;
    bool foundLayerDeclaration = false;
    bool foundStrictDependenciesDeclaration = false;

    void postTransformSend(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);

        // Disallowed methods
        if (send.fun == core::Names::extend() || send.fun == core::Names::include()) {
            if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidPackageExpression)) {
                e.setHeader("Invalid expression in package: `{}` is not allowed", send.fun.shortName(ctx));
            }
            return;
        }

        // Sanity check arguments for unrecognized methods
        if (!isSpecMethod(send)) {
            for (auto &arg : send.posArgs()) {
                if (!ast::isa_tree<ast::Literal>(arg)) {
                    if (auto e = ctx.beginError(arg.loc(), core::errors::Packager::InvalidPackageExpression)) {
                        e.setHeader("Invalid expression in package: Arguments to functions must be literals");
                    }
                }
            }
        }

        if (send.fun == core::Names::export_()) {
            if (send.numPosArgs() == 1) {
                // null indicates an invalid export.
                if (auto target = verifyConstant(ctx, core::Names::export_(), send.getPosArg(0))) {
                    exported.emplace_back(getFullyQualifiedName(ctx, target), target->loc);
                }
            }
        } else if ((send.fun == core::Names::import() || send.fun == core::Names::testImport())) {
            if (send.numPosArgs() == 1) {
                // null indicates an invalid import.
                if (auto *target = verifyConstant(ctx, send.fun, send.getPosArg(0))) {
                    // Transform: `import Foo` -> `import <PackageSpecRegistry>::Foo`
                    auto &posArg = send.getPosArg(0);
                    auto importArg = move(posArg);
                    posArg = ast::packager::prependRegistry(move(importArg));

                    // No such thing as "wildcard imports"--imports must pass a complete package name.
                    auto allowNamespace = false;
                    info.importedPackageNames.emplace_back(resolvePackageName(ctx, target, allowNamespace),
                                                           method2ImportType(send), send.loc);
                }
                // also validate the keyword args, since one is valid
                for (auto [key, value] : send.kwArgPairs()) {
                    auto keyLit = ast::cast_tree<ast::Literal>(key);
                    ENFORCE(keyLit);
                    if (keyLit->asSymbol() == core::Names::only()) {
                        auto valLit = ast::cast_tree<ast::Literal>(value);
                        // if it's not a literal, then it'll get caught elsewhere
                        if (valLit && (!valLit->isString() || valLit->asString() != core::Names::testRb())) {
                            if (auto e =
                                    ctx.beginError(value.loc(), core::errors::Packager::InvalidPackageExpression)) {
                                e.setHeader("Invalid expression in package: the only valid value for `{}` is `{}`",
                                            "only:", "\"test_rb\"");
                            }
                        }
                    }
                }
            }
        } else if (send.fun == core::Names::exportAll()) {
            if (send.numPosArgs() == 0) {
                info.exportAll_ = true;
            }
        } else if (send.fun == core::Names::preludePackage() && !send.hasBlock() && !send.hasNonBlockArgs()) {
            info.isPreludePackage_ = true;
        } else if (send.fun == core::Names::visibleTo()) {
            if (send.numPosArgs() == 1) {
                if (auto target = ast::cast_tree<ast::Literal>(send.getPosArg(0))) {
                    // the only valid literal here is `visible_to "tests"`; others should be rejected
                    if (!target->isString() || target->asString() != core::Names::tests()) {
                        if (auto e = ctx.beginError(target->loc, core::errors::Packager::InvalidConfiguration)) {
                            e.setHeader("Argument to `{}` must be a constant or the string literal `{}`",
                                        send.fun.show(ctx), "\"tests\"");
                        }
                        return;
                    }
                    info.visibleToTests_ = true;
                } else if (auto target = ast::cast_tree<ast::Send>(send.getPosArg(0))) {
                    // Constant::* is valid Ruby, and parses as a send of the method * to Constant
                    // so let's take advantage of this to implement wildcards
                    if (target->fun != core::Names::star() || target->numPosArgs() > 0 || target->numKwArgs() > 0 ||
                        target->hasBlock()) {
                        if (auto e = ctx.beginError(target->loc, core::errors::Packager::InvalidConfiguration)) {
                            e.setHeader("Argument to `{}` must be a constant or the string literal `{}`",
                                        send.fun.show(ctx), "\"tests\"");
                        }
                        return;
                    }

                    if (auto *recv = verifyConstant(ctx, send.fun, target->recv)) {
                        auto &posArg = send.getPosArg(0);
                        auto importArg = move(target->recv);
                        posArg = ast::packager::prependRegistry(move(importArg));
                        // Allow namespaces to enable wildcard visible_to of namespaces, not just rooted packages.
                        auto allowNamespace = true;
                        info.visibleTo_.emplace_back(resolvePackageName(ctx, recv, allowNamespace),
                                                     VisibleToType::Wildcard, send.loc);
                    } else {
                        if (auto e = ctx.beginError(target->loc, core::errors::Packager::InvalidConfiguration)) {
                            e.setHeader("Argument to `{}` must be a constant or the string literal `{}`",
                                        send.fun.show(ctx), "\"tests\"");
                        }
                        return;
                    }
                } else if (auto *target = verifyConstant(ctx, send.fun, send.getPosArg(0))) {
                    auto &posArg = send.getPosArg(0);
                    auto importArg = move(posArg);
                    posArg = ast::packager::prependRegistry(move(importArg));

                    auto allowNamespace = false;
                    info.visibleTo_.emplace_back(resolvePackageName(ctx, target, allowNamespace), VisibleToType::Normal,
                                                 send.loc);
                }
            }
        } else if (send.fun == core::Names::strictDependencies()) {
            foundStrictDependenciesDeclaration = true;
            if (!ctx.state.packageDB().enforceLayering()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidStrictDependencies)) {
                    e.setHeader("Found `{}` annotation, but `{}` was not passed", send.fun.show(ctx),
                                "--packager-layers");
                    e.addErrorNote("Use `{}` to define the valid layers, or `{}` to use the default layers "
                                   "of `{}` and `{}`",
                                   "--packager-layers=foo,bar", "--packager-layers", "library", "application");
                }
                return;
            }
            if (info.strictDependenciesLevel_.has_value()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::DuplicateDirective)) {
                    e.setHeader("Repeated declaration of `{}`", send.fun.show(ctx));
                    e.addErrorLine(ctx.locAt(info.strictDependenciesLevel_.value().second), "Previously declared here");
                    e.replaceWith("Remove this declaration", ctx.locAt(send.loc), "");
                }
                return;
            }

            if (send.numPosArgs() > 0) {
                auto parsedValue = parseStrictDependenciesOption(send.getPosArg(0));
                if (parsedValue.has_value()) {
                    info.strictDependenciesLevel_ = make_pair(parsedValue.value(), send.getPosArg(0).loc());
                } else {
                    if (auto e = ctx.beginError(send.argsLoc(), core::errors::Packager::InvalidStrictDependencies)) {
                        e.setHeader("Argument to `{}` must be one of: `{}`, `{}`, `{}`, or `{}`", send.fun.show(ctx),
                                    "'false'", "'layered'", "'layered_dag'", "'dag'");
                    }
                }
            }
        } else if (send.fun == core::Names::layer()) {
            foundLayerDeclaration = true;
            if (!ctx.state.packageDB().enforceLayering()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::InvalidLayer)) {
                    e.setHeader("Found `{}` annotation, but `{}` was not passed", send.fun.show(ctx),
                                "--packager-layers");
                    e.addErrorNote("Use `{}` to define the valid layers, or `{}` to use the default layers "
                                   "of `{}` and `{}`",
                                   "--packager-layers=foo,bar", "--packager-layers", "library", "application");
                }
                return;
            }
            if (info.layer_.has_value()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::DuplicateDirective)) {
                    e.setHeader("Repeated declaration of `{}`", send.fun.show(ctx));
                    e.addErrorLine(ctx.locAt(info.layer_.value().second), "Previously declared here");
                    e.replaceWith("Remove this declaration", ctx.locAt(send.loc), "");
                }
                return;
            }

            if (send.numPosArgs() > 0) {
                auto parsedValue = parseLayerOption(ctx.state, send.getPosArg(0));
                if (parsedValue.has_value()) {
                    info.layer_ = make_pair(parsedValue.value(), send.getPosArg(0).loc());
                } else {
                    if (auto e = ctx.beginError(send.argsLoc(), core::errors::Packager::InvalidLayer)) {
                        e.setHeader("Argument to `{}` must be one of: {}", send.fun.show(ctx),
                                    buildValidLayersStr(ctx.state));
                    }
                }
            }
        } else if (send.fun == core::Names::sorbet()) {
            // TODO(neil): enforce the minimum sigil declared here
            if (info.minTypedLevel_.has_value()) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::DuplicateDirective)) {
                    e.setHeader("Repeated declaration of `{}`", send.fun.show(ctx));
                    e.addErrorLine(ctx.locAt(info.minTypedLevel_.value().first.second), "Previously declared here");
                    e.replaceWith("Remove this declaration", ctx.locAt(send.loc), "");
                }
                return;
            }

            if (send.numKwArgs() >= 2) {
                std::optional<std::pair<core::StrictLevel, core::LocOffsets>> minTypedLevel;
                std::optional<std::pair<core::StrictLevel, core::LocOffsets>> testsMinTypedLevel;
                for (const auto [key, value] : send.kwArgPairs()) {
                    auto keyLit = ast::cast_tree<ast::Literal>(key);
                    ENFORCE(keyLit);
                    auto typedLevel = parseTypedLevelOption(ctx, value);
                    if (!typedLevel.has_value()) {
                        if (keyLit->asSymbol() == core::Names::minTypedLevel() ||
                            keyLit->asSymbol() == core::Names::testsMinTypedLevel()) {
                            if (auto e = ctx.beginError(send.argsLoc(), core::errors::Packager::InvalidMinTypedLevel)) {
                                e.setHeader("Argument to `{}` must be one of: `{}`, `{}`, `{}`, `{}`, or `{}`",
                                            keyLit->asSymbol().show(ctx), "ignore", "false", "true", "strict",
                                            "strong");
                            }
                        }
                        continue;
                    }
                    if (keyLit->asSymbol() == core::Names::minTypedLevel()) {
                        minTypedLevel = make_pair(typedLevel.value(), value.loc());
                    } else if (keyLit->asSymbol() == core::Names::testsMinTypedLevel()) {
                        testsMinTypedLevel = make_pair(typedLevel.value(), value.loc());
                    } else {
                        // Handled elsewhere
                    }
                }
                if (minTypedLevel.has_value() && testsMinTypedLevel.has_value()) {
                    info.minTypedLevel_ = make_pair(minTypedLevel.value(), testsMinTypedLevel.value());
                }
            }
        } else {
            // Extra directives
            info.extraDirectives_.push_back(send.loc);
        }
    }

    void preTransformClassDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (classDef.symbol == core::Symbols::root()) {
            // Skip over top-level <root>
            return;
        }

        if (!this->foundFirstPackageSpec) {
            auto packageSpecClass = ast::packager::asPackageSpecClass(tree);
            this->foundFirstPackageSpec |= (packageSpecClass != nullptr);
            return;
        }

        illegalNode(ctx, tree);
    }

    // Generate a list of FQNs exported by this package. No export may be a prefix of another.
    void finalize(core::Context ctx) {
        if (exported.empty()) {
            return;
        }

        if (info.exportAll()) {
            // we're only here because exports exist, which means if
            // `exportAll` is set then we've got conflicting
            // information about export; flag the exports as wrong
            for (auto it = exported.begin(); it != exported.end(); ++it) {
                if (auto e = ctx.beginError(it->loc, core::errors::Packager::ExportConflict)) {
                    e.setHeader("Package `{}` declares `{}` and therefore should not use explicit exports",
                                info.mangledName_.owner.show(ctx), "export_all!");
                }
            }
        }

        fast_sort(exported, Export::lexCmp);
        vector<size_t> dupInds;
        for (auto it = exported.begin(); it != exported.end(); ++it) {
            LexNext upperBound(it->parts());
            auto longer = it + 1;
            for (; longer != exported.end() && !(upperBound < *longer); ++longer) {
                if (auto e = ctx.beginError(longer->loc, core::errors::Packager::ExportConflict)) {
                    if (it->parts() == longer->parts()) {
                        e.setHeader("Duplicate export of `{}`",
                                    fmt::map_join(longer->parts(), "::", [&](const auto &nr) { return nr.show(ctx); }));
                    } else {
                        e.setHeader("Cannot export `{}` because another exported name `{}` is a prefix of it",
                                    fmt::map_join(longer->parts(), "::", [&](const auto &nr) { return nr.show(ctx); }),
                                    fmt::map_join(it->parts(), "::", [&](const auto &nr) { return nr.show(ctx); }));
                    }
                    e.addErrorLine(ctx.locAt(it->loc), "Prefix exported here");
                }

                dupInds.emplace_back(distance(exported.begin(), longer));
            }
        }

        // Remove duplicates we found (in reverse order)
        fast_sort(dupInds);
        dupInds.erase(unique(dupInds.begin(), dupInds.end()), dupInds.end());
        for (auto indIt = dupInds.rbegin(); indIt != dupInds.rend(); ++indIt) {
            // Yes this is quadratic, but this only happens in an error condition.
            exported.erase(exported.begin() + *indIt);
        }

        ENFORCE(info.exports_.empty());
        std::swap(exported, info.exports_);
    }

    bool isSpecMethod(const sorbet::ast::Send &send) const {
        switch (send.fun.rawId()) {
            case core::Names::import().rawId():
            case core::Names::testImport().rawId():
            case core::Names::export_().rawId():
            case core::Names::visibleTo().rawId():
            case core::Names::exportAll().rawId():
            case core::Names::preludePackage().rawId():
                return true;
            default:
                return false;
        }
    }

    ImportType method2ImportType(const ast::Send &send) const {
        switch (send.fun.rawId()) {
            case core::Names::import().rawId():
                return ImportType::Normal;
            case core::Names::testImport().rawId():
                // we'll validate elsewhere that the only valid keyword args to appear here are `only: :test_rb`,
                // which means if there are keyword args _at all_, they must indicate a test unit import
                if (send.numKwArgs() > 0) {
                    return ImportType::TestUnit;
                } else {
                    return ImportType::TestHelper;
                }
            default:
                ENFORCE(false);
                Exception::notImplemented();
        }
    }

    /* Forbid arbitrary computation in packages */
    void illegalNode(core::Context ctx, const ast::ExpressionPtr &original) {
        if (auto e = ctx.beginError(original.loc(), core::errors::Packager::InvalidPackageExpression)) {
            e.setHeader("Invalid expression in package: `{}` not allowed", original.nodeName());
            e.addErrorNote("To learn about what's allowed in `{}` files, see http://go/package-layout", "__package.rb");
        }
    }

    void preTransformExpressionPtr(core::Context ctx, const ast::ExpressionPtr &original) {
        auto tag = original.tag();
        if ( // PackageSpec definition; handled above explicitly
            tag == ast::Tag::ClassDef ||
            // Various DSL methods; handled above explicitly
            tag == ast::Tag::Send ||
            // Arguments to DSL methods; always allowed
            tag == ast::Tag::UnresolvedConstantLit || tag == ast::Tag::ConstantLit || tag == ast::Tag::Literal ||
            // Technically only in scopes of constant literals, but easier to just always allow
            tag == ast::Tag::EmptyTree ||
            // Technically only as receiver of DSL method, but easier to just always allow
            original.isSelfReference()) {
            return;
        }

        illegalNode(ctx, original);
    }

private:
    optional<StrictDependenciesLevel> parseStrictDependenciesOption(ast::ExpressionPtr &arg) {
        auto lit = ast::cast_tree<ast::Literal>(arg);
        if (!lit || !lit->isString()) {
            return nullopt;
        }
        auto value = lit->asString();

        if (value == core::Names::false_()) {
            return StrictDependenciesLevel::False;
        } else if (value == core::Names::layered()) {
            return StrictDependenciesLevel::Layered;
        } else if (value == core::Names::layeredDag()) {
            return StrictDependenciesLevel::LayeredDag;
        } else if (value == core::Names::dag()) {
            return StrictDependenciesLevel::Dag;
        }

        return nullopt;
    }

    optional<core::NameRef> parseLayerOption(const core::GlobalState &gs, ast::ExpressionPtr &arg) {
        auto validLayers = gs.packageDB().layers();
        auto lit = ast::cast_tree<ast::Literal>(arg);
        if (!lit || !lit->isString()) {
            return nullopt;
        }
        auto value = lit->asString();
        if (absl::c_find(validLayers, value) != validLayers.end()) {
            return value;
        }
        return nullopt;
    }

    optional<core::StrictLevel> parseTypedLevelOption(const core::GlobalState &gs, ast::ExpressionPtr &arg) {
        auto lit = ast::cast_tree<ast::Literal>(arg);
        if (!lit || !lit->isString()) {
            return nullopt;
        }
        auto value = lit->asString();
        auto strictLevel = core::SigilTraits<core::StrictLevel>::fromString(value.show(gs));
        switch (strictLevel) {
            case core::StrictLevel::Ignore:
            case core::StrictLevel::False:
            case core::StrictLevel::True:
            case core::StrictLevel::Strict:
            case core::StrictLevel::Strong:
                return strictLevel;
            default:
                return nullopt;
        }
    }
};

unique_ptr<PackageInfo> definePackage(core::GlobalState &gs, ast::ParsedFile &package) {
    ENFORCE(package.file.exists());
    ENFORCE(package.file.data(gs).isPackage(gs));
    // Assumption: Root of AST is <root> class. (This won't be true
    // for `typed: ignore` files, so we should make sure to catch that
    // elsewhere.)
    ENFORCE(ast::isa_tree<ast::ClassDef>(package.tree));
    ENFORCE(ast::cast_tree_nonnull<ast::ClassDef>(package.tree).symbol == core::Symbols::root());

    core::Context ctx(gs, core::Symbols::root(), package.file);

    auto &rootClass = ast::cast_tree_nonnull<ast::ClassDef>(package.tree);

    for (auto &rootStmt : rootClass.rhs) {
        auto packageSpecClass = ast::packager::asPackageSpecClass(rootStmt);
        if (packageSpecClass == nullptr) {
            // rewriter already reported an error
            continue;
        }

        // Eagerly resolve the superclass, so that we can rely on the
        packageSpecClass->symbol.data(gs)->setSuperClass(core::Symbols::PackageSpec());

        auto nameTree = ast::cast_tree<ast::ConstantLit>(packageSpecClass->name);
        ENFORCE(nameTree != nullptr, "Invariant from rewriter");

        return make_unique<PackageInfo>(MangledName(packageSpecClass->symbol), ctx.locAt(packageSpecClass->loc),
                                        ctx.locAt(packageSpecClass->declLoc));
    }

    return nullptr;
}

void rewritePackageSpec(const core::GlobalState &gs, ast::ParsedFile &package, PackageInfo &info) {
    PackageSpecBodyWalk bodyWalk(info);
    core::Context ctx(gs, core::Symbols::root(), package.file);
    ast::TreeWalk::apply(ctx, bodyWalk, package.tree);
    if (gs.packageDB().enforceLayering()) {
        if (!bodyWalk.foundLayerDeclaration) {
            if (auto e = gs.beginError(info.declLoc(), core::errors::Packager::InvalidLayer)) {
                e.setHeader("This package does not declare a `{}`", "layer");
            }
        }
        if (!bodyWalk.foundStrictDependenciesDeclaration) {
            if (auto e = gs.beginError(info.declLoc(), core::errors::Packager::InvalidStrictDependencies)) {
                e.setHeader("This package does not declare a `{}` level", "strict_dependencies");
            }
        }
    }
    bodyWalk.finalize(ctx);
}

void populatePackagePathPrefixes(core::GlobalState &gs, ast::ParsedFile &package, PackageInfo &info) {
    auto extraPackageFilesDirectoryUnderscorePrefixes = gs.packageDB().extraPackageFilesDirectoryUnderscorePrefixes();
    auto extraPackageFilesDirectorySlashDeprecatedPrefixes =
        gs.packageDB().extraPackageFilesDirectorySlashDeprecatedPrefixes();
    auto extraPackageFilesDirectorySlashPrefixes = gs.packageDB().extraPackageFilesDirectorySlashPrefixes();

    const auto numPrefixes = extraPackageFilesDirectoryUnderscorePrefixes.size() +
                             extraPackageFilesDirectorySlashDeprecatedPrefixes.size() +
                             extraPackageFilesDirectorySlashPrefixes.size() + 1;
    info.packagePathPrefixes.reserve(numPrefixes);
    auto packageFilePath = package.file.data(gs).path();
    ENFORCE(FileOps::getFileName(packageFilePath) == PACKAGE_FILE_NAME);
    info.packagePathPrefixes.emplace_back(packageFilePath.substr(0, packageFilePath.find_last_of('/') + 1));
    auto fullName = fullyQualifiedNameFromMangledName(gs, info.mangledName_);
    const auto shortName = absl::StrJoin(fullName, "_", NameFormatter(gs));
    const auto slashDirName = absl::StrJoin(fullName, "/", NameFormatter(gs)) + "/";
    const string_view dirNameFromShortName = shortName;

    for (const string &prefix : extraPackageFilesDirectoryUnderscorePrefixes) {
        // Project_FooBar -- munge with underscore
        info.packagePathPrefixes.emplace_back(absl::StrCat(prefix, dirNameFromShortName, "/"));
    }

    for (const string &prefix : extraPackageFilesDirectorySlashDeprecatedPrefixes) {
        // project/Foo_bar -- convert camel-case to snake-case and munge with slash
        string additionalDirPath;
        additionalDirPath.reserve(prefix.size() + 2 * dirNameFromShortName.length() + 1);
        additionalDirPath += prefix;
        for (int i = 0; i < dirNameFromShortName.length(); i++) {
            if (dirNameFromShortName[i] == '_') {
                additionalDirPath.push_back('/');
            } else if (i == 0 || dirNameFromShortName[i - 1] == '_') {
                // Capitalizing first letter in each directory name to avoid conflicts with ignored directories,
                // which tend to be all lower case
                additionalDirPath.push_back(std::toupper(dirNameFromShortName[i]));
            } else {
                if (isupper(dirNameFromShortName[i])) {
                    additionalDirPath.push_back('_'); // snake-case munging
                }

                additionalDirPath.push_back(std::tolower(dirNameFromShortName[i]));
            }
        }
        additionalDirPath.push_back('/');

        info.packagePathPrefixes.emplace_back(std::move(additionalDirPath));
    }

    for (const string &prefix : extraPackageFilesDirectorySlashPrefixes) {
        // Project/FooBar -- each constant name is a file or directory name
        info.packagePathPrefixes.emplace_back(absl::StrCat(prefix, slashDirName));
    }
}

void validateLayering(core::Context ctx, const Import &i) {
    if (i.isTestImport()) {
        return;
    }

    const auto &packageDB = ctx.state.packageDB();
    ENFORCE(packageDB.getPackageInfo(i.mangledName).exists())
    ENFORCE(packageDB.getPackageNameForFile(ctx.file).exists())
    auto &thisPkg = PackageInfo::from(ctx, packageDB.getPackageNameForFile(ctx.file));
    auto &otherPkg = packageDB.getPackageInfo(i.mangledName);
    ENFORCE(thisPkg.sccID().has_value(), "computeSCCs should already have been called and set sccID");
    ENFORCE(otherPkg.sccID().has_value(), "computeSCCs should already have been called and set sccID");

    if (!thisPkg.strictDependenciesLevel().has_value() || !otherPkg.strictDependenciesLevel().has_value() ||
        !thisPkg.layer().has_value() || !otherPkg.layer().has_value()) {
        return;
    }

    if (thisPkg.strictDependenciesLevel().value().first == StrictDependenciesLevel::False) {
        return;
    }

    auto pkgLayer = thisPkg.layer().value().first;
    auto otherPkgLayer = otherPkg.layer().value().first;

    if (thisPkg.causesLayeringViolation(packageDB, otherPkgLayer)) {
        if (auto e = ctx.beginError(i.loc, core::errors::Packager::LayeringViolation)) {
            e.setHeader("Layering violation: cannot import `{}` (in layer `{}`) from `{}` (in layer `{}`)",
                        otherPkg.show(ctx), otherPkgLayer.show(ctx), thisPkg.show(ctx), pkgLayer.show(ctx));
            e.addErrorLine(core::Loc(thisPkg.loc.file(), thisPkg.layer().value().second), "`{}`'s `{}` declared here",
                           thisPkg.show(ctx), "layer");
            e.addErrorLine(core::Loc(otherPkg.loc.file(), otherPkg.layer().value().second), "`{}`'s `{}` declared here",
                           otherPkg.show(ctx), "layer");
            // TODO(neil): if the import is unused (ie. there are no references in this package to the imported
            // package), autocorrect to delete import
        }
    }

    StrictDependenciesLevel otherPkgExpectedLevel = thisPkg.minimumStrictDependenciesLevel();

    if (otherPkg.strictDependenciesLevel().value().first < otherPkgExpectedLevel) {
        if (auto e = ctx.beginError(i.loc, core::errors::Packager::StrictDependenciesViolation)) {
            e.setHeader("Strict dependencies violation: All of `{}`'s `{}`s must be `{}` or higher", thisPkg.show(ctx),
                        "import", strictDependenciesLevelToString(otherPkgExpectedLevel));
            e.addErrorLine(core::Loc(otherPkg.loc.file(), otherPkg.strictDependenciesLevel().value().second),
                           "`{}`'s `{}` level declared here", otherPkg.show(ctx), "strict_dependencies");
            // TODO(neil): if the import is unused (ie. there are no references in this package to the imported
            // package), autocorrect to delete import
            // TODO(neil): if the imported package can be trivially upgraded to the required level (ex. it's at 'false'
            // but has no layering violations), autocorrect to do so
        }
    }

    if (thisPkg.strictDependenciesLevel().value().first >= StrictDependenciesLevel::LayeredDag) {
        if (thisPkg.sccID() == otherPkg.sccID()) {
            if (auto e = ctx.beginError(i.loc, core::errors::Packager::StrictDependenciesViolation)) {
                auto level =
                    fmt::format("strict_dependencies '{}'",
                                strictDependenciesLevelToString(thisPkg.strictDependenciesLevel().value().first));
                e.setHeader("Strict dependencies violation: importing `{}` will put `{}` into a cycle, which is not "
                            "valid at `{}`",
                            otherPkg.show(ctx), thisPkg.show(ctx), level);
                auto path = otherPkg.pathTo(ctx, thisPkg.mangledName());
                ENFORCE(path.has_value(),
                        "Path from otherPkg to thisPkg should always exist if they are in the same SCC");
                e.addErrorNote("Path from `{}` to `{}`:\n{}", otherPkg.show(ctx), thisPkg.show(ctx), path.value());
            }
            // TODO(neil): if the import is unused (ie. there are no references in this package to the imported
            // package), autocorrect to delete import
        }
    }
}

void validateVisibility(core::Context ctx, const PackageInfo &absPkg, const Import i) {
    ENFORCE(ctx.state.packageDB().getPackageInfo(i.mangledName).exists())
    ENFORCE(ctx.state.packageDB().getPackageNameForFile(ctx.file).exists())
    auto &otherPkg = ctx.state.packageDB().getPackageInfo(i.mangledName);

    const auto &visibleTo = otherPkg.visibleTo();
    if (visibleTo.empty() && !otherPkg.visibleToTests()) {
        return;
    }

    if (otherPkg.visibleToTests() && i.isTestImport()) {
        return;
    }

    bool allowed = absl::c_any_of(
        visibleTo, [&ctx, &absPkg](const auto &other) { return visibilityApplies(ctx, other, absPkg.mangledName()); });

    if (!allowed) {
        if (auto e = ctx.beginError(i.loc, core::errors::Packager::ImportNotVisible)) {
            e.setHeader("Package `{}` includes explicit visibility modifiers and cannot be imported from `{}`",
                        otherPkg.show(ctx), absPkg.show(ctx));
            e.addErrorNote("Please consult with the owning team before adding a `{}` line to the package `{}`",
                           "visible_to", otherPkg.show(ctx));
        }
    }
}

void validatePreludePackage(const core::GlobalState &gs, core::FileRef file, PackageInfo &info) {
    auto &packageDB = gs.packageDB();

    // Prelude packages must be in the lowest possible layer, as that's the only way to ensure that the implicit
    // imports of them are valid.
    auto enforceLayering = gs.packageDB().enforceLayering();
    if (enforceLayering) {
        if (auto layer = info.layer()) {
            if (packageDB.layerIndex(layer->first) != 0) {
                core::Loc layerLoc(file, layer->second);
                if (auto e = gs.beginError(layerLoc, core::errors::Packager::PreludeLowestLayer)) {
                    auto layers = packageDB.layers();
                    e.setHeader("Prelude package `{}` must be in the lowest layer, `{}`", info.show(gs),
                                layers.front().toString(gs));
                }

                // Overwrite the invalid layer to ensure that we don't see additional errors from other packages.
                info.layer_->first = packageDB.layers().front();
            }
        }
    }

    // We disallow `visible_to` annotations in prelude packages, as they're implicitly imported by all non-prelude
    // packages.
    for (auto &v : info.visibleTo()) {
        if (auto e = gs.beginError(core::Loc(file, v.loc), core::errors::Packager::NoPreludeVisibleTo)) {
            e.setHeader("Prelude package `{}` may not include `{}` annotations", info.show(gs), "visible_to");
            e.addErrorNote("Prelude packages are implicitly imported by all other packages, which would\n"
                           "mean that `{}` annotations would have to be added to for every package to be\n"
                           "valid.",
                           "visible_to");
        }
    }

    // Ensure that there aren't any `visible_to` annotations present, as they will cause errors with all other packages.
    info.visibleTo_.clear();
}

void validatePackage(core::Context ctx) {
    const auto &packageDB = ctx.state.packageDB();
    auto absPkg = packageDB.getPackageNameForFile(ctx.file);
    if (!absPkg.exists()) {
        // We already produced an error on this package when producing its package info.
        // The correct course of action is to abort the transform.
        return;
    }

    // Sanity check: __package.rb files _must_ be typed: strict
    if (ctx.file.data(ctx).originalSigil < core::StrictLevel::Strict) {
        if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::PackageFileMustBeStrict)) {
            e.setHeader("Package files must be at least `{}`", "# typed: strict");
            // TODO(neil): Autocorrect to update the sigil?
        }
    }

    auto &pkgInfo = PackageInfo::from(ctx, absPkg);
    bool skipImportVisibilityCheck = packageDB.allowRelaxedPackagerChecksFor(pkgInfo.mangledName());
    auto enforceLayering = ctx.state.packageDB().enforceLayering();

    if (skipImportVisibilityCheck && !enforceLayering) {
        return;
    }

    bool pkgIsPrelude = pkgInfo.isPreludePackage();
    for (auto &i : pkgInfo.importedPackageNames) {
        auto &otherPkg = packageDB.getPackageInfo(i.mangledName);

        // this might mean the other package doesn't exist, but that should have been caught already
        if (!otherPkg.exists()) {
            continue;
        }

        if (pkgIsPrelude) {
            // Prelude packages may only import other prelude packages
            ENFORCE(!i.isPrelude(), "Prelude packages shouldn't have implicit imports");
            if (!otherPkg.isPreludePackage()) {
                if (auto e = ctx.beginError(i.loc, core::errors::Packager::PreludePackageImport)) {
                    string_view import;
                    switch (i.type) {
                        case core::packages::ImportType::Normal:
                            import = "import";
                            break;
                        case core::packages::ImportType::TestHelper:
                        case core::packages::ImportType::TestUnit:
                            import = "test_import";
                            break;
                    }
                    e.setHeader("Prelude package `{}` may not `{}` non-prelude package `{}`", pkgInfo.show(ctx), import,
                                otherPkg.show(ctx));
                }
            }
        } else {
            // Implicit imports of prelude packages are all added with locs that don't exist.
            if (!i.isPrelude() && otherPkg.isPreludePackage()) {
                if (auto e = ctx.beginError(i.loc, core::errors::Packager::NoExplicitPreludeImport)) {
                    e.setHeader("Prelude package `{}` may not be explicitly imported", otherPkg.show(ctx));
                }
            }
        }

        if (enforceLayering) {
            validateLayering(ctx, i);
        }

        if (!skipImportVisibilityCheck) {
            validateVisibility(ctx, pkgInfo, i);
        }

        if (i.mangledName == pkgInfo.mangledName()) {
            if (auto e = ctx.beginError(i.loc, core::errors::Packager::NoSelfImport)) {
                string import_;
                switch (i.type) {
                    case ImportType::Normal:
                        import_ = "import";
                        break;
                    case ImportType::TestUnit:
                    case ImportType::TestHelper:
                        import_ = "test_import";
                        break;
                }
                e.setHeader("Package `{}` cannot {} itself", pkgInfo.mangledName_.owner.show(ctx), import_);
            }
        }
    }
}

void validatePackagedFile(core::Context ctx, const ast::ExpressionPtr &tree) {
    auto &file = ctx.file.data(ctx);
    ENFORCE(!file.isPackage(ctx));

    if (file.isPayload()) {
        // Files in Sorbet's payload are parsed and loaded in the --store-state phase, which runs
        // outside of the packager mode. They're allowed to not belong to a package.
        //
        // Note that other RBIs that are not in Sorbet's payload follow the normal packaging rules.
        //
        // We normally skip running the packager when building in sorbet-orig mode, which computes
        // the stored state, but payload files can be retypechecked by the fast path during LSP.
        return;
    }

    auto pkg = ctx.state.packageDB().getPackageNameForFile(ctx.file);
    if (!pkg.exists()) {
        // Don't transform, but raise an error on the first line.
        if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::UnpackagedFile)) {
            e.setHeader("File `{}` does not belong to a package; add a `{}` file to one "
                        "of its parent directories",
                        ctx.file.data(ctx).path(), PACKAGE_FILE_NAME);
        }
        return;
    }

    auto &pkgImpl = PackageInfo::from(ctx, pkg);

    EnforcePackagePrefix enforcePrefix(ctx, pkgImpl);
    ast::ConstShallowWalk::apply(ctx, enforcePrefix, tree);
}

} // namespace

void Packager::findPackages(core::GlobalState &gs, absl::Span<ast::ParsedFile> files) {
    // Ensure files are in canonical order.
    // TODO(jez) Is this sort redundant? Should we move this sort to callers?
    fast_sort(files, [](const auto &a, const auto &b) -> bool { return a.file < b.file; });

    Timer timeit(gs.tracer(), "packager.findPackages");

    // Find packages and determine their imports/exports.
    {
        core::UnfreezeNameTable unfreeze(gs);
        UnfreezePackages packages = gs.unfreezePackages();
        for (auto &file : files) {
            if (!file.file.data(gs).isPackage(gs)) {
                continue;
            }

            auto pkg = definePackage(gs, file);
            if (pkg == nullptr) {
                // There was an error creating a PackageInfo for this file, and getPackageInfo has already
                // surfaced that error to the user. Nothing to do here.
                continue;
            }

            auto &prevPkg = gs.packageDB().getPackageInfo(pkg->mangledName());
            if (prevPkg.exists() && prevPkg.declLoc() != pkg->declLoc()) {
                if (auto e = gs.beginError(pkg->loc, core::errors::Packager::RedefinitionOfPackage)) {
                    auto pkgName = pkg->mangledName_.owner.show(gs);
                    e.setHeader("Redefinition of package `{}`", pkgName);
                    e.addErrorLine(prevPkg.declLoc(), "Package `{}` originally defined here", pkgName);
                }
            } else {
                populatePackagePathPrefixes(gs, file, *pkg);
                packages.db.enterPackage(move(pkg));
            }
        }

        // Must be called after any calls to enterPackage (i.e., only here)
        gs.packageDB().resolvePackagesWithRelaxedChecks(gs);
    }

    setPackageNameOnFiles(gs, files);

    {
        core::UnfreezeNameTable unfreeze(gs);
        auto packages = gs.unfreezePackages();

        vector<MangledName> preludePackages;
        for (auto &file : files) {
            if (!file.file.data(gs).isPackage(gs)) {
                continue;
            }

            auto pkgName = gs.packageDB().getPackageNameForFile(file.file);
            if (!pkgName.exists()) {
                continue;
            }
            auto &info = PackageInfo::from(gs, pkgName);
            rewritePackageSpec(gs, file, info);

            if (info.isPreludePackage()) {
                preludePackages.emplace_back(pkgName);
                validatePreludePackage(gs, file.file, info);
            }
        }

        // Now that we know the set of prelude packages, we can loop over the whole package DB and add implicit imports
        // to the imports of the non-prelude set.
        if (!preludePackages.empty()) {
            for (auto pkgName : gs.packageDB().packages()) {
                auto *info = gs.packageDB().getPackageInfoNonConst(pkgName);
                ENFORCE(info != nullptr);
                if (info->isPreludePackage()) {
                    continue;
                }

                // Extend the imports with the implicit prelude imports, which can be identified by their non-existant
                // locs.
                auto implicitImportLoc = info->declLoc_.offsets();
                info->importedPackageNames.reserve(info->importedPackageNames.size() + preludePackages.size());
                absl::c_transform(preludePackages, back_inserter(info->importedPackageNames),
                                  [implicitImportLoc](auto name) { return Import::prelude(name, implicitImportLoc); });
            }
        }
    }
}

namespace {

template <typename Elem, typename Proj>
void setPackageNameOnFilesImpl(core::GlobalState &gs, absl::Span<const Elem> files, Proj &&proj) {
    vector<pair<core::FileRef, MangledName>> mapping;
    mapping.reserve(files.size());

    {
        auto &db = gs.packageDB();
        for (auto &f : files) {
            auto fref = proj(f);

            auto pkg = db.getPackageNameForFile(fref);
            if (pkg.exists()) {
                continue;
            }

            pkg = db.findPackageByPath(gs, fref);
            if (!pkg.exists()) {
                continue;
            }

            mapping.emplace_back(fref, pkg);
        }
    }

    {
        auto packages = gs.unfreezePackages();
        for (auto [file, package] : mapping) {
            packages.db.setPackageNameForFile(file, package);
        }
    }
}

} // namespace

void Packager::setPackageNameOnFiles(core::GlobalState &gs, absl::Span<const ast::ParsedFile> files) {
    setPackageNameOnFilesImpl(gs, files, [](auto &p) { return p.file; });
}

void Packager::setPackageNameOnFiles(core::GlobalState &gs, absl::Span<const core::FileRef> files) {
    setPackageNameOnFilesImpl(gs, files, [](auto f) { return f; });
}

namespace {

enum class PackagerMode {
    PackagesOnly,
    PackagedFilesOnly,
    AllFiles,
};

class PackageDBPackageGraph {
    PackageDB &packageDB;

public:
    PackageDBPackageGraph(PackageDB &packageDB) : packageDB(packageDB) {}

    const absl::Span<const Import> getImports(MangledName packageName) const {
        ENFORCE(packageDB.getPackageInfo(packageName).exists());
        return packageDB.getPackageInfo(packageName).importedPackageNames;
    }

    void setSCCId(MangledName packageName, int sccID) {
        auto *pkgInfoPtr = packageDB.getPackageInfoNonConst(packageName);
        if (!pkgInfoPtr) {
            return;
        }
        pkgInfoPtr->sccID_ = sccID;
    }

    int getSCCId(MangledName packageName) const {
        ENFORCE(packageDB.getPackageInfo(packageName).exists());
        ENFORCE(packageDB.getPackageInfo(packageName).sccID().has_value());
        return packageDB.getPackageInfo(packageName).sccID().value();
    }

    void setTestSCCId(MangledName packageName, int sccID) {
        auto *pkgInfoPtr = packageDB.getPackageInfoNonConst(packageName);
        if (!pkgInfoPtr) {
            return;
        }
        pkgInfoPtr->testSccID_ = sccID;
    }
};

template <PackagerMode Mode>
void packageRunCore(core::GlobalState &gs, WorkerPool &workers, absl::Span<ast::ParsedFile> files) {
    ENFORCE(!gs.cacheSensitiveOptions.runningUnderAutogen, "Packager pass does not run in autogen");

    Timer timeit(gs.tracer(), "packager");

    switch (Mode) {
        case PackagerMode::PackagesOnly:
            timeit.setTag("mode", "packages_only");
            break;
        case PackagerMode::PackagedFilesOnly:
            timeit.setTag("mode", "packaged_files_only");
            break;
        case PackagerMode::AllFiles:
            break;
    }

    constexpr bool buildPackageDB = Mode == PackagerMode::PackagesOnly || Mode == PackagerMode::AllFiles;
    constexpr bool validatePackagedFiles = Mode == PackagerMode::PackagedFilesOnly || Mode == PackagerMode::AllFiles;

    if constexpr (buildPackageDB) {
        Packager::findPackages(gs, files);
    }

    {
        Timer timeit(gs.tracer(), "packager.rewritePackagesAndFiles");

        if constexpr (buildPackageDB) {
            PackageDBPackageGraph packageGraph{gs.packageDB()};
            gs.packageDB().setCondensation(ComputePackageSCCs::run(gs, packageGraph));
        }

        {
            Timer timeit(gs.tracer(), "packager.validatePackagesAndFiles");
            Parallel::iterate(workers, "validatePackagesAndFiles", absl::MakeSpan(files),
                              [&gs = as_const(gs)](auto &job) {
                                  auto file = job.file;
                                  core::Context ctx(gs, core::Symbols::root(), file);

                                  if (file.data(gs).isPackage(gs)) {
                                      ENFORCE(buildPackageDB);
                                      validatePackage(ctx);
                                  } else {
                                      ENFORCE(validatePackagedFiles);
                                      validatePackagedFile(ctx, job.tree);
                                  }
                              });
        }
    }
}

bool isTestExport(const ast::ExpressionPtr &expr) {
    auto send = ast::cast_tree<ast::Send>(expr);
    if (!send || send->fun != core::Names::export_() || send->numPosArgs() != 1) {
        return false;
    }

    auto sym = ast::cast_tree<ast::UnresolvedConstantLit>(send->getPosArg(0));
    while (sym) {
        if (ast::isa_tree<ast::EmptyTree>(sym->scope)) {
            return sym->cnst == core::Names::Constants::Test();
        }

        if (auto parent = ast::cast_tree<ast::UnresolvedConstantLit>(sym->scope)) {
            sym = parent;
        } else {
            break;
        }
    }

    return false;
}

} // namespace

void Packager::run(core::GlobalState &gs, WorkerPool &workers, absl::Span<ast::ParsedFile> files) {
    packageRunCore<PackagerMode::AllFiles>(gs, workers, files);
}

vector<ast::ParsedFile> Packager::runIncremental(const core::GlobalState &gs, vector<ast::ParsedFile> files,
                                                 WorkerPool &workers) {
    // Note: This will only run if packages have not been changed (byte-for-byte equality).
    // TODO(nroman-stripe) This could be further incrementalized to avoid processing all packages by
    // building in an understanding of the dependencies between packages.
    Timer timeit(gs.tracer(), "packager.runIncremental");

    Parallel::iterate(workers, "validatePackagesAndFiles", absl::MakeSpan(files), [&gs = as_const(gs)](auto &file) {
        core::Context ctx(gs, core::Symbols::root(), file.file);
        if (file.file.data(gs).isPackage(gs)) {
            auto packageName = gs.packageDB().getPackageNameForFile(file.file);
            auto &info = gs.packageDB().getPackageInfo(packageName);
            if (info.exists()) {
                // We aren't going to mutate the packageDB at this point, but we do need something to give to
                // rewritePackageSpec. We make the most shallow copy possible, to ensure that we don't raise duplicate
                // errors on the fast path.
                PackageInfo copy{info.mangledName_, info.loc, info.declLoc_};
                rewritePackageSpec(gs, file, copy);

                if (info.isPreludePackage()) {
                    validatePreludePackage(gs, file.file, copy);
                }
            }
            validatePackage(ctx);
        } else {
            validatePackagedFile(ctx, file.tree);
        }
    });

    return files;
}

void Packager::buildPackageDB(core::GlobalState &gs, WorkerPool &workers, absl::Span<ast::ParsedFile> packageFiles) {
    packageRunCore<PackagerMode::PackagesOnly>(gs, workers, packageFiles);
}

void Packager::validatePackagedFiles(core::GlobalState &gs, WorkerPool &workers, absl::Span<ast::ParsedFile> files) {
    packageRunCore<PackagerMode::PackagedFilesOnly>(gs, workers, files);
}

ast::ParsedFile Packager::copyPackageWithoutTestExports(const core::GlobalState &gs, const ast::ParsedFile &ast) {
    ENFORCE(ast.file.isPackage(gs));

    ast::ParsedFile result{ast.tree.deepCopy(), ast.file};

    auto root = ast::cast_tree<ast::ClassDef>(result.tree);
    if (!root || root->rhs.size() != 1) {
        return result;
    }

    auto package = ast::cast_tree<ast::ClassDef>(root->rhs.front());
    if (!package) {
        return result;
    }

    auto it = std::remove_if(package->rhs.begin(), package->rhs.end(), isTestExport);
    package->rhs.erase(it, package->rhs.end());

    return result;
}

} // namespace sorbet::packager
