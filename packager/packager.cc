#include "packager/packager.h"
#include "ast/packager/packager.h"
#include "ast/treemap/treemap.h"
#include "common/FileOps.h"
#include "common/concurrency/Parallel.h"
#include "common/sort/sort.h"
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

// TODO(jez) Might be nice to eagerly resolve these UnresolvedConstantLit to ConstantLit so resolver doesn't have to.
MangledName resolvePackageName(core::Context ctx, const ast::UnresolvedConstantLit *constantLit, bool allowNamespace) {
    ENFORCE(constantLit != nullptr);

    using ResolutionState = pair<core::ClassOrModuleRef, size_t>;
    auto [owner, parts] = constantLit->foldFromRoot(
        [&](const ast::ExpressionPtr &root) -> ResolutionState {
            auto resolved = ast::cast_tree<ast::ConstantLit>(root);
            ENFORCE(resolved == nullptr || resolved->original() == nullptr);
            return {core::Symbols::PackageSpecRegistry(), 0};
        },
        [&](ResolutionState state, core::NameRef name, core::LocOffsets loc) {
            state.second++;
            if (!state.first.exists()) {
                return state;
            }
            auto member = state.first.data(ctx)->findMember(ctx, name);
            state.first = member.exists() && member.isClassOrModule() ? member.asClassOrModuleRef()
                                                                      : core::Symbols::noClassOrModule();
            return state;
        });
    ENFORCE(parts > 0);

    if (owner == core::Symbols::PackageSpecRegistry()) {
        // This is a weird case, because I don't think it's possible to get here, but we can handle it anyways.
        // This whole function should go away with the switch to PackageRef anyways. As in, we
        // should probably be able to pre-resolve the constants in import/visible_to/etc. lines at
        // this point, and report an eager error if those package names fail to resolve, rather than
        // resorting handling this (impossible?) edge case.
        ENFORCE(parts == 0);
        owner = core::Symbols::noClassOrModule();
    }

    if (owner.exists()) {
        if (!allowNamespace && owner.data(ctx)->superClass() != core::Symbols::PackageSpec()) {
            return MangledName();
        }
    }

    return MangledName(owner);
}

const ast::UnresolvedConstantLit *verifyConstant(core::Context ctx, core::NameRef fun, const ast::ExpressionPtr &expr) {
    auto target = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    if (target == nullptr) {
        if (auto e = ctx.beginError(expr.loc(), core::errors::Packager::InvalidConfiguration)) {
            e.setHeader("Argument to `{}` must be a constant", fun.show(ctx));
        }
        return nullptr;
    }

    const auto &outermostScope = target->outermostScope();
    if (ast::isa_tree<ast::EmptyTree>(outermostScope)) {
        return target;
    }
    if (auto scope = ast::cast_tree<ast::ConstantLit>(outermostScope);
        scope != nullptr && scope->symbol() == core::Symbols::root()) {
        if (auto e = ctx.beginError(expr.loc(), core::errors::Packager::InvalidConfiguration)) {
            e.setHeader("Argument to `{}` must not use a `{}` prefix", fun.show(ctx), "::");
        }
    } else if (auto e = ctx.beginError(expr.loc(), core::errors::Packager::InvalidConfiguration)) {
        e.setHeader("Argument to `{}` must be a constant", fun.show(ctx));
    }
    return nullptr;
}

bool isRootScopedDefinition(const ast::ConstantLit *lit) {
    bool isRootScoped = false;
    lit->walkOriginalPath(
        [&](core::LocOffsets, core::SymbolRef symbol, bool isConstant) {
            isRootScoped = isConstant && symbol == core::Symbols::root();
            return true;
        },
        [](core::LocOffsets, core::SymbolRef, core::NameRef, bool) { return true; });
    return isRootScoped;
}

// Namespace definitions are checked by namer. This visitor checks behavior in package namespaces.
class EnforcePackageBehavior final {
    const PackageInfo &pkg;

    // By contrast with `Context::owner`, this `scope` field:
    //
    // - Only tracks constant symbols (`owner` will be a MethodRef inside `{pre,post}TransformMethodDef`)
    // - `Context::owner` does not track a loc
    vector<pair<core::SymbolRef, core::LocOffsets>> scope;

    // Meant to track when we're inside something like `class ::A; class B; end; end` instead of
    // `class A; class B; end; end`. Classes that start from an absolutely qualified "cbase" with a
    // leading `::` are opted out of the package behavior checks in prelude packages.
    //
    // TODO(jez) Document this in the public docs for the packager
    //   (at least in the error reference, but also in any eventual docs on the package system).
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
    EnforcePackageBehavior(core::Context ctx, const PackageInfo &pkg) : pkg(pkg) {
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

        if (rootConsts > 0 && pkg.isPreludePackage()) {
            // This is a root-scoped constant, like `class ::A; end`.
            // These are exempted from package prefix checking.
            return;
        }

        auto data = classDef.symbol.data(ctx);
        if (!data->package.exists() && !data->packageRegistryOwner.exists()) {
            // Namer isolates invalid definitions as unpackaged symbols. Suppress behavior errors
            // throughout the lexical subtree, including nested definitions that reset their scope.
            ENFORCE(errorDepth == 0);
            errorDepth++;
        } else if (hasParentClass(classDef)) {
            // A class definition that includes a parent `class Foo::Bar < Baz`
            // must be made in that package
            checkBehaviorLoc(ctx, classDef.declLoc);
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
        if (lhs != nullptr && lhs->symbol().name(ctx).hasUniqueNameKind(ctx, core::UniqueNameKind::MangleRename)) {
            // Namer has already rejected this assignment. Do not diagnose behavior in its RHS.
            errorDepth++;
        }
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
        if ((rootConsts > 0 && pkg.isPreludePackage()) || scope.empty()) {
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
                e.setHeader("This file must only define behavior in enclosing package `{}`",
                            pkg.mangledName_.owner.show(ctx));
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

    bool hasParentClass(const ast::ClassDef &def) const {
        return def.kind == ast::ClassDef::Kind::Class && !def.ancestors.empty() &&
               ast::isa_tree<ast::UnresolvedConstantLit>(def.ancestors[0]);
    }
};

struct PackageSpecBodyWalk {
    PackageSpecBodyWalk(PackageInfo &info) : info(info) {}

    PackageInfo &info;
    bool foundFirstPackageSpec = false;
    bool foundLayerDeclaration = false;
    bool foundStrictDependenciesDeclaration = false;
    bool foundSorbetDeclaration = false;

    void postTransformSend(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);

        // Disallowed methods. Errors will be raised in the resolver when processing the mixins.
        if (send.fun == core::Names::extend() || send.fun == core::Names::include()) {
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
            // Exports have no meaning, because constants are public unless marked `package_private!`. Drop them without
            // resolving them, so that exports of constants that no longer exist don't report errors.
            tree = ast::make_expression<ast::EmptyTree>();
            return;
        } else if (send.fun == core::Names::import()) {
            if (send.numPosArgs() == 1) {
                Import *imp = nullptr;

                // null indicates an invalid import.
                if (auto *target = verifyConstant(ctx, send.fun, send.getPosArg(0))) {
                    // Transform: `import Foo` -> `import <PackageSpecRegistry>::Foo`
                    auto &posArg = send.getPosArg(0);
                    auto importArg = move(posArg);
                    posArg = ast::packager::prependRegistry(move(importArg));

                    // No such thing as "wildcard imports"--imports must pass a complete package name.
                    auto allowNamespace = false;
                    auto importName = resolvePackageName(ctx, target, allowNamespace);
                    if (importName == info.mangledName()) {
                        if (auto e = ctx.beginError(send.loc, core::errors::Packager::NoSelfImport)) {
                            e.setHeader("Package `{}` cannot {} itself", info.show(ctx), send.fun.show(ctx));
                        }
                    }

                    auto skipRecordingImport = false;
                    if (info.isPreludePackage()) {
                        auto &otherPkg = ctx.state.packageDB().getPackageInfo(importName);
                        if (otherPkg.exists()) {
                            // Prelude packages may only import other prelude packages
                            if (!otherPkg.isPreludePackage()) {
                                skipRecordingImport = true;
                                if (auto e = ctx.beginError(send.loc, core::errors::Packager::PreludePackageImport)) {
                                    e.setHeader("Prelude package `{}` may not `{}` non-prelude package `{}`",
                                                info.show(ctx), "import", otherPkg.show(ctx));

                                    auto importLoc = ctx.locAt(send.loc);
                                    auto line = importLoc.toDetails(ctx.state).first.line;
                                    auto &file = importLoc.file().data(ctx.state);
                                    auto fileEnd = static_cast<uint32_t>(file.source().size());
                                    if (auto lineStart = core::Loc::detail2Pos(file, {line, 1})) {
                                        auto nextLineStart =
                                            core::Loc::detail2Pos(file, {line + 1, 1}).value_or(fileEnd);
                                        nextLineStart = min(nextLineStart, fileEnd);
                                        e.replaceWith("Delete invalid import",
                                                      core::Loc(importLoc.file(), *lineStart, nextLineStart), "");
                                    }
                                }
                            }
                        }
                    }

                    if (!skipRecordingImport) {
                        imp = &info.importedPackageNames.emplace_back(importName, send.loc);
                    }
                }

                // also validate the keyword args, since one is valid
                for (auto [key, value] : send.kwArgPairs()) {
                    auto keyLit = ast::cast_tree<ast::Literal>(key);
                    ENFORCE(keyLit);
                    switch (keyLit->asSymbol().rawId()) {
                        case core::Names::usesInternals().rawId():
                            auto valLit = ast::cast_tree<ast::Literal>(value);
                            if (valLit && valLit->isTrue(ctx)) {
                                if (imp) {
                                    imp->usesInternals = true;
                                }
                            } else {
                                if (auto e =
                                        ctx.beginError(value.loc(), core::errors::Packager::InvalidPackageExpression)) {
                                    e.setHeader("Invalid expression in package: the only valid value for `{}` is `{}`",
                                                "uses_internals:", "true");
                                }
                            }

                            break;
                    }
                }
            }
        } else if (send.fun == core::Names::exportAll()) {
            // Handled as a class-level modifier in namer.
        } else if (send.fun == core::Names::prelude_bang() && !send.hasBlock() && !send.hasNonBlockArgs()) {
            // Handled as a class-level modifier in namer.
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
            auto seenStrictDependenciesDecl = this->foundStrictDependenciesDeclaration;
            this->foundStrictDependenciesDeclaration = true;

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

            auto parsedValue = StrictDependenciesLevel::None;
            if (send.numPosArgs() > 0) {
                parsedValue = parseStrictDependenciesOption(send.getPosArg(0));
            }

            // We explicitly disallow duplicate `strict_dependencies` declarations. Additionally, if we're processing a
            // __package.rb file in an already populated PackageDB, we require that the declaration matches the value
            // that's already present.
            bool valueMatches =
                !info.locs.strictDependenciesLevel.exists() || parsedValue == info.strictDependenciesLevel;
            if (seenStrictDependenciesDecl || !valueMatches) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::DuplicateDirective)) {
                    e.setHeader("Repeated declaration of `{}`", send.fun.show(ctx));
                    e.addErrorLine(ctx.locAt(info.locs.strictDependenciesLevel), "Previously declared here");
                    e.replaceWith("Remove this declaration", ctx.locAt(send.loc), "");
                }
                return;
            }

            if (send.numPosArgs() > 0) {
                if (parsedValue != StrictDependenciesLevel::None) {
                    info.strictDependenciesLevel = parsedValue;
                    info.locs.strictDependenciesLevel = send.getPosArg(0).loc();
                } else {
                    if (auto e = ctx.beginError(send.argsLoc(), core::errors::Packager::InvalidStrictDependencies)) {
                        e.setHeader("Argument to `{}` must be one of: `{}`, `{}`, `{}`, or `{}`", send.fun.show(ctx),
                                    "'false'", "'layered'", "'layered_dag'", "'dag'");
                    }
                }
            }
        } else if (send.fun == core::Names::layer()) {
            bool seenLayerDecl = this->foundLayerDeclaration;
            this->foundLayerDeclaration = true;

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

            core::NameRef parsedValue;
            if (send.numPosArgs() > 0) {
                parsedValue = parseLayerOption(ctx.state, send.getPosArg(0));
            }

            // We explicitly disallow duplicate `layer` declarations. Additionally, if we're processing a __package.rb
            // file in an already populated PackageDB, we require that the declaration matches the value that's already
            // present.
            bool valueMatches = !info.locs.layer.exists() || parsedValue == info.layer;
            if (seenLayerDecl || !valueMatches) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::DuplicateDirective)) {
                    e.setHeader("Repeated declaration of `{}`", send.fun.show(ctx));
                    e.addErrorLine(ctx.locAt(info.locs.layer), "Previously declared here");
                    e.replaceWith("Remove this declaration", ctx.locAt(send.loc), "");
                }
                return;
            }

            if (send.numPosArgs() > 0) {
                if (parsedValue.exists()) {
                    info.layer = parsedValue;
                    info.locs.layer = send.getPosArg(0).loc();
                } else {
                    if (auto e = ctx.beginError(send.argsLoc(), core::errors::Packager::InvalidLayer)) {
                        e.setHeader("Argument to `{}` must be one of: {}", send.fun.show(ctx),
                                    buildValidLayersStr(ctx.state));
                    }
                }
            }
        } else if (send.fun == core::Names::sorbet()) {
            bool seenSorbetDecl = this->foundSorbetDeclaration;
            this->foundSorbetDeclaration = true;

            core::StrictLevel minTypedLevel = core::StrictLevel::None;
            core::LocOffsets minTypedLevelLoc;

            if (send.numKwArgs() >= 1) {
                for (const auto [key, value] : send.kwArgPairs()) {
                    auto keyLit = ast::cast_tree<ast::Literal>(key);
                    ENFORCE(keyLit);

                    if (keyLit->asSymbol() != core::Names::minTypedLevel()) {
                        // `tests_min_typed_level` is ignored, and infer reports any other keyword
                        continue;
                    }

                    auto typedLevel = parseTypedLevelOption(ctx, value);
                    if (typedLevel == core::StrictLevel::None) {
                        if (auto e = ctx.beginError(send.argsLoc(), core::errors::Packager::InvalidMinTypedLevel)) {
                            e.setHeader("Argument to `{}` must be one of: `{}`, `{}`, `{}`, `{}`, or `{}`",
                                        keyLit->asSymbol().show(ctx), "ignore", "false", "true", "strict", "strong");
                        }
                        continue;
                    }

                    minTypedLevel = typedLevel;
                    minTypedLevelLoc = value.loc();
                }
            }

            // We explicitly disallow duplicate `sorbet` declarations. Additionally, if we're processing a __package.rb
            // file in an already populated PackageDB, we require that the declaration matches the values that are
            // already present.
            bool valueMatches = !info.locs.minTypedLevel.exists() || minTypedLevel == info.minTypedLevel;
            if (seenSorbetDecl || !valueMatches) {
                if (auto e = ctx.beginError(send.loc, core::errors::Packager::DuplicateDirective)) {
                    e.setHeader("Repeated declaration of `{}`", send.fun.show(ctx));
                    e.addErrorLine(ctx.locAt(info.locs.minTypedLevel), "Previously declared here");
                    e.replaceWith("Remove this declaration", ctx.locAt(send.loc), "");
                }
                return;
            }

            if (send.numKwArgs() >= 1) {
                info.minTypedLevel = minTypedLevel;
                info.locs.minTypedLevel = minTypedLevelLoc;
            }
        } else if (send.fun == core::Names::test_bang()) {
            // Handled as a class-level modifier in namer.
        } else {
            // Extra directives
            if (send.fun != core::Names::star()) {
                info.extraDirectives_.push_back(send.loc);
            }
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

    bool isSpecMethod(const sorbet::ast::Send &send) const {
        switch (send.fun.rawId()) {
            case core::Names::import().rawId():
            case core::Names::export_().rawId():
            case core::Names::visibleTo().rawId():
            case core::Names::exportAll().rawId():
            case core::Names::prelude_bang().rawId():
            case core::Names::test_bang().rawId():
                return true;

            default:
                return false;
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
    StrictDependenciesLevel parseStrictDependenciesOption(ast::ExpressionPtr &arg) {
        auto lit = ast::cast_tree<ast::Literal>(arg);
        if (!lit || !lit->isString()) {
            return StrictDependenciesLevel::None;
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

        return StrictDependenciesLevel::None;
    }

    core::NameRef parseLayerOption(const core::GlobalState &gs, ast::ExpressionPtr &arg) {
        auto validLayers = gs.packageDB().layers();
        auto lit = ast::cast_tree<ast::Literal>(arg);
        if (!lit || !lit->isString()) {
            return core::NameRef::noName();
        }
        auto value = lit->asString();
        if (absl::c_find(validLayers, value) != validLayers.end()) {
            return value;
        }
        return core::NameRef::noName();
    }

    core::StrictLevel parseTypedLevelOption(const core::GlobalState &gs, ast::ExpressionPtr &arg) {
        auto lit = ast::cast_tree<ast::Literal>(arg);
        if (!lit || !lit->isString()) {
            return core::StrictLevel::None;
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
                return core::StrictLevel::None;
        }
    }
};

void rewritePackageSpec(const core::GlobalState &gs, ast::ParsedFile &package, PackageInfo &info) {
    // The body walk appends to these lists. This might not be the first time we've seen the package (e.g., when a
    // slow path copies a prefix of the symbol table, it also copies the populated package DB), so clear them first.
    info.importedPackageNames.clear();
    info.exports_.clear();
    info.visibleTo_.clear();
    info.extraDirectives_.clear();

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
}

void validateLayering(core::Context ctx, const Import &i) {
    const auto &packageDB = ctx.state.packageDB();
    ENFORCE(packageDB.getPackageInfo(i.mangledName).exists())
    ENFORCE(packageDB.getPackageNameForFile(ctx.file).exists())
    auto &thisPkg = packageDB.getPackageInfo(packageDB.getPackageNameForFile(ctx.file));
    auto &otherPkg = packageDB.getPackageInfo(i.mangledName);
    ENFORCE(thisPkg.sccID().has_value(), "computeSCCs should already have been called and set sccID");
    ENFORCE(otherPkg.sccID().has_value(), "computeSCCs should already have been called and set sccID");

    if (thisPkg.strictDependenciesLevel == StrictDependenciesLevel::None ||
        otherPkg.strictDependenciesLevel == StrictDependenciesLevel::None || !thisPkg.layer.exists() ||
        !otherPkg.layer.exists()) {
        return;
    }

    if (thisPkg.strictDependenciesLevel == StrictDependenciesLevel::False) {
        return;
    }

    auto pkgLayer = thisPkg.layer;
    auto otherPkgLayer = otherPkg.layer;

    if (thisPkg.causesLayeringViolation(packageDB, otherPkgLayer)) {
        if (auto e = ctx.beginError(i.loc, core::errors::Packager::LayeringViolation)) {
            e.setHeader("Layering violation: cannot import `{}` (in layer `{}`) from `{}` (in layer `{}`)",
                        otherPkg.show(ctx), otherPkgLayer.show(ctx), thisPkg.show(ctx), pkgLayer.show(ctx));
            e.addErrorLine(core::Loc(thisPkg.file, thisPkg.locs.layer), "`{}`'s `{}` declared here", thisPkg.show(ctx),
                           "layer");
            e.addErrorLine(core::Loc(otherPkg.file, otherPkg.locs.layer), "`{}`'s `{}` declared here",
                           otherPkg.show(ctx), "layer");
            // TODO(neil): if the import is unused (ie. there are no references in this package to the imported
            // package), autocorrect to delete import
        }
    }

    StrictDependenciesLevel otherPkgExpectedLevel = thisPkg.minimumStrictDependenciesLevel();

    if (otherPkg.strictDependenciesLevel < otherPkgExpectedLevel) {
        if (auto e = ctx.beginError(i.loc, core::errors::Packager::StrictDependenciesViolation)) {
            e.setHeader("Strict dependencies violation: All of `{}`'s `{}`s must be `{}` or higher", thisPkg.show(ctx),
                        "import", strictDependenciesLevelToString(otherPkgExpectedLevel));
            e.addErrorLine(core::Loc(otherPkg.file, otherPkg.locs.strictDependenciesLevel),
                           "`{}`'s `{}` level declared here", otherPkg.show(ctx), "strict_dependencies");
            // TODO(neil): if the import is unused (ie. there are no references in this package to the imported
            // package), autocorrect to delete import
            // TODO(neil): if the imported package can be trivially upgraded to the required level (ex. it's at 'false'
            // but has no layering violations), autocorrect to do so
        }
    }

    if (thisPkg.strictDependenciesLevel >= StrictDependenciesLevel::LayeredDag) {
        if (thisPkg.sccID() == otherPkg.sccID()) {
            if (auto e = ctx.beginError(i.loc, core::errors::Packager::StrictDependenciesViolation)) {
                auto level = fmt::format("strict_dependencies '{}'",
                                         strictDependenciesLevelToString(thisPkg.strictDependenciesLevel));
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
    auto &otherPkg = ctx.state.packageDB().getPackageInfo(i.mangledName);
    ENFORCE(otherPkg.exists());

    // We should only report a visibility error if we're not going to add a visible_to to the package
    // Otherwise the error is pointless since it'll go away after the new visible_to is added
    if (!otherPkg.isVisibleTo(ctx, absPkg) && !ctx.state.packageDB().updateVisibilityFor(i.mangledName)) {
        if (auto e = ctx.beginError(i.loc, core::errors::Packager::ImportNotVisible)) {
            e.setHeader("Package `{}` includes explicit visibility modifiers and cannot be imported from `{}`",
                        otherPkg.show(ctx), absPkg.show(ctx));
            e.addErrorNote("Please consult with the owning team before adding a `{}` line to the package `{}`",
                           "visible_to", otherPkg.show(ctx));
        }
    }
}

// We don't have a way (yet?) to allow using packages to control access to things defined in the payload.
//
// It doesn't work today, because all the payload symbols have nothing for their `package` and
// `packageRegistryOwner` fields (because those symbols were entered before any package files were available)
//
// It would be nice to change this, but for now, let's be explicit about telling the user this is unsupported.
void validatePackageNamespace(core::Context ctx, const PackageInfo &pkgInfo) {
    auto registryRoot = pkgInfo.mangledName().owner;
    while (registryRoot.data(ctx)->owner != core::Symbols::PackageSpecRegistry()) {
        registryRoot = registryRoot.data(ctx)->owner;
    }

    // It's sufficient to just check the top-most thing (vs having to accumulate a vector of names,
    // and then walk back down from `root` in the non `<PackageSpecRegistry>` hierarchy): everything
    // nested inside a payload constant is also from the payload, and nothing defined outside the
    // payload can make a payload constant appear beneath it.
    auto existing = core::Symbols::root().data(ctx)->findMemberNoDealias(registryRoot.data(ctx)->name);
    if (!existing.exists()) {
        return;
    }

    auto locs = existing.locs(ctx);
    auto payloadLoc = absl::c_find_if(locs, [&](auto loc) { return loc.exists() && loc.file().data(ctx).isPayload(); });
    if (payloadLoc == locs.end()) {
        return;
    }

    if (auto e = ctx.beginError(pkgInfo.locs.declLoc, core::errors::Packager::InvalidPackageDefinition)) {
        e.setHeader("Package `{}` cannot be defined because `{}` is defined in Sorbet's payload", pkgInfo.show(ctx),
                    existing.show(ctx));
        e.addErrorLine(*payloadLoc, "Defined here");
        e.addErrorNote("Constants from Sorbet's payload do not belong to any package, so this package's imports, "
                       "exports, and `{}` declarations would have no effect on them",
                       "visible_to");
    }
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

    auto &pkgInfo = packageDB.getPackageInfo(absPkg);
    validatePackageNamespace(ctx, pkgInfo);

    bool skipImportVisibilityCheck = packageDB.allowRelaxedPackagerChecksFor(pkgInfo.mangledName());
    auto enforceLayering = ctx.state.packageDB().enforceLayering();

    if (skipImportVisibilityCheck && !enforceLayering) {
        return;
    }

    for (auto &i : pkgInfo.importedPackageNames) {
        auto &otherPkg = packageDB.getPackageInfo(i.mangledName);

        // this might mean the other package doesn't exist, but that should have been caught already
        if (!otherPkg.exists()) {
            continue;
        }

        // It's not acceptable to import a `test!` package from application code
        if (otherPkg.testPackage() && !pkgInfo.testPackage()) {
            if (auto e = ctx.beginError(i.loc, core::errors::Packager::TestImportMismatch)) {
                e.setHeader("Package `{}` may not import `{}` packages", pkgInfo.show(ctx), "test!");
                e.addErrorLine(pkgInfo.declLoc(), "Defined here");
                e.addErrorLine(otherPkg.declLoc(), "Imported `{}` package defined here", "test!");
            }
        }

        // `uses_internals: true` is only valid from a `test!` package
        if (i.usesInternals && !pkgInfo.testPackage()) {
            if (auto e = ctx.beginError(i.loc, core::errors::Packager::TestImportMismatch)) {
                e.setHeader("Non-`{}` package `{}` cannot have imports with `{}`", "test!", pkgInfo.show(ctx),
                            "uses_internals: true");
                e.addErrorLine(pkgInfo.declLoc(), "Defined here");
            }
        }

        if (enforceLayering) {
            validateLayering(ctx, i);
        }

        if (!skipImportVisibilityCheck) {
            validateVisibility(ctx, pkgInfo, i);
        }
    }
}

void validatePackagedFile(core::Context ctx, const ast::ExpressionPtr &tree) {
    auto &file = ctx.file.data(ctx);
    ENFORCE(!file.isPackage(ctx));
    // Files in Sorbet's payload belong to no package, but they're only ever indexed outside of the packager: when
    // building the payload (`--store-state`, which `--sorbet-packages` rejects). LSP doesn't retypecheck them.
    ENFORCE(!file.isPayload(), "Payload file `{}` in packager mode", file.path());

    if (file.sourceSize() == 0) {
        // Either this is an empty file, or it's a file that was deleted. In both cases, we don't need to validate it.
        // TODO(neil): we might want to move this logic to skip files like this somewhere higher up.
        return;
    }

    auto pkg = ctx.state.packageDB().getPackageNameForFile(ctx.file);
    if (!pkg.exists()) {
        return;
    }

    auto &pkgImpl = ctx.state.packageDB().getPackageInfo(pkg);

    // NOTE: the loc where we report the error may be wrong, the first line could be another sigil like
    // frozen_string_literal Unfortunately, we don't store the loc of the typed sigil, so this is the best we can do
    if (pkgImpl.locs.minTypedLevel.exists() && file.originalSigil < pkgImpl.minTypedLevel) {
        if (auto e = ctx.beginError(core::LocOffsets{0, 0}, core::errors::Packager::TypedSigilTooLow)) {
            e.setHeader("All files in package `{}` must be at least `{}{}`", pkg.owner.show(ctx),
                        "typed: ", core::SigilTraits<core::StrictLevel>::toString(pkgImpl.minTypedLevel));
            e.addErrorLine(core::Loc(pkgImpl.file, pkgImpl.locs.minTypedLevel),
                           "Minimum typed level for files in this package declared here");
        }
    }
    // TODO(neil): Autocorrect to update the sigil?

    EnforcePackageBehavior enforceBehavior(ctx, pkgImpl);
    ast::ConstShallowWalk::apply(ctx, enforceBehavior, tree);
}

} // namespace

namespace {

class PackageDBPackageGraph {
    PackageDB &packageDB;

public:
    PackageDBPackageGraph(PackageDB &packageDB) : packageDB(packageDB) {}

    absl::Span<const Import> getImports(MangledName packageName) const {
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
};

} // namespace

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
                PackageInfo copy{info.mangledName_, info.file, info.locs.loc, info.locs.declLoc};
                copy.locs.exportAll = info.locs.exportAll;
                copy.locs.preludePackage = info.locs.preludePackage;
                copy.locs.testPackage = info.locs.testPackage;
                rewritePackageSpec(gs, file, copy);
            }
            validatePackage(ctx);
        } else {
            validatePackagedFile(ctx, file.tree);
        }
    });

    return files;
}

void Packager::buildPackageDB(core::GlobalState &gs, WorkerPool &workers, absl::Span<ast::ParsedFile> packageFiles,
                              absl::Span<core::FileRef> nonPackageFiles) {
    Timer timeit(gs.tracer(), "packager");
    timeit.setTag("mode", "packages_only");

    {
        Timer timeit(gs.tracer(), "packager.findPackages");

        gs.packageDB().resolvePackagesWithRelaxedChecks(gs);
        gs.packageDB().resolvePackagesToUpdateVisibilityFor(gs);

        {
            core::UnfreezeNameTable unfreeze(gs);
            auto packages = gs.unfreezePackages();

            for (auto &file : packageFiles) {
                if (!file.file.data(gs).isPackage(gs)) {
                    continue;
                }

                auto pkgName = gs.packageDB().getPackageNameForFile(file.file);
                if (!pkgName.exists()) {
                    continue;
                }
                auto &info = *gs.packageDB().getPackageInfoNonConst(pkgName);
                rewritePackageSpec(gs, file, info);

                auto parentPkgName = gs.packageDB().getParentPackage(gs, pkgName);
                if (parentPkgName.exists()) {
                    auto &info = *gs.packageDB().getPackageInfoNonConst(parentPkgName);

                    // As we're iterating over all packages in this pass, it's sufficient to mark only the parent as
                    // having subpackages.
                    info.hasSubPackages = true;
                }
            }
        }

        PackageDBPackageGraph packageGraph{gs.packageDB()};
        gs.packageDB().setCondensation(ComputePackageSCCs::run(gs, packageGraph));
    }

    {
        Timer timeit(gs.tracer(), "packager.validatePackages");
        Parallel::iterate(workers, "validatePackages", absl::MakeSpan(packageFiles), [&gs = as_const(gs)](auto &job) {
            core::Context ctx(gs, core::Symbols::root(), job.file);
            ENFORCE(job.file.data(gs).isPackage(gs));
            validatePackage(ctx);
        });
    }

    for (auto fref : nonPackageFiles) {
        auto pkg = gs.packageDB().getPackageNameForFile(fref);
        if (pkg.exists()) {
            continue;
        }

        pkg = gs.packageDB().findPackageByPath(gs, fref);
        if (!pkg.exists()) {
            continue;
        }

        gs.packageDB().setPackageNameForFile(fref, pkg);
    }
}

void Packager::validatePackagedFiles(const core::GlobalState &gs, WorkerPool &workers,
                                     absl::Span<ast::ParsedFile> files) {
    Timer timeit(gs.tracer(), "packager");
    timeit.setTag("mode", "packaged_files_only");

    Parallel::iterate(workers, "validatePackagesAndFiles", absl::MakeSpan(files), [&gs](auto &job) {
        core::Context ctx(gs, core::Symbols::root(), job.file);
        ENFORCE(!job.file.data(gs).isPackage(gs));
        validatePackagedFile(ctx, job.tree);
    });
}

} // namespace sorbet::packager
