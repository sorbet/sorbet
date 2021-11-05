#include "core/errors/resolver.h"
#include "ast/Helpers.h"
#include "ast/Trees.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/sort.h"
#include "core/Error.h"
#include "core/Names.h"
#include "core/StrictLevel.h"
#include "core/core.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "resolver/CorrectTypeAlias.h"
#include "resolver/resolver.h"
#include "resolver/type_syntax.h"

#include "absl/algorithm/container.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "common/Timer.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "core/Symbols.h"
#include <utility>
#include <vector>

using namespace std;

namespace sorbet::resolver {
namespace {

/*
 * Note: There are multiple separate tree walks defined in this file, the main
 * ones being:
 *
 * - ResolveConstantsWalk
 * - ResolveSignaturesWalk
 *
 * There are also other important parts of resolver found elsewhere in the
 * resolver/ package (GlobalPass, type_syntax). Below we describe
 * ResolveConstantsWalk, which is particularly sophisticated.
 *
 *                                - - - - -
 *
 * Ruby supports resolving constants via ancestors--superclasses and mixins.
 * Since superclass and mixins are themselves constant references, we thus may
 * not be able to resolve certain constants until after we've resolved others.
 *
 * To solve this, we collect any failed resolutions in a number of TODO lists,
 * and iterate over them to a fixed point (namely, either all constants
 * resolve, or no new constants resolve and we stub out any that remain).
 * In practice this fixed point computation terminates after 3 or fewer passes
 * on most real codebases.
 *
 * The four TODO lists that this loop maintains are:
 *
 *  - constants to be resolved
 *  - ancestors to be filled that require constants to be resolved
 *  - class aliases (class aliases know the symbol they alias to)
 *  - type aliases (type aliases know the fully parsed type of their RHS, and
 *    thus require their RHS to be resolved)
 *
 * Successful resolutions are removed from the lists, and then we loop again.
 * We track all these lists separately for the dual reasons that
 *
 * 1. Upon successful resolution, we need to do additional work (mutating the
 *    symbol table to reflect the new ancestors) and
 * 2. Resolving those constants potentially renders additional constants
 *    resolvable, and so if any resolution succeeds, we need to keep looping in
 *    the outer loop.
 *
 * We also track failure in the item, in order to distinguish a thing left in the
 * list because we simply haven't succeeded yet and a thing left in the list
 * because we have actively found a failure. For example, we might know that a
 * given constant is unresolvable by Sorbet because it was qualified under
 * not-a-constant: we mark this kind of job `resolution_failed`. The reason for
 * this is unresolved constants are set to noSymbol, and once constant resolution
 * has truly finished, we want to know which remaining failed jobs we need to set
 * to a sensible default value. (Setting a conceptually failed job to untyped()
 * before we've completed this loop can occasionally cause other jobs to
 * non-deterministically half-resolve in the presence of multiple errors---c.f.
 * issue #1126 on Github---so we mark jobs as failed rather than reporting an
 * error and "resolving" them as untyped during the loop.)
 *
 * After the above passes:
 *
 * - ast::UnresolvedConstantLit nodes (constants that have a NameRef) are
 *   replaced with ast::ConstantLit nodes (constants that have a SymbolRef).
 * - Every constant SymbolRef has enough to completely understand it's own
 *   place in the ancestor hierarchy.
 * - Every type alias symbol carries with it the type it should be treated as.
 *
 * The resolveConstants method is the best place to start if you want to browse
 * the fixed point loop at a high level.
 */

class ResolveConstantsWalk {
    friend class ResolveSanityCheckWalk;

private:
    struct Nesting {
        const shared_ptr<Nesting> parent;
        const core::SymbolRef scope;

        Nesting(shared_ptr<Nesting> parent, core::SymbolRef scope) : parent(std::move(parent)), scope(scope) {}
    };
    shared_ptr<Nesting> nesting_;

    struct ResolutionItem {
        shared_ptr<Nesting> scope;
        core::FileRef file;
        bool resolutionFailed = false;
        ast::ConstantLit *out;

        ResolutionItem() = default;
        ResolutionItem(const shared_ptr<Nesting> &scope, core::FileRef file, ast::ConstantLit *lit)
            : scope(scope), file(file), out(lit) {}
        ResolutionItem(ResolutionItem &&rhs) noexcept = default;
        ResolutionItem &operator=(ResolutionItem &&rhs) noexcept = default;

        ResolutionItem(const ResolutionItem &rhs) = delete;
        const ResolutionItem &operator=(const ResolutionItem &rhs) = delete;
    };
    struct AncestorResolutionItem {
        ast::ConstantLit *ancestor;
        core::ClassOrModuleRef klass;
        core::FileRef file;

        bool isSuperclass; // true if superclass, false for mixin
        bool isInclude;    // true if include, false if extend

        AncestorResolutionItem() = default;
        AncestorResolutionItem(AncestorResolutionItem &&rhs) noexcept = default;
        AncestorResolutionItem &operator=(AncestorResolutionItem &&rhs) noexcept = default;

        AncestorResolutionItem(const AncestorResolutionItem &rhs) = delete;
        const AncestorResolutionItem &operator=(const AncestorResolutionItem &rhs) = delete;
    };

    struct ClassAliasResolutionItem {
        core::SymbolRef lhs;
        core::FileRef file;
        ast::ConstantLit *rhs;

        ClassAliasResolutionItem() = default;
        ClassAliasResolutionItem(ClassAliasResolutionItem &&) noexcept = default;
        ClassAliasResolutionItem &operator=(ClassAliasResolutionItem &&rhs) noexcept = default;

        ClassAliasResolutionItem(const ClassAliasResolutionItem &) = delete;
        const ClassAliasResolutionItem &operator=(const ClassAliasResolutionItem &) = delete;
    };

    struct TypeAliasResolutionItem {
        core::SymbolRef lhs;
        core::FileRef file;
        ast::ExpressionPtr *rhs;

        TypeAliasResolutionItem(TypeAliasResolutionItem &&) noexcept = default;
        TypeAliasResolutionItem &operator=(TypeAliasResolutionItem &&rhs) noexcept = default;

        TypeAliasResolutionItem(const TypeAliasResolutionItem &) = delete;
        const TypeAliasResolutionItem &operator=(const TypeAliasResolutionItem &) = delete;
    };

    struct ClassMethodsResolutionItem {
        core::FileRef file;
        core::SymbolRef owner;
        ast::Send *send;

        ClassMethodsResolutionItem(ClassMethodsResolutionItem &&) noexcept = default;
        ClassMethodsResolutionItem &operator=(ClassMethodsResolutionItem &&rhs) noexcept = default;

        ClassMethodsResolutionItem(const ClassMethodsResolutionItem &) = delete;
        const ClassMethodsResolutionItem &operator=(const ClassMethodsResolutionItem &) = delete;
    };

    struct RequireAncestorResolutionItem {
        core::FileRef file;
        core::SymbolRef owner;
        ast::Send *send;

        RequireAncestorResolutionItem(RequireAncestorResolutionItem &&) noexcept = default;
        RequireAncestorResolutionItem &operator=(RequireAncestorResolutionItem &&rhs) noexcept = default;

        RequireAncestorResolutionItem(const RequireAncestorResolutionItem &) = delete;
        const RequireAncestorResolutionItem &operator=(const RequireAncestorResolutionItem &) = delete;
    };

    vector<ResolutionItem> todo_;
    vector<AncestorResolutionItem> todoAncestors_;
    vector<ClassAliasResolutionItem> todoClassAliases_;
    vector<TypeAliasResolutionItem> todoTypeAliases_;
    vector<ClassMethodsResolutionItem> todoClassMethods_;
    vector<RequireAncestorResolutionItem> todoRequiredAncestors_;

    static core::SymbolRef resolveLhs(core::Context ctx, const shared_ptr<Nesting> &nesting, core::NameRef name) {
        Nesting *scope = nesting.get();
        while (scope != nullptr) {
            auto lookup = scope->scope.data(ctx)->findMember(ctx, name);
            if (lookup.exists()) {
                return lookup;
            }
            scope = scope->parent.get();
        }
        return nesting->scope.data(ctx)->findMemberTransitive(ctx, name);
    }

    static bool isAlreadyResolved(core::Context ctx, const ast::ConstantLit &original) {
        auto sym = original.symbol;
        if (!sym.exists()) {
            return false;
        }
        auto data = sym.data(ctx);
        if (data->isTypeAlias()) {
            return data->resultType != nullptr;
        } else {
            return true;
        }
    }

    class ResolutionChecker {
    public:
        bool seenUnresolved = false;

        ast::ExpressionPtr postTransformConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
            auto &original = ast::cast_tree_nonnull<ast::ConstantLit>(tree);
            seenUnresolved |= !isAlreadyResolved(ctx, original);
            return tree;
        };
    };

    static bool isFullyResolved(core::Context ctx, const ast::ExpressionPtr &expression) {
        ResolutionChecker checker;
        ast::ExpressionPtr dummy(expression.getTagged());
        dummy = ast::TreeMap::apply(ctx, checker, std::move(dummy));
        ENFORCE(dummy == expression);
        dummy.release();
        return !checker.seenUnresolved;
    }

    static core::SymbolRef resolveConstant(core::Context ctx, const shared_ptr<Nesting> &nesting,
                                           const ast::UnresolvedConstantLit &c, bool &resolutionFailed) {
        if (ast::isa_tree<ast::EmptyTree>(c.scope)) {
            core::SymbolRef result = resolveLhs(ctx, nesting, c.cnst);
            return result;
        }
        if (auto *id = ast::cast_tree<ast::ConstantLit>(c.scope)) {
            auto sym = id->symbol;
            if (sym.exists() && sym.data(ctx)->isTypeAlias() && !resolutionFailed) {
                if (auto e = ctx.beginError(c.loc, core::errors::Resolver::ConstantInTypeAlias)) {
                    e.setHeader("Resolving constants through type aliases is not supported");
                }
                resolutionFailed = true;
                return core::Symbols::noSymbol();
            }
            if (!sym.exists()) {
                return core::Symbols::noSymbol();
            }
            core::SymbolRef resolved = id->symbol.data(ctx)->dealias(ctx);
            core::SymbolRef result = resolved.data(ctx)->findMember(ctx, c.cnst);

            // Private constants are allowed to be resolved, when there is no scope set (the scope is checked above),
            // otherwise we should error out. Private constant references _are not_ enforced inside RBI files.
            if (result.exists() &&
                ((result.isClassOrModule() && result.data(ctx)->isClassOrModulePrivate()) ||
                 (result.isStaticField(ctx) && result.data(ctx)->isStaticFieldPrivate())) &&
                !ctx.file.data(ctx).isRBI()) {
                if (auto e = ctx.beginError(c.loc, core::errors::Resolver::PrivateConstantReferenced)) {
                    e.setHeader("Non-private reference to private constant `{}` referenced", result.show(ctx));
                }
            }
            return result;
        } else {
            if (!resolutionFailed) {
                if (auto e = ctx.beginError(c.loc, core::errors::Resolver::DynamicConstant)) {
                    e.setHeader("Dynamic constant references are unsupported");
                }
            }
            resolutionFailed = true;
            return core::Symbols::noSymbol();
        }
    }

    // We have failed to resolve the constant. We'll need to report the error and stub it so that we can proceed
    static void constantResolutionFailed(core::MutableContext ctx, ResolutionItem &job, bool suggestDidYouMean) {
        auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(job.out->original);

        auto resolved = resolveConstant(ctx.withOwner(job.scope->scope), job.scope, original, job.resolutionFailed);
        if (resolved.exists() && resolved.data(ctx)->isTypeAlias()) {
            if (resolved.data(ctx)->resultType == nullptr) {
                // This is actually a use-site error, but we limit ourselves to emitting it once by checking resultType
                auto loc = resolved.data(ctx)->loc();
                if (auto e = ctx.state.beginError(loc, core::errors::Resolver::RecursiveTypeAlias)) {
                    e.setHeader("Unable to resolve right hand side of type alias `{}`", resolved.show(ctx));
                    e.addErrorLine(core::Loc(ctx.file, job.out->original.loc()), "Type alias used here");
                }
                resolved.data(ctx)->resultType =
                    core::Types::untyped(ctx, resolved); // <<-- This is the reason this takes a MutableContext
            }
            job.out->symbol = resolved;
            return;
        }
        if (job.resolutionFailed) {
            // we only set this when a job has failed for other reasons and we've already reported an error, and
            // continuining on will only redundantly report that we can't resolve the constant, so bail early here
            job.out->symbol = core::Symbols::untyped();
            return;
        }
        ENFORCE(!resolved.exists());
        ENFORCE(!job.out->symbol.exists());

        job.out->symbol = core::Symbols::StubModule();

        bool alreadyReported = false;
        if (auto *id = ast::cast_tree<ast::ConstantLit>(original.scope)) {
            auto originalScope = id->symbol.data(ctx)->dealias(ctx);
            if (originalScope == core::Symbols::StubModule()) {
                // If we were trying to resolve some literal like C::D but `C` itself was already stubbed,
                // no need to also report that `D` is missing.
                alreadyReported = true;

                job.out->resolutionScopes = {core::Symbols::noSymbol()};
            } else {
                // We were trying to resolve a constant literal that had an explicit scope.
                // Since Sorbet doesn't combine ancestor resolution and explicit scope resolution,
                // we just put a single entry in the resolutionScopes list.
                job.out->resolutionScopes = {originalScope};
            }
        } else {
            auto nesting = job.scope;
            while (true) {
                job.out->resolutionScopes.emplace_back(nesting->scope);
                if (nesting->parent == nullptr) {
                    break;
                }

                nesting = nesting->parent;
            }
        }

        ENFORCE(!job.out->resolutionScopes.empty());
        ENFORCE(job.scope->scope != core::Symbols::StubModule());

        auto customAutogenError = original.cnst == core::Symbols::Subclasses().data(ctx)->name;
        if (!alreadyReported || customAutogenError) {
            if (auto e = ctx.beginError(job.out->original.loc(), core::errors::Resolver::StubConstant)) {
                e.setHeader("Unable to resolve constant `{}`", original.cnst.show(ctx));

                auto suggestScope = job.out->resolutionScopes.front();
                if (customAutogenError) {
                    e.addErrorNote("If this constant is generated by Autogen, you "
                                   "may need to re-generate the .rbi. Try running:\n"
                                   "  scripts/bin/remote-script sorbet/shim_generation/autogen.rb");
                } else if (suggestDidYouMean && suggestScope.exists() && suggestScope.isClassOrModule()) {
                    auto suggested = suggestScope.data(ctx)->findMemberFuzzyMatch(ctx, original.cnst);
                    if (suggested.size() > 3) {
                        suggested.resize(3);
                    }
                    if (!suggested.empty()) {
                        vector<core::ErrorLine> lines;
                        for (auto suggestion : suggested) {
                            const auto replacement = suggestion.symbol.show(ctx);
                            lines.emplace_back(core::ErrorLine::from(suggestion.symbol.data(ctx)->loc(),
                                                                     "Did you mean: `{}`?", replacement));
                            e.replaceWith(fmt::format("Replace with `{}`", replacement),
                                          core::Loc(ctx.file, job.out->loc), "{}", replacement);
                        }
                        e.addErrorSection(core::ErrorSection(lines));
                    }
                }
            }
        }
    }

    static bool resolveJob(core::Context ctx, ResolutionItem &job) {
        if (isAlreadyResolved(ctx, *job.out)) {
            return true;
        }
        auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(job.out->original);
        auto resolved = resolveConstant(ctx.withOwner(job.scope->scope), job.scope, original, job.resolutionFailed);
        if (!resolved.exists()) {
            return false;
        }
        if (resolved.data(ctx)->isTypeAlias()) {
            if (resolved.data(ctx)->resultType != nullptr) {
                job.out->symbol = resolved;
                return true;
            }
            return false;
        }

        job.out->symbol = resolved;
        return true;
    }

    static bool resolveTypeAliasJob(core::MutableContext ctx, TypeAliasResolutionItem &job) {
        core::TypeMemberRef enclosingTypeMember;
        core::ClassOrModuleRef enclosingClass = job.lhs.enclosingClass(ctx);
        while (enclosingClass != core::Symbols::root()) {
            auto typeMembers = enclosingClass.data(ctx)->typeMembers();
            if (!typeMembers.empty()) {
                enclosingTypeMember = typeMembers[0].asTypeMemberRef();
                break;
            }
            enclosingClass = enclosingClass.data(ctx)->owner.enclosingClass(ctx);
        }
        auto &rhs = *job.rhs;
        if (enclosingTypeMember.exists()) {
            if (auto e = ctx.beginError(rhs.loc(), core::errors::Resolver::TypeAliasInGenericClass)) {
                e.setHeader("Type aliases are not allowed in generic classes");
                e.addErrorLine(enclosingTypeMember.data(ctx)->loc(), "Here is enclosing generic member");
            }
            job.lhs.data(ctx)->resultType = core::Types::untyped(ctx, job.lhs);
            return true;
        }
        if (isFullyResolved(ctx, rhs)) {
            // this todo will be resolved during ResolveTypeMembersAndFieldsWalk below
            job.lhs.data(ctx)->resultType = core::make_type<core::ClassType>(core::Symbols::todo());
            return true;
        }

        return false;
    }

    static bool resolveClassAliasJob(core::MutableContext ctx, ClassAliasResolutionItem &it) {
        auto rhsSym = it.rhs->symbol;
        if (!rhsSym.exists()) {
            return false;
        }

        auto rhsData = rhsSym.data(ctx);
        if (rhsData->isTypeAlias()) {
            if (auto e = ctx.beginError(it.rhs->loc, core::errors::Resolver::ReassignsTypeAlias)) {
                e.setHeader("Reassigning a type alias is not allowed");
                e.addErrorLine(rhsData->loc(), "Originally defined here");
                auto rhsLoc = core::Loc{ctx.file, it.rhs->loc};
                if (rhsLoc.exists()) {
                    e.replaceWith("Declare as type alias", rhsLoc, "T.type_alias {{{}}}", rhsLoc.source(ctx).value());
                }
            }
            it.lhs.data(ctx)->resultType = core::Types::untypedUntracked();
            return true;
        } else {
            if (rhsData->dealias(ctx) != it.lhs) {
                it.lhs.data(ctx)->resultType = core::make_type<core::AliasType>(rhsSym);
            } else {
                if (auto e =
                        ctx.state.beginError(it.lhs.data(ctx)->loc(), core::errors::Resolver::RecursiveClassAlias)) {
                    e.setHeader("Class alias aliases to itself");
                }
                it.lhs.data(ctx)->resultType = core::Types::untypedUntracked();
            }
            return true;
        }
    }

    static void saveAncestorTypeForHashing(core::MutableContext ctx, const AncestorResolutionItem &item) {
        // For LSP, create a synthetic method <unresolved-ancestors> that has a return type containing a type
        // for every ancestor. When this return type changes, LSP takes the slow path (see
        // Symbol::methodShapeHash()).
        auto unresolvedPath = item.ancestor->fullUnresolvedPath(ctx);
        if (!unresolvedPath.has_value()) {
            return;
        }

        auto ancestorType =
            core::make_type<core::UnresolvedClassType>(unresolvedPath->first, move(unresolvedPath->second));

        auto uaSym = ctx.state.enterMethodSymbol(core::Loc::none(), item.klass, core::Names::unresolvedAncestors());
        core::TypePtr resultType = uaSym.data(ctx)->resultType;
        if (!resultType) {
            uaSym.data(ctx)->resultType = core::make_type<core::TupleType>(vector<core::TypePtr>{ancestorType});
        } else if (auto tt = core::cast_type<core::TupleType>(resultType)) {
            tt->elems.push_back(ancestorType);
        } else {
            ENFORCE(false);
        }
    }

    static core::ClassOrModuleRef stubSymbolForAncestor(const AncestorResolutionItem &item) {
        if (item.isSuperclass) {
            return core::Symbols::StubSuperClass();
        } else {
            return core::Symbols::StubMixin();
        }
    }

    static bool resolveAncestorJob(core::MutableContext ctx, AncestorResolutionItem &job, bool lastRun) {
        auto ancestorSym = job.ancestor->symbol;
        if (!ancestorSym.exists()) {
            return false;
        }

        core::ClassOrModuleRef resolvedClass;
        {
            core::SymbolRef resolved;
            if (ancestorSym.data(ctx)->isTypeAlias()) {
                if (!lastRun) {
                    return false;
                }
                if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::DynamicSuperclass)) {
                    e.setHeader("Superclasses and mixins may not be type aliases");
                }
                resolved = stubSymbolForAncestor(job);
            } else {
                resolved = ancestorSym.data(ctx)->dealias(ctx);
            }

            if (!resolved.isClassOrModule()) {
                if (!lastRun) {
                    return false;
                }
                if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::DynamicSuperclass)) {
                    e.setHeader("Superclasses and mixins may only use class aliases like `{}`", "A = Integer");
                }
                resolved = stubSymbolForAncestor(job);
            }
            resolvedClass = resolved.asClassOrModuleRef();
        }

        if (resolvedClass == job.klass) {
            if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::CircularDependency)) {
                e.setHeader("Circular dependency: `{}` is a parent of itself", job.klass.show(ctx));
                e.addErrorLine(resolvedClass.data(ctx)->loc(), "Class definition");
            }
            resolvedClass = stubSymbolForAncestor(job);
        } else if (resolvedClass.data(ctx)->derivesFrom(ctx, job.klass)) {
            if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::CircularDependency)) {
                e.setHeader("Circular dependency: `{}` and `{}` are declared as parents of each other",
                            job.klass.show(ctx), resolvedClass.show(ctx));
                e.addErrorLine(job.klass.data(ctx)->loc(), "One definition");
                e.addErrorLine(resolvedClass.data(ctx)->loc(), "Other definition");
            }
            resolvedClass = stubSymbolForAncestor(job);
        }

        bool ancestorPresent = true;
        if (job.isSuperclass) {
            if (resolvedClass == core::Symbols::todo()) {
                // No superclass specified
                ancestorPresent = false;
            } else if (!job.klass.data(ctx)->superClass().exists() ||
                       job.klass.data(ctx)->superClass() == core::Symbols::todo() ||
                       job.klass.data(ctx)->superClass() == resolvedClass) {
                job.klass.data(ctx)->setSuperClass(resolvedClass);
            } else {
                if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::RedefinitionOfParents)) {
                    e.setHeader("Parent of class `{}` redefined from `{}` to `{}`", job.klass.show(ctx),
                                job.klass.data(ctx)->superClass().show(ctx), resolvedClass.show(ctx));
                }
            }
        } else {
            if (!job.klass.data(ctx)->addMixin(ctx, resolvedClass)) {
                if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::IncludesNonModule)) {
                    e.setHeader("Only modules can be `{}`d, but `{}` is a class", job.isInclude ? "include" : "extend",
                                resolvedClass.show(ctx));
                    e.addErrorLine(resolvedClass.data(ctx)->loc(), "`{}` defined as a class here",
                                   resolvedClass.show(ctx));
                }
            }
        }

        if (ancestorPresent) {
            saveAncestorTypeForHashing(ctx, job);
        }
        return true;
    }

    static void resolveClassMethodsJob(core::GlobalState &gs, const ClassMethodsResolutionItem &todo) {
        auto owner = todo.owner;
        auto send = todo.send;
        if (!owner.isClassOrModule() || !owner.data(gs)->isClassOrModuleModule()) {
            if (auto e =
                    gs.beginError(core::Loc(todo.file, send->loc), core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` can only be declared inside a module, not a class", send->fun.show(gs));
            }
            return;
        }

        if (send->args.size() < 1) {
            // The arity mismatch error will be emitted later by infer.
            return;
        }

        auto encounteredError = false;
        for (auto &arg : send->args) {
            if (arg.isSelfReference()) {
                auto recv = ast::cast_tree<ast::ConstantLit>(send->recv);
                if (recv != nullptr && recv->symbol == core::Symbols::Magic()) {
                    // This is the first argument of a Magic.mixes_in_class_methods() call
                    continue;
                }
            }

            auto *id = ast::cast_tree<ast::ConstantLit>(arg);

            if (id == nullptr || !id->symbol.exists()) {
                if (auto e = gs.beginError(core::Loc(todo.file, send->loc),
                                           core::errors::Resolver::InvalidMixinDeclaration)) {
                    e.setHeader("Argument to `{}` must be statically resolvable to a module", send->fun.show(gs));
                }
                continue;
            }
            if (!id->symbol.isClassOrModule()) {
                if (auto e =
                        gs.beginError(core::Loc(todo.file, id->loc), core::errors::Resolver::InvalidMixinDeclaration)) {
                    e.setHeader("Argument to `{}` must be statically resolvable to a module", send->fun.show(gs));
                }
                continue;
            }
            if (id->symbol.data(gs)->isClassOrModuleClass()) {
                if (auto e =
                        gs.beginError(core::Loc(todo.file, id->loc), core::errors::Resolver::InvalidMixinDeclaration)) {
                    e.setHeader("`{}` is a class, not a module; Only modules may be mixins", id->symbol.show(gs));
                }
                encounteredError = true;
            }
            if (id->symbol == owner) {
                if (auto e =
                        gs.beginError(core::Loc(todo.file, id->loc), core::errors::Resolver::InvalidMixinDeclaration)) {
                    e.setHeader("Must not pass your self to `{}`", send->fun.show(gs));
                }
                encounteredError = true;
            }
            if (encounteredError) {
                continue;
            }

            // Get the fake property holding the mixes
            auto mixMethod = owner.data(gs)->findMember(gs, core::Names::mixedInClassMethods());
            auto loc = core::Loc(owner.data(gs)->loc().file(), send->loc);
            if (!mixMethod.exists()) {
                // We never stored a mixin in this symbol
                // Create a the fake property that will hold the mixed in modules
                mixMethod = gs.enterMethodSymbol(loc, owner.asClassOrModuleRef(), core::Names::mixedInClassMethods());
                vector<core::TypePtr> targs;
                mixMethod.data(gs)->resultType = core::make_type<core::TupleType>(move(targs));

                // Create a dummy block argument to satisfy sanitycheck during GlobalState::expandNames
                auto &arg =
                    gs.enterMethodArgumentSymbol(core::Loc::none(), mixMethod.asMethodRef(), core::Names::blkArg());
                arg.flags.isBlock = true;
            }

            auto type = core::make_type<core::ClassType>(id->symbol.asClassOrModuleRef());
            auto &elems = (core::cast_type<core::TupleType>(mixMethod.data(gs)->resultType))->elems;
            // Make sure we are not adding existing symbols to our tuple
            if (absl::c_find(elems, type) == elems.end()) {
                elems.emplace_back(type);
            }
        }
    }

    static void resolveRequiredAncestorsJob(core::GlobalState &gs, const RequireAncestorResolutionItem &todo) {
        auto owner = todo.owner;
        auto send = todo.send;
        auto loc = core::Loc(todo.file, send->loc);

        if (!owner.data(gs)->isClassOrModuleModule() && !owner.data(gs)->isClassOrModuleAbstract()) {
            if (auto e = gs.beginError(loc, core::errors::Resolver::InvalidRequiredAncestor)) {
                e.setHeader("`{}` can only be declared inside a module or an abstract class", send->fun.show(gs));
            }
            return;
        }

        auto *block = ast::cast_tree<ast::Block>(send->block);

        if (!send->args.empty()) {
            if (auto e = gs.beginError(loc, core::errors::Resolver::InvalidRequiredAncestor)) {
                e.setHeader("`{}` only accepts a block", send->fun.show(gs));
                e.addErrorNote("Use {} to auto-correct using the new syntax",
                               "--isolate-error-code 5062 -a --typed true");

                if (block != nullptr) {
                    return;
                }

                string replacement = "";
                int indent = core::Loc::offset2Pos(todo.file.data(gs), send->loc.beginPos()).column - 1;
                int index = 1;
                for (auto &arg : send->args) {
                    auto argLoc = core::Loc(todo.file, arg.loc());
                    replacement += fmt::format("{:{}}{} {{ {} }}{}", "", index == 1 ? 0 : indent, send->fun.show(gs),
                                               argLoc.source(gs).value(), index < send->args.size() ? "\n" : "");
                    index += 1;
                }
                e.addAutocorrect(
                    core::AutocorrectSuggestion{fmt::format("Replace `{}` with `{}`", send->fun.show(gs), replacement),
                                                {core::AutocorrectSuggestion::Edit{loc, replacement}}});
            }
            return;
        }

        if (block == nullptr) {
            return; // The sig mismatch error will be emitted later by infer.
        }

        ENFORCE(block->body);

        auto blockLoc = core::Loc(todo.file, block->body.loc());
        auto *id = ast::cast_tree<ast::ConstantLit>(block->body);

        if (id == nullptr || !id->symbol.exists() || !id->symbol.data(gs)->isClassOrModule()) {
            if (auto e = gs.beginError(blockLoc, core::errors::Resolver::InvalidRequiredAncestor)) {
                e.setHeader("Argument to `{}` must be statically resolvable to a class or a module",
                            send->fun.show(gs));
            }
            return;
        }

        if (id->symbol == owner) {
            if (auto e = gs.beginError(blockLoc, core::errors::Resolver::InvalidRequiredAncestor)) {
                e.setHeader("Must not pass yourself to `{}`", send->fun.show(gs));
            }
            return;
        }

        owner.data(gs)->recordRequiredAncestor(gs, id->symbol.asClassOrModuleRef(), blockLoc);
    }

    static void tryRegisterSealedSubclass(core::MutableContext ctx, AncestorResolutionItem &job) {
        ENFORCE(job.ancestor->symbol.exists(), "Ancestor must exist, or we can't check whether it's sealed.");
        auto ancestorSym = job.ancestor->symbol.data(ctx)->dealias(ctx);

        if (!ancestorSym.data(ctx)->isClassOrModuleSealed()) {
            return;
        }
        Timer timeit(ctx.state.tracer(), "resolver.registerSealedSubclass");

        ancestorSym.data(ctx)->recordSealedSubclass(ctx, job.klass);
    }

    void transformAncestor(core::Context ctx, core::ClassOrModuleRef klass, ast::ExpressionPtr &ancestor,
                           bool isInclude, bool isSuperclass = false) {
        if (auto *constScope = ast::cast_tree<ast::UnresolvedConstantLit>(ancestor)) {
            auto scopeTmp = nesting_;
            if (isSuperclass) {
                nesting_ = nesting_->parent;
            }
            ancestor = postTransformUnresolvedConstantLit(ctx, std::move(ancestor));
            nesting_ = scopeTmp;
        }
        AncestorResolutionItem job;
        job.klass = klass;
        job.isSuperclass = isSuperclass;
        job.file = ctx.file;
        job.isInclude = isInclude;

        if (auto *cnst = ast::cast_tree<ast::ConstantLit>(ancestor)) {
            auto sym = cnst->symbol;
            if (sym.exists() && sym.data(ctx)->isTypeAlias()) {
                if (auto e = ctx.beginError(cnst->loc, core::errors::Resolver::DynamicSuperclass)) {
                    e.setHeader("Superclasses and mixins may not be type aliases");
                }
                return;
            }
            ENFORCE(sym.exists() ||
                    ast::isa_tree<ast::ConstantLit>(
                        ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(cnst->original).scope) ||
                    ast::isa_tree<ast::EmptyTree>(
                        ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(cnst->original).scope));
            if (isSuperclass && sym == core::Symbols::todo()) {
                // This is the case where the superclass is empty, for example: `class A; end`
                return;
            }
            job.ancestor = cnst;
        } else if (ancestor.isSelfReference()) {
            auto loc = ancestor.loc();
            auto enclosingClass = ctx.owner.enclosingClass(ctx);
            auto nw = ast::MK::UnresolvedConstant(loc, std::move(ancestor), enclosingClass.data(ctx)->name);
            auto out = ast::make_expression<ast::ConstantLit>(loc, enclosingClass, std::move(nw));
            job.ancestor = ast::cast_tree<ast::ConstantLit>(out);
            ancestor = std::move(out);
        } else if (ast::isa_tree<ast::EmptyTree>(ancestor)) {
            return;
        } else {
            ENFORCE(false, "Namer should have not allowed this");
        }

        todoAncestors_.emplace_back(std::move(job));
    }

public:
    ResolveConstantsWalk() : nesting_(nullptr) {}

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        nesting_ = make_unique<Nesting>(std::move(nesting_), ast::cast_tree_nonnull<ast::ClassDef>(tree).symbol);
        return tree;
    }

    ast::ExpressionPtr postTransformUnresolvedConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
        auto &c = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(tree);
        if (ast::isa_tree<ast::UnresolvedConstantLit>(c.scope)) {
            c.scope = postTransformUnresolvedConstantLit(ctx, std::move(c.scope));
        }
        auto loc = c.loc;
        auto out = ast::make_expression<ast::ConstantLit>(loc, core::Symbols::noSymbol(), std::move(tree));
        ResolutionItem job{nesting_, ctx.file, ast::cast_tree<ast::ConstantLit>(out)};
        if (resolveJob(ctx, job)) {
            categoryCounterInc("resolve.constants.nonancestor", "firstpass");
        } else {
            todo_.emplace_back(std::move(job));
        }
        return out;
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        auto klass = original.symbol;

        bool isInclude = true;
        for (auto &ancst : original.ancestors) {
            bool isSuperclass = (original.kind == ast::ClassDef::Kind::Class && &ancst == &original.ancestors.front() &&
                                 !klass.data(ctx)->isSingletonClass(ctx));
            transformAncestor(isSuperclass ? ctx : ctx.withOwner(klass), klass, ancst, isInclude, isSuperclass);
        }

        auto singleton = klass.data(ctx)->lookupSingletonClass(ctx);
        isInclude = false;
        for (auto &ancst : original.singletonAncestors) {
            ENFORCE(singleton.exists());
            transformAncestor(ctx.withOwner(klass), singleton, ancst, isInclude);
        }

        nesting_ = nesting_->parent;
        return tree;
    }

    ast::ExpressionPtr postTransformAssign(core::Context ctx, ast::ExpressionPtr tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);

        auto *id = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (id == nullptr || !id->symbol.dataAllowingNone(ctx)->isStaticField()) {
            return tree;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        if (send != nullptr && send->fun == core::Names::typeAlias()) {
            if (!send->block) {
                // if we have an invalid (i.e. nullary) call to TypeAlias, then we'll treat it as a type alias for
                // Untyped and report an error here: otherwise, we end up in a state at the end of constant resolution
                // that won't match our expected invariants (and in fact will fail our sanity checks)
                // auto temporaryUntyped = ast::MK::Untyped(asgn->lhs.get()->loc);
                auto temporaryUntyped = ast::MK::Block0(asgn.lhs.loc(), ast::MK::Untyped(asgn.lhs.loc()));
                send->block = std::move(temporaryUntyped);

                // because we're synthesizing a fake "untyped" here and actually adding it to the AST, we won't report
                // an arity mismatch for `T.untyped` in the future, so report the arity mismatch now
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeAlias)) {
                    e.setHeader("No block given to `{}`", "T.type_alias");
                    CorrectTypeAlias::eagerToLazy(ctx, e, send);
                }
            }
            auto &block = ast::cast_tree_nonnull<ast::Block>(send->block);
            auto typeAliasItem = TypeAliasResolutionItem{id->symbol, ctx.file, &block.body};
            this->todoTypeAliases_.emplace_back(std::move(typeAliasItem));

            // We also enter a ResolutionItem for the lhs of a type alias so even if the type alias isn't used,
            // we'll still emit a warning when the rhs of a type alias doesn't resolve.
            auto item = ResolutionItem{nesting_, ctx.file, id};
            this->todo_.emplace_back(std::move(item));
            return tree;
        }

        auto *rhs = ast::cast_tree<ast::ConstantLit>(asgn.rhs);
        if (rhs == nullptr) {
            return tree;
        }

        auto item = ClassAliasResolutionItem{id->symbol, ctx.file, rhs};

        // TODO(perf) currently, by construction the last item in resolve todo list is the one this alias depends on
        // We may be able to get some perf by using this
        this->todoClassAliases_.emplace_back(std::move(item));
        return tree;
    }

    ast::ExpressionPtr postTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);
        if (send.recv.isSelfReference()) {
            if (send.fun == core::Names::mixesInClassMethods()) {
                auto item = ClassMethodsResolutionItem{ctx.file, ctx.owner, &send};
                this->todoClassMethods_.emplace_back(move(item));
            } else if (send.fun == core::Names::requiresAncestor()) {
                if (ctx.state.requiresAncestorEnabled) {
                    auto item = RequireAncestorResolutionItem{ctx.file, ctx.owner, &send};
                    this->todoRequiredAncestors_.emplace_back(move(item));
                }
            }
        } else {
            auto recvAsConstantLit = ast::cast_tree<ast::ConstantLit>(send.recv);
            if (recvAsConstantLit != nullptr && recvAsConstantLit->symbol == core::Symbols::Magic() &&
                send.fun == core::Names::mixesInClassMethods()) {
                auto item = ClassMethodsResolutionItem{ctx.file, ctx.owner, &send};
                this->todoClassMethods_.emplace_back(move(item));
            }
        }
        return tree;
    }

    static bool compareLocs(const core::GlobalState &gs, core::Loc lhs, core::Loc rhs) {
        core::StrictLevel left = core::StrictLevel::Strong;
        core::StrictLevel right = core::StrictLevel::Strong;

        if (lhs.file().exists()) {
            left = lhs.file().data(gs).strictLevel;
        }
        if (rhs.file().exists()) {
            right = rhs.file().data(gs).strictLevel;
        }

        if (left != right) {
            return right < left;
        }
        if (lhs.file() != rhs.file()) {
            return lhs.file() < rhs.file();
        }
        if (lhs.beginPos() != rhs.beginPos()) {
            return lhs.beginPos() < rhs.beginPos();
        }
        return lhs.endPos() < rhs.endPos();
    }

    static int constantDepth(ast::ConstantLit *exp) {
        int depth = 0;
        ast::ConstantLit *scope = exp;
        while (scope->original && (scope = ast::cast_tree<ast::ConstantLit>(
                                       ast::cast_tree<ast::UnresolvedConstantLit>(scope->original)->scope))) {
            depth += 1;
        }
        return depth;
    }

    struct ResolveWalkResult {
        vector<ResolutionItem> todo_;
        vector<AncestorResolutionItem> todoAncestors_;
        vector<ClassAliasResolutionItem> todoClassAliases_;
        vector<TypeAliasResolutionItem> todoTypeAliases_;
        vector<ClassMethodsResolutionItem> todoClassMethods_;
        vector<RequireAncestorResolutionItem> todoRequiredAncestors_;
        vector<ast::ParsedFile> trees;
    };

    static bool locCompare(core::Loc lhs, core::Loc rhs) {
        if (lhs.file() < rhs.file()) {
            return true;
        }
        if (lhs.file() > rhs.file()) {
            return false;
        }
        if (lhs.beginPos() < rhs.beginPos()) {
            return true;
        }
        if (lhs.beginPos() > rhs.beginPos()) {
            return false;
        }
        return lhs.endPos() < rhs.endPos();
    }

    static vector<ast::ParsedFile> resolveConstants(core::GlobalState &gs, vector<ast::ParsedFile> trees,
                                                    WorkerPool &workers) {
        Timer timeit(gs.tracer(), "resolver.resolve_constants");
        const core::GlobalState &igs = gs;
        auto resultq = make_shared<BlockingBoundedQueue<ResolveWalkResult>>(trees.size());
        auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
        for (auto &tree : trees) {
            fileq->push(move(tree), 1);
        }

        workers.multiplexJob("resolveConstantsWalk", [&igs, fileq, resultq]() {
            Timer timeit(igs.tracer(), "ResolveConstantsWorker");
            ResolveConstantsWalk constants;
            vector<ast::ParsedFile> partiallyResolvedTrees;
            ast::ParsedFile job;
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    core::Context ictx(igs, core::Symbols::root(), job.file);
                    job.tree = ast::TreeMap::apply(ictx, constants, std::move(job.tree));
                    partiallyResolvedTrees.emplace_back(move(job));
                }
            }
            if (!partiallyResolvedTrees.empty()) {
                ResolveWalkResult result{move(constants.todo_),
                                         move(constants.todoAncestors_),
                                         move(constants.todoClassAliases_),
                                         move(constants.todoTypeAliases_),
                                         move(constants.todoClassMethods_),
                                         move(constants.todoRequiredAncestors_),
                                         move(partiallyResolvedTrees)};
                auto computedTreesCount = result.trees.size();
                resultq->push(move(result), computedTreesCount);
            }
        });
        trees.clear();
        vector<ResolutionItem> todo;
        vector<AncestorResolutionItem> todoAncestors;
        vector<ClassAliasResolutionItem> todoClassAliases;
        vector<TypeAliasResolutionItem> todoTypeAliases;
        vector<ClassMethodsResolutionItem> todoClassMethods;
        vector<RequireAncestorResolutionItem> todoRequiredAncestors;

        {
            ResolveWalkResult threadResult;
            for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
                 !result.done();
                 result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
                if (result.gotItem()) {
                    todo.insert(todo.end(), make_move_iterator(threadResult.todo_.begin()),
                                make_move_iterator(threadResult.todo_.end()));
                    todoAncestors.insert(todoAncestors.end(), make_move_iterator(threadResult.todoAncestors_.begin()),
                                         make_move_iterator(threadResult.todoAncestors_.end()));
                    todoClassAliases.insert(todoClassAliases.end(),
                                            make_move_iterator(threadResult.todoClassAliases_.begin()),
                                            make_move_iterator(threadResult.todoClassAliases_.end()));
                    todoTypeAliases.insert(todoTypeAliases.end(),
                                           make_move_iterator(threadResult.todoTypeAliases_.begin()),
                                           make_move_iterator(threadResult.todoTypeAliases_.end()));
                    todoClassMethods.insert(todoClassMethods.end(),
                                            make_move_iterator(threadResult.todoClassMethods_.begin()),
                                            make_move_iterator(threadResult.todoClassMethods_.end()));
                    todoRequiredAncestors.insert(todoRequiredAncestors.end(),
                                                 make_move_iterator(threadResult.todoRequiredAncestors_.begin()),
                                                 make_move_iterator(threadResult.todoRequiredAncestors_.end()));
                    trees.insert(trees.end(), make_move_iterator(threadResult.trees.begin()),
                                 make_move_iterator(threadResult.trees.end()));
                }
            }
        }

        fast_sort(todo, [](const auto &lhs, const auto &rhs) -> bool {
            return locCompare(core::Loc(lhs.file, lhs.out->loc), core::Loc(rhs.file, rhs.out->loc));
        });
        fast_sort(todoAncestors, [](const auto &lhs, const auto &rhs) -> bool {
            return locCompare(core::Loc(lhs.file, lhs.ancestor->loc), core::Loc(rhs.file, rhs.ancestor->loc));
        });
        fast_sort(todoClassAliases, [](const auto &lhs, const auto &rhs) -> bool {
            return locCompare(core::Loc(lhs.file, lhs.rhs->loc), core::Loc(rhs.file, rhs.rhs->loc));
        });
        fast_sort(todoTypeAliases, [](const auto &lhs, const auto &rhs) -> bool {
            return locCompare(core::Loc(lhs.file, (*lhs.rhs).loc()), core::Loc(rhs.file, (*rhs.rhs).loc()));
        });
        fast_sort(todoClassMethods, [](const auto &lhs, const auto &rhs) -> bool {
            return locCompare(core::Loc(lhs.file, lhs.send->loc), core::Loc(rhs.file, rhs.send->loc));
        });
        fast_sort(todoRequiredAncestors, [](const auto &lhs, const auto &rhs) -> bool {
            return locCompare(core::Loc(lhs.file, lhs.send->loc), core::Loc(rhs.file, rhs.send->loc));
        });

        ENFORCE(todoRequiredAncestors.empty() || gs.requiresAncestorEnabled);
        fast_sort(trees, [](const auto &lhs, const auto &rhs) -> bool {
            return locCompare(core::Loc(lhs.file, lhs.tree.loc()), core::Loc(rhs.file, rhs.tree.loc()));
        });

        Timer timeit1(gs.tracer(), "resolver.resolve_constants.fixed_point");

        bool progress = true;
        bool first = true; // we need to run at least once to force class aliases and type aliases

        while (progress && (first || !todo.empty() || !todoAncestors.empty())) {
            first = false;
            counterInc("resolve.constants.retries");
            {
                Timer timeit(gs.tracer(), "resolver.resolve_constants.fixed_point.ancestors");
                // This is an optimization. The order should not matter semantically
                // We try to resolve most ancestors second because this makes us much more likely to resolve everything
                // else.
                int origSize = todoAncestors.size();
                auto it =
                    remove_if(todoAncestors.begin(), todoAncestors.end(), [&gs](AncestorResolutionItem &job) -> bool {
                        core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                        auto resolved = resolveAncestorJob(ctx, job, false);
                        if (resolved) {
                            tryRegisterSealedSubclass(ctx, job);
                        }
                        return resolved;
                    });
                todoAncestors.erase(it, todoAncestors.end());
                progress = (origSize != todoAncestors.size());
                categoryCounterAdd("resolve.constants.ancestor", "retry", origSize - todoAncestors.size());
            }
            {
                Timer timeit(gs.tracer(), "resolver.resolve_constants.fixed_point.constants");
                int origSize = todo.size();
                auto it = remove_if(todo.begin(), todo.end(), [&gs](ResolutionItem &job) -> bool {
                    core::Context ictx(gs, core::Symbols::root(), job.file);
                    return resolveJob(ictx, job);
                });
                todo.erase(it, todo.end());
                progress = progress || (origSize != todo.size());
                categoryCounterAdd("resolve.constants.nonancestor", "retry", origSize - todo.size());
            }
            {
                Timer timeit(gs.tracer(), "resolver.resolve_constants.fixed_point.class_aliases");
                // This is an optimization. The order should not matter semantically
                // This is done as a "pre-step" because the first iteration of this effectively ran in TreeMap.
                // every item in todoClassAliases implicitly depends on an item in item in todo
                // there would be no point in running the todoClassAliases step before todo

                int origSize = todoClassAliases.size();
                auto it = remove_if(todoClassAliases.begin(), todoClassAliases.end(),
                                    [&gs](ClassAliasResolutionItem &it) -> bool {
                                        core::MutableContext ctx(gs, core::Symbols::root(), it.file);
                                        return resolveClassAliasJob(ctx, it);
                                    });
                todoClassAliases.erase(it, todoClassAliases.end());
                progress = progress || (origSize != todoClassAliases.size());
                categoryCounterAdd("resolve.constants.aliases", "retry", origSize - todoClassAliases.size());
            }
            {
                Timer timeit(gs.tracer(), "resolver.resolve_constants.fixed_point.type_aliases");
                int origSize = todoTypeAliases.size();
                auto it = remove_if(todoTypeAliases.begin(), todoTypeAliases.end(),
                                    [&gs](TypeAliasResolutionItem &it) -> bool {
                                        core::MutableContext ctx(gs, core::Symbols::root(), it.file);
                                        return resolveTypeAliasJob(ctx, it);
                                    });
                todoTypeAliases.erase(it, todoTypeAliases.end());
                progress = progress || (origSize != todoTypeAliases.size());
                categoryCounterAdd("resolve.constants.typealiases", "retry", origSize - todoTypeAliases.size());
            }
        }

        {
            Timer timeit(gs.tracer(), "resolver.mixes_in_class_methods");
            for (auto &todo : todoClassMethods) {
                resolveClassMethodsJob(gs, todo);
            }
            todoClassMethods.clear();
        }

        {
            Timer timeit(gs.tracer(), "resolver.requires_ancestor");
            for (auto &todo : todoRequiredAncestors) {
                resolveRequiredAncestorsJob(gs, todo);
            }
            todoRequiredAncestors.clear();
        }

        // We can no longer resolve new constants. All the code below reports errors

        categoryCounterAdd("resolve.constants.nonancestor", "failure", todo.size());
        categoryCounterAdd("resolve.constants.ancestor", "failure", todoAncestors.size());

        /*
         * Sort errors so we choose a deterministic error to report for each
         * missing constant:
         *
         * - Visit the strictest files first. If we were to report an error in
         *     an untyped file it would get suppressed, even if the same error
         *     also appeared in a typed file.
         *
         * - Break ties within strictness levels by file ID. We populate file
         *     IDs in the order we are given files on the command-line, so this
         *     means users see the error on the first file they provided.
         *
         * - Within a file, report the first occurrence.
         */
        fast_sort(todo, [&gs](const auto &lhs, const auto &rhs) -> bool {
            if (core::Loc(lhs.file, lhs.out->loc) == core::Loc(rhs.file, rhs.out->loc)) {
                return constantDepth(lhs.out) < constantDepth(rhs.out);
            }
            return compareLocs(gs, core::Loc(lhs.file, lhs.out->loc), core::Loc(rhs.file, rhs.out->loc));
        });

        fast_sort(todoAncestors, [&gs](const auto &lhs, const auto &rhs) -> bool {
            if (core::Loc(lhs.file, lhs.ancestor->loc) == core::Loc(rhs.file, rhs.ancestor->loc)) {
                return constantDepth(lhs.ancestor) < constantDepth(rhs.ancestor);
            }
            return compareLocs(gs, core::Loc(lhs.file, lhs.ancestor->loc), core::Loc(rhs.file, rhs.ancestor->loc));
        });

        // Note that this is missing alias stubbing, thus resolveJob needs to be able to handle missing aliases.

        {
            Timer timeit(gs.tracer(), "resolver.resolve_constants.errors");
            int i = -1;
            for (auto &job : todo) {
                i++;
                // Only give suggestions for the first 10, because fuzzy suggestions are expensive.
                auto suggestDidYouMean = i < 10;
                core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                constantResolutionFailed(ctx, job, suggestDidYouMean);
            }

            for (auto &job : todoAncestors) {
                core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                auto resolved = resolveAncestorJob(ctx, job, true);
                if (!resolved) {
                    resolved = resolveAncestorJob(ctx, job, true);
                    ENFORCE(resolved);
                }
            }
        }

        return trees;
    }
};

class ResolveTypeMembersAndFieldsWalk {
    // A type_member, type_template, or T.type_alias that needs to have types
    // resolved.
    struct ResolveAssignItem {
        // The owner at the time the assignment was encountered.
        core::SymbolRef owner;

        // The symbol being populated, either a type alias or an individual
        // type_member.
        core::SymbolRef lhs;

        // The type_member, type_template, or type_alias of the RHS.
        ast::Send *rhs;

        // The symbols that this alias depends on that have type members.
        vector<core::SymbolRef> dependencies;

        core::FileRef file;
    };

    struct ResolveAttachedClassItem {
        core::SymbolRef owner;
        core::ClassOrModuleRef klass;
        core::FileRef file;
    };

    struct ResolveCastItem {
        core::FileRef file;
        core::SymbolRef owner;
        ast::ExpressionPtr *typeArg;
        ast::Cast *cast;
    };

    struct ResolveFieldItem {
        core::FileRef file;
        core::SymbolRef owner;
        ast::UnresolvedIdent *ident;
        ast::Cast *cast;
        bool atTopLevel = false;
    };

    struct ResolveSimpleStaticFieldItem {
        core::FieldRef sym;
        core::TypePtr resultType;
    };

    struct ResolveStaticFieldItem {
        core::FileRef file;
        core::FieldRef sym;
        ast::Assign *asgn;
    };

    struct ResolveMethodAliasItem {
        core::FileRef file;
        core::ClassOrModuleRef owner;
        core::LocOffsets loc;
        core::LocOffsets toNameLoc;
        core::NameRef toName;
        core::NameRef fromName;
    };

    struct ResolveTypeMembersAndFieldsResult {
        vector<ast::ParsedFile> files;
        vector<ResolveAssignItem> todoAssigns;
        vector<ResolveAttachedClassItem> todoAttachedClassItems;
        vector<core::SymbolRef> todoUntypedResultTypes;
        vector<ResolveCastItem> todoResolveCastItems;
        vector<ResolveFieldItem> todoResolveFieldItems;
        vector<ResolveStaticFieldItem> todoResolveStaticFieldItems;
        vector<ResolveSimpleStaticFieldItem> todoResolveSimpleStaticFieldItems;
        vector<ResolveMethodAliasItem> todoMethodAliasItems;
    };

    vector<ResolveAssignItem> todoAssigns_;
    vector<ResolveAttachedClassItem> todoAttachedClassItems_;
    vector<core::SymbolRef> todoUntypedResultTypes_;
    vector<ResolveCastItem> todoResolveCastItems_;
    vector<ResolveFieldItem> todoResolveFieldItems_;
    vector<ResolveStaticFieldItem> todoResolveStaticFieldItems_;
    vector<ResolveSimpleStaticFieldItem> todoResolveSimpleStaticFieldItems_;
    vector<ResolveMethodAliasItem> todoMethodAliasItems_;

    // State for tracking type usage inside of a type alias or type member
    // definition
    bool trackDependencies_ = false;
    vector<bool> classOfDepth_;
    vector<core::SymbolRef> dependencies_;
    std::vector<int> nestedBlockCounts;

    void extendClassOfDepth(ast::Send &send) {
        if (trackDependencies_) {
            classOfDepth_.emplace_back(isT(send.recv) && send.fun == core::Names::classOf());
        }
    }

    static bool isT(const ast::ExpressionPtr &expr) {
        auto *tMod = ast::cast_tree<ast::ConstantLit>(expr);
        return tMod && tMod->symbol == core::Symbols::T();
    }

    static bool isTodo(const core::TypePtr &type) {
        return core::isa_type<core::ClassType>(type) &&
               core::cast_type_nonnull<core::ClassType>(type).symbol == core::Symbols::todo();
    }

    static bool isLHSResolved(core::Context ctx, core::SymbolRef sym) {
        if (sym.isTypeMember()) {
            auto *lambdaParam = core::cast_type<core::LambdaParam>(sym.data(ctx)->resultType);
            ENFORCE(lambdaParam != nullptr);

            // both bounds are set to todo in the namer, so it's sufficient to
            // just check one here.
            return !isTodo(lambdaParam->lowerBound);
        } else {
            return !isTodo(sym.data(ctx)->resultType);
        }
    }

    static bool isGenericResolved(core::Context ctx, core::SymbolRef sym) {
        if (sym.isClassOrModule()) {
            return absl::c_all_of(sym.data(ctx)->typeMembers(),
                                  [&](core::SymbolRef tm) { return isLHSResolved(ctx, tm); });
        } else {
            return isLHSResolved(ctx, sym);
        }
    }

    // Resolve a cast to a simple, non-generic class type (e.g., T.let(x, ClassOrModule)). Returns `false` if
    // `ResolveCastItem` is not simple.
    [[nodiscard]] static bool tryResolveSimpleClassCastItem(core::Context ctx, ResolveCastItem &job) {
        if (!ast::isa_tree<ast::ConstantLit>(*job.typeArg)) {
            return false;
        }

        auto &lit = ast::cast_tree_nonnull<ast::ConstantLit>(*job.typeArg);
        auto data = lit.symbol.data(ctx);
        if (!data->isClassOrModule()) {
            return false;
        }

        // A class with type members is not simple.
        if (!data->typeMembers().empty()) {
            return false;
        }

        resolveCastItem(ctx, job);
        return true;
    }

    // Resolve a potentially more complex cast (e.g., may reference type member or alias).
    static void resolveCastItem(core::Context ctx, ResolveCastItem &job) {
        ParsedSig emptySig;
        auto allowSelfType = true;
        auto allowRebind = false;
        auto allowTypeMember = true;
        job.cast->type = TypeSyntax::getResultType(
            ctx, *job.typeArg, emptySig,
            TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, core::Symbols::noSymbol()});
    }

    // Attempts to resolve the type of the given field. Returns `false` if the cast is not yet resolved.
    static void resolveField(core::MutableContext ctx, ResolveFieldItem &job) {
        auto cast = job.cast;

        core::ClassOrModuleRef scope;
        auto uid = job.ident;
        if (uid->kind == ast::UnresolvedIdent::Kind::Class) {
            if (!ctx.owner.data(ctx)->isClassOrModule()) {
                if (auto e = ctx.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                    e.setHeader("The class variable `{}` must be declared at class scope", uid->name.show(ctx));
                }
            }

            scope = ctx.owner.enclosingClass(ctx);
        } else {
            // we need to check nested block counts because we want all fields to be declared on top level of either
            // class or body, rather then nested in some block
            if (job.atTopLevel && ctx.owner.data(ctx)->isClassOrModule()) {
                // Declaring a class instance variable
            } else if (job.atTopLevel && ctx.owner.data(ctx)->name == core::Names::initialize()) {
                // Declaring a instance variable
            } else if (ctx.owner.data(ctx)->isMethod() && ctx.owner.data(ctx)->owner.data(ctx)->isSingletonClass(ctx) &&
                       !core::Types::isSubType(ctx, core::Types::nilClass(), cast->type)) {
                // Declaring a class instance variable in a static method
                if (auto e = ctx.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                    e.setHeader("The singleton instance variable `{}` must be declared inside the class body or "
                                "declared nilable",
                                uid->name.show(ctx));
                }
            } else if (!core::Types::isSubType(ctx, core::Types::nilClass(), cast->type)) {
                // Inside a method; declaring a normal instance variable
                if (auto e = ctx.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                    e.setHeader("The instance variable `{}` must be declared inside `{}` or declared nilable",
                                uid->name.show(ctx), "initialize");
                }
            }
            scope = ctx.selfClass();
        }

        auto prior = scope.data(ctx)->findMember(ctx, uid->name);
        if (prior.exists()) {
            if (core::Types::equiv(ctx, prior.data(ctx)->resultType, cast->type)) {
                // We already have a symbol for this field, and it matches what we already saw, so we can short
                // circuit.
                return;
            } else {
                // We do some normalization here to ensure that the file / line we report the error on doesn't
                // depend on the order that we traverse files nor the order we traverse within a file.
                auto priorLoc = prior.data(ctx)->loc();
                core::Loc reportOn;
                core::Loc errorLine;
                core::Loc thisLoc = core::Loc(job.file, uid->loc);
                if (thisLoc.file() == priorLoc.file()) {
                    reportOn = thisLoc.beginPos() < priorLoc.beginPos() ? thisLoc : priorLoc;
                    errorLine = thisLoc.beginPos() < priorLoc.beginPos() ? priorLoc : thisLoc;
                } else {
                    reportOn = thisLoc.file() < priorLoc.file() ? thisLoc : priorLoc;
                    errorLine = thisLoc.file() < priorLoc.file() ? priorLoc : thisLoc;
                }

                if (auto e = ctx.state.beginError(reportOn, core::errors::Resolver::DuplicateVariableDeclaration)) {
                    e.setHeader("Redeclaring variable `{}` with mismatching type", uid->name.show(ctx));
                    e.addErrorLine(errorLine, "Previous declaration is here:");
                }
                return;
            }
        }
        core::FieldRef var;

        if (uid->kind == ast::UnresolvedIdent::Kind::Class) {
            var = ctx.state.enterStaticFieldSymbol(core::Loc(job.file, uid->loc), scope, uid->name);
        } else {
            var = ctx.state.enterFieldSymbol(core::Loc(job.file, uid->loc), scope, uid->name);
        }

        var.data(ctx)->resultType = cast->type;
        return;
    }

    // Resolve the type of the rhs of a constant declaration. This logic is
    // extremely simplistic; We only handle simple literals, and explicit casts.
    //
    // We don't handle array or hash literals, because intuiting the element
    // type (once we have generics) will be nontrivial.
    [[nodiscard]] static core::TypePtr resolveConstantType(core::Context ctx, const ast::ExpressionPtr &expr) {
        core::TypePtr result;
        typecase(
            expr, [&](const ast::Literal &a) { result = a.value; },
            [&](const ast::Cast &cast) {
                if (cast.type == core::Types::todo()) {
                    return;
                }

                if (cast.cast != core::Names::let() && cast.cast != core::Names::uncheckedLet()) {
                    if (auto e = ctx.beginError(cast.loc, core::errors::Resolver::ConstantAssertType)) {
                        e.setHeader("Use `{}` to specify the type of constants", "T.let");
                    }
                }
                result = cast.type;
            },
            [&](const ast::InsSeq &outer) { result = resolveConstantType(ctx, outer.expr); },
            [&](const ast::ExpressionPtr &expr) {});
        return result;
    }

    // Tries to resolve the given static field. Returns Types::todo() if it is unable to resolve the field.
    [[nodiscard]] static core::TypePtr tryResolveStaticField(core::Context ctx, ResolveStaticFieldItem &job) {
        ENFORCE(job.sym.data(ctx)->isStaticField());
        auto &asgn = job.asgn;
        auto data = job.sym.data(ctx);
        if (data->resultType == nullptr) {
            if (auto resultType = resolveConstantType(ctx, asgn->rhs)) {
                return resultType;
            }
        }
        // resultType was already set. We may be running on the incremental path. Force this field to be resolved in
        // `resolveStaticField`, which may produce an error message.
        return core::Types::todo();
    }

    [[nodiscard]] static core::TypePtr resolveStaticField(core::Context ctx, ResolveStaticFieldItem &job) {
        ENFORCE(job.sym.data(ctx)->isStaticField());
        auto &asgn = job.asgn;
        auto data = job.sym.data(ctx);
        if (data->resultType == nullptr) {
            auto resultType = resolveConstantType(ctx, asgn->rhs);
            if (resultType == nullptr) {
                // Instead of emitting an error now, emit an error in infer that has a proper type suggestion
                auto rhs = move(job.asgn->rhs);
                auto loc = rhs.loc();
                if (!loc.exists()) {
                    // If the rhs happens to be an EmptyTree (e.g., `begin; end`) there will be no loc.
                    // In that case, use the assign's loc instead.
                    loc = job.asgn->loc;
                }
                job.asgn->rhs = ast::MK::Send1(loc, ast::MK::Constant(loc, core::Symbols::Magic()),
                                               core::Names::suggestType(), move(rhs));
            }
            return resultType;
        } else if (!core::isa_type<core::AliasType>(data->resultType)) {
            // If we've already resolved a temporary constant, we still want to run resolveConstantType to
            // report errors (e.g. so that a stand-in untyped value won't suppress errors in subsequent
            // typechecking runs) but we only want to run this on constants that are value-level and not class
            // or type aliases. The check for isa_type<AliasType> makes sure that we skip aliases of the form `X
            // = Integer` and only run this over constant value assignments like `X = 5` or `Y = 5; X = Y`.
            if (resolveConstantType(ctx, asgn->rhs) == nullptr) {
                if (auto e = ctx.beginError(asgn->rhs.loc(), core::errors::Resolver::ConstantMissingTypeAnnotation)) {
                    e.setHeader("Constants must have type annotations with `{}` when specifying `{}`", "T.let",
                                "# typed: strict");
                }
            }
        }
        return data->resultType;
    }

    static void resolveTypeMember(core::MutableContext ctx, core::TypeMemberRef lhs, ast::Send *rhs,
                                  vector<bool> &resolvedAttachedClasses) {
        auto data = lhs.data(ctx);
        auto owner = data->owner;

        core::LambdaParam *parentType = nullptr;
        core::SymbolRef parentMember = core::Symbols::noSymbol();
        parentMember = owner.data(ctx)->superClass().data(ctx)->findMember(ctx, data->name);

        // check mixins if the type member doesn't exist in the parent
        if (!parentMember.exists()) {
            for (auto mixin : owner.data(ctx)->mixins()) {
                parentMember = mixin.data(ctx)->findMember(ctx, data->name);
                if (parentMember.exists()) {
                    break;
                }
            }
        }

        if (parentMember.exists()) {
            if (parentMember.isTypeMember()) {
                parentType = core::cast_type<core::LambdaParam>(parentMember.data(ctx)->resultType);
                ENFORCE(parentType != nullptr);
            } else if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::ParentTypeBoundsMismatch)) {
                const auto parentShow = parentMember.show(ctx);
                e.setHeader("`{}` is a type member but `{}` is not a type member", lhs.show(ctx), parentShow);
                e.addErrorLine(parentMember.data(ctx)->loc(), "`{}` definition", parentShow);
            }
        }

        // Initialize the resultType to a LambdaParam with default bounds
        auto lambdaParam = core::make_type<core::LambdaParam>(lhs, core::Types::bottom(), core::Types::top());
        data->resultType = lambdaParam;
        auto *memberType = core::cast_type<core::LambdaParam>(lambdaParam);

        // When no args are supplied, this implies that the upper and lower
        // bounds of the type parameter are top and bottom.
        auto [posEnd, kwEnd] = rhs->kwArgsRange();
        if (kwEnd - posEnd > 0) {
            for (auto i = posEnd; i < kwEnd; i += 2) {
                auto &keyExpr = rhs->args[i];

                auto lit = ast::cast_tree<ast::Literal>(keyExpr);
                if (lit && lit->isSymbol(ctx)) {
                    auto &value = rhs->args[i + 1];

                    ParsedSig emptySig;
                    auto allowSelfType = true;
                    auto allowRebind = false;
                    auto allowTypeMember = false;
                    core::TypePtr resTy = TypeSyntax::getResultType(
                        ctx, value, emptySig, TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, lhs});

                    switch (lit->asSymbol(ctx).rawId()) {
                        case core::Names::fixed().rawId():
                            memberType->lowerBound = resTy;
                            memberType->upperBound = resTy;
                            break;

                        case core::Names::lower().rawId():
                            memberType->lowerBound = resTy;
                            break;

                        case core::Names::upper().rawId():
                            memberType->upperBound = resTy;
                            break;
                    }
                }
            }
        }

        // If the parent bounds existis, validate the new bounds against
        // those of the parent.
        // NOTE: these errors could be better for cases involving
        // `fixed`.
        if (parentType != nullptr) {
            if (!core::Types::isSubType(ctx, parentType->lowerBound, memberType->lowerBound)) {
                if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::ParentTypeBoundsMismatch)) {
                    e.setHeader("parent lower bound `{}` is not a subtype of lower bound `{}`",
                                parentType->lowerBound.show(ctx), memberType->lowerBound.show(ctx));
                }
            }
            if (!core::Types::isSubType(ctx, memberType->upperBound, parentType->upperBound)) {
                if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::ParentTypeBoundsMismatch)) {
                    e.setHeader("upper bound `{}` is not a subtype of parent upper bound `{}`",
                                memberType->upperBound.show(ctx), parentType->upperBound.show(ctx));
                }
            }
        }

        // Ensure that the new lower bound is a subtype of the upper
        // bound. This will be a no-op in the case that the type member
        // is fixed.
        if (!core::Types::isSubType(ctx, memberType->lowerBound, memberType->upperBound)) {
            if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::InvalidTypeMemberBounds)) {
                e.setHeader("`{}` is not a subtype of `{}`", memberType->lowerBound.show(ctx),
                            memberType->upperBound.show(ctx));
            }
        }

        // Once the owner has had all of its type members resolved, resolve the
        // AttachedClass on its singleton.
        if (isGenericResolved(ctx, owner)) {
            resolveAttachedClass(ctx, owner.asClassOrModuleRef(), resolvedAttachedClasses);
        }
    }

    static void resolveAttachedClass(core::MutableContext ctx, core::ClassOrModuleRef sym,
                                     vector<bool> &resolvedAttachedClasses) {
        // Avoid trying to re-resolve symbols that are already resolved.
        // This avoids (relatively) expensive findMember operations.
        if (resolvedAttachedClasses[sym.id()] == true) {
            return;
        }

        resolvedAttachedClasses[sym.id()] = true;

        auto singleton = sym.data(ctx)->lookupSingletonClass(ctx);
        if (!singleton.exists()) {
            return;
        }

        // NOTE: AttachedClass will not exist on `T.untyped`, which is a problem
        // because RuntimeProfiled is used as a synonym for `T.untyped`
        // internally.
        auto attachedClass = singleton.data(ctx)->findMember(ctx, core::Names::Constants::AttachedClass());
        if (!attachedClass.exists()) {
            return;
        }

        auto *lambdaParam = core::cast_type<core::LambdaParam>(attachedClass.data(ctx)->resultType);
        ENFORCE(lambdaParam != nullptr);

        if (isTodo(lambdaParam->lowerBound)) {
            lambdaParam->upperBound = sym.data(ctx)->unsafeComputeExternalType(ctx);
            lambdaParam->lowerBound = core::Types::bottom();
        }

        // If all of the singleton members have been resolved, attempt to
        // resolve the singleton of the singleton, if it exists. This case is
        // not redundant with resolveTypeMember, as it will cover the case where
        // there are no non-AttachedClass type members defined on the singleton.
        if (isGenericResolved(ctx, singleton)) {
            // Since we've resolved the singleton's AttachedClass type member, check
            // to see if there's another singleton above that must also be resolved.
            auto parent = singleton.data(ctx)->lookupSingletonClass(ctx);
            if (parent.exists()) {
                resolveAttachedClass(ctx, singleton, resolvedAttachedClasses);
            }
        }
    }

    static void resolveTypeAlias(core::MutableContext ctx, core::SymbolRef lhs, ast::Send *rhs) {
        // this is provided by ResolveConstantsWalk
        ENFORCE(ast::isa_tree<ast::Block>(rhs->block));
        auto &block = ast::cast_tree_nonnull<ast::Block>(rhs->block);
        ENFORCE(block.body);

        auto allowSelfType = true;
        auto allowRebind = false;
        auto allowTypeMember = true;
        lhs.data(ctx)->resultType = TypeSyntax::getResultType(
            ctx, block.body, ParsedSig{}, TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, lhs});
    }

    static bool resolveJob(core::MutableContext ctx, ResolveAssignItem &job, vector<bool> &resolvedAttachedClasses) {
        ENFORCE(job.lhs.data(ctx)->isTypeAlias() || job.lhs.isTypeMember());

        if (isLHSResolved(ctx, job.lhs)) {
            return true;
        }

        auto it = std::remove_if(job.dependencies.begin(), job.dependencies.end(), [&](core::SymbolRef dep) {
            if (isGenericResolved(ctx, dep)) {
                if (dep.isClassOrModule()) {
                    // `dep`'s dependencies are resolved. Compute its externalType here so that we can resolve this
                    // type member or type alias's type.
                    dep.data(ctx)->unsafeComputeExternalType(ctx);
                }
                return true;
            }
            return false;
        });
        job.dependencies.erase(it, job.dependencies.end());
        if (!job.dependencies.empty()) {
            return false;
        }
        if (job.lhs.isTypeMember()) {
            resolveTypeMember(ctx.withOwner(job.owner), job.lhs.asTypeMemberRef(), job.rhs, resolvedAttachedClasses);
        } else {
            resolveTypeAlias(ctx.withOwner(job.owner), job.lhs, job.rhs);
        }

        return true;
    }

    static void resolveMethodAlias(core::MutableContext ctx, const ResolveMethodAliasItem &job) {
        core::SymbolRef member = ctx.owner.data(ctx)->findMemberNoDealias(ctx, job.toName);
        // TODO(jvilk): Would be nice to have findMember that returned MethodRef.
        core::MethodRef toMethod = core::Symbols::noMethod();
        if (member.exists()) {
            toMethod = member.data(ctx)->dealiasMethod(ctx);
        }

        if (!toMethod.exists()) {
            if (auto e = ctx.beginError(job.toNameLoc, core::errors::Resolver::BadAliasMethod)) {
                e.setHeader("Can't make method alias from `{}` to non existing method `{}`", job.fromName.show(ctx),
                            job.toName.show(ctx));
            }
            toMethod = core::Symbols::Sorbet_Private_Static_badAliasMethodStub();
        }

        core::SymbolRef fromMethod = ctx.owner.data(ctx)->findMemberNoDealias(ctx, job.fromName);
        if (fromMethod.exists() && fromMethod.data(ctx)->dealiasMethod(ctx) != toMethod) {
            if (auto e = ctx.beginError(job.loc, core::errors::Resolver::BadAliasMethod)) {
                auto dealiased = fromMethod.data(ctx)->dealiasMethod(ctx);
                if (fromMethod == dealiased) {
                    e.setHeader("Redefining the existing method `{}` as a method alias", fromMethod.show(ctx));
                    e.addErrorLine(fromMethod.data(ctx)->loc(), "Previous definition");
                } else {
                    e.setHeader("Redefining method alias `{}` from `{}` to `{}`", fromMethod.show(ctx),
                                dealiased.show(ctx), toMethod.show(ctx));
                    e.addErrorLine(fromMethod.data(ctx)->loc(), "Previous alias definition");
                    e.addErrorLine(dealiased.data(ctx)->loc(), "Previous alias pointed to");
                    e.addErrorLine(toMethod.data(ctx)->loc(), "Redefining alias to");
                }
            }
            return;
        }

        // No need to make an alias when they're already the same symbol.
        if (fromMethod == toMethod) {
            return;
        }

        auto alias = ctx.state.enterMethodSymbol(core::Loc(ctx.file, job.loc), job.owner, job.fromName);
        alias.data(ctx)->resultType = core::make_type<core::AliasType>(core::SymbolRef(toMethod));
    }

    // Returns `true` if `asgn` is a field declaration.
    bool handleFieldDeclaration(core::Context ctx, ast::Assign &asgn) {
        auto *uid = ast::cast_tree<ast::UnresolvedIdent>(asgn.lhs);
        if (uid == nullptr) {
            return false;
        }

        if (uid->kind != ast::UnresolvedIdent::Kind::Instance && uid->kind != ast::UnresolvedIdent::Kind::Class) {
            return false;
        }

        auto *recur = &asgn.rhs;
        while (auto *outer = ast::cast_tree<ast::InsSeq>(*recur)) {
            recur = &outer->expr;
        }

        auto *cast = ast::cast_tree<ast::Cast>(*recur);
        if (cast == nullptr) {
            return false;
        } else if (cast->cast != core::Names::let()) {
            if (auto e = ctx.beginError(cast->loc, core::errors::Resolver::ConstantAssertType)) {
                e.setHeader("Use `{}` to specify the type of constants", "T.let");
                auto rhsLoc = core::Loc(ctx.file, asgn.rhs.loc());
                auto argSource = core::Loc(ctx.file, cast->arg.loc()).source(ctx).value();
                e.replaceWith("Replace with `T.let`", rhsLoc, "T.let({}, {})", argSource, cast->type.show(ctx));
                if (cast->cast == core::Names::cast()) {
                    e.addErrorNote("If you really want to use `{}`, assign to an intermediate variable first and then "
                                   "assign that variable to `{}`",
                                   "T.cast", asgn.lhs.toString(ctx));
                }
            }
        }

        ENFORCE_NO_TIMER(!nestedBlockCounts.empty());
        todoResolveFieldItems_.emplace_back(
            ResolveFieldItem{ctx.file, ctx.owner, uid, cast, nestedBlockCounts.back() == 0});

        return true;
    }

    static void computeExternalTypes(core::GlobalState &gs) {
        Timer timeit(gs.tracer(), "resolver.computeExternalType");
        // Ensure all symbols have `externalType` computed.
        for (u4 i = 1; i < gs.classAndModulesUsed(); i++) {
            core::ClassOrModuleRef(gs, i).data(gs)->unsafeComputeExternalType(gs);
        }
    }

    void validateNonForcingIsA(core::Context ctx, const ast::Send &send) {
        constexpr string_view method = "T::NonForcingConstants.non_forcing_is_a?";

        if (send.numPosArgs != 2) {
            return;
        }

        auto [posEnd, kwEnd] = send.kwArgsRange();
        auto numKwArgs = (kwEnd - posEnd) >> 1;
        if (numKwArgs > 1) {
            return;
        }

        auto stringLoc = send.args[1].loc();

        auto *literalNode = ast::cast_tree<ast::Literal>(send.args[1]);
        if (literalNode == nullptr) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("`{}` only accepts string literals", method);
            }
            return;
        }

        if (!core::isa_type<core::LiteralType>(literalNode->value)) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("`{}` only accepts string literals", method);
            }
            return;
        }

        auto literal = core::cast_type_nonnull<core::LiteralType>(literalNode->value);
        if (literal.literalKind != core::LiteralType::LiteralTypeKind::String) {
            // Infer will report a type error
            return;
        }

        core::TypePtr packageType = nullptr;
        optional<core::LocOffsets> packageLoc;
        if (send.hasKwArgs()) {
            // this means we got the third package arg
            auto *key = ast::cast_tree<ast::Literal>(send.args[posEnd]);
            if (!key || !key->isSymbol(ctx) || key->asSymbol(ctx) != ctx.state.lookupNameUTF8("package")) {
                return;
            }

            auto *packageNode = ast::cast_tree<ast::Literal>(send.args[posEnd + 1]);
            packageLoc = std::optional<core::LocOffsets>{send.args[posEnd + 1].loc()};
            if (packageNode == nullptr) {
                if (auto e = ctx.beginError(send.args[posEnd + 1].loc(), core::errors::Resolver::LazyResolve)) {
                    e.setHeader("`{}` only accepts string literals", method);
                }
                return;
            }

            if (!core::isa_type<core::LiteralType>(packageNode->value) ||
                core::cast_type_nonnull<core::LiteralType>(packageNode->value).literalKind !=
                    core::LiteralType::LiteralTypeKind::String) {
                // Infer will report a type error
                return;
            }
            packageType = packageNode->value;
        }
        // if we got no keyword args, then package should be null, and
        // if we got keyword args, then package should be non-null
        ENFORCE((!send.hasKwArgs() && !packageType) || (send.hasKwArgs() && packageType));

        auto name = literal.asName(ctx);
        auto shortName = name.shortName(ctx);
        if (shortName.empty()) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("The string given to `{}` must not be empty", method);
            }
            return;
        }

        // If this string _begins_ with `::`, then the first fragment will be an empty string; in multiple places below,
        // we'll check to find out whether the first part is `""` or not, which means we're testing whether the string
        // did or did not begin with `::`.
        vector<string> parts = absl::StrSplit(shortName, "::");
        if (packageType) {
            // there's a little bit of a complexity here: we want
            // `non_forcing_is_a?("C::D", package: "A::B")` to be
            // looking for, more or less, `A::B::C::D`. We want this
            // _regardless_ of whether we're in packaged mode or not,
            // in fact: in non-packaged mode, we should treat this as
            // looking for `::A::B::C::D`, and in packaged mode, we're
            // looking for `A::B::C::D` nested inside the desugared
            // `PkgRegistry::Pkg_A_B` namespace.
            if (parts.front() == "") {
                if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                    e.setHeader("The string given to `{}` should not be an absolute constant reference if a "
                                "package name is also provided",
                                method);
                }
                return;
            }
            auto package = core::cast_type_nonnull<core::LiteralType>(packageType);
            auto name = package.asName(ctx).shortName(ctx);
            vector<string> pkgParts = absl::StrSplit(name, "::");
            // add the initial empty string to mimic the leading `::`
            if (ctx.state.packageDB().empty()) {
                pkgParts.insert(pkgParts.begin(), "");
            }
            // and now add the rest of `parts` to it
            pkgParts.insert(pkgParts.end(), parts.begin(), parts.end());
            // and then treat this new vector as the parts to walk over
            parts = move(pkgParts);
            // the path down below tries to find out if `packageType`
            // is null to find out whether it should look up a package
            // or not, so if we're not in package mode set
            // `packageType` to null
            if (ctx.state.packageDB().empty()) {
                packageType = nullptr;
            }
        }

        core::SymbolRef current;
        for (auto part : parts) {
            if (!current.exists()) {
                // First iteration
                if (!packageType) {
                    if (part != "") {
                        if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                            e.setHeader(
                                "The string given to `{}` must be an absolute constant reference that starts with `{}`",
                                method, "::");
                        }
                        return;
                    }
                    current = core::Symbols::root();
                    continue;
                } else {
                    auto package = core::cast_type_nonnull<core::LiteralType>(packageType);
                    auto packageName = package.asName(ctx);
                    auto mangledName = packageName.lookupMangledPackageName(ctx.state);
                    // if the mangled name doesn't exist, then this means probably there's no package named this
                    if (!mangledName.exists()) {
                        if (auto e = ctx.beginError(*packageLoc, core::errors::Resolver::LazyResolve)) {
                            e.setHeader("Unable to find package: `{}`", packageName.toString(ctx));
                        }
                        return;
                    }
                    current = core::Symbols::PackageRegistry().data(ctx)->findMember(ctx, mangledName);
                    if (!current.exists()) {
                        if (auto e = ctx.beginError(*packageLoc, core::errors::Resolver::LazyResolve)) {
                            e.setHeader("Unable to find package `{}`", packageName.toString(ctx));
                        }
                        return;
                    }
                }
            }

            auto member = ctx.state.lookupNameConstant(part);
            if (!member.exists()) {
                if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                    auto prettyCurrent = current == core::Symbols::root() ? "" : "::" + current.show(ctx);
                    auto pretty = fmt::format("{}::{}", prettyCurrent, part);
                    e.setHeader("Unable to resolve constant `{}`", pretty);
                }
                return;
            }

            auto newCurrent = current.data(ctx)->findMember(ctx, member);
            if (!newCurrent.exists()) {
                if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                    auto prettyCurrent = current == core::Symbols::root() ? "" : "::" + current.show(ctx);
                    auto pretty = fmt::format("{}::{}", prettyCurrent, part);
                    e.setHeader("Unable to resolve constant `{}`", pretty);
                }
                return;
            }
            current = newCurrent;
        }

        ENFORCE(current.exists(), "Loop invariant violated");

        if (!current.isClassOrModule()) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("The string given to `{}` must resolve to a class or module", method);
                e.addErrorLine(current.data(ctx)->loc(), "Resolved to this constant");
            }
            return;
        }
    }

    core::ClassOrModuleRef methodOwner(core::Context ctx) {
        core::ClassOrModuleRef owner = ctx.owner.enclosingClass(ctx);
        if (owner == core::Symbols::root()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }

public:
    ResolveTypeMembersAndFieldsWalk() {
        nestedBlockCounts.emplace_back(0);
    }

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        nestedBlockCounts.emplace_back(0);

        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        // If this is a class with no type members defined, resolve attached
        // class. Otherwise, it will be resolved once all type members have been
        // resolved as well.
        if (isGenericResolved(ctx, klass.symbol)) {
            todoAttachedClassItems_.emplace_back(ResolveAttachedClassItem{ctx.owner, klass.symbol, ctx.file});
        }

        return tree;
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        nestedBlockCounts.pop_back();
        return tree;
    }

    ast::ExpressionPtr preTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
        nestedBlockCounts.emplace_back(0);
        return tree;
    }

    ast::ExpressionPtr postTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
        nestedBlockCounts.pop_back();
        return tree;
    }

    ast::ExpressionPtr preTransformBlock(core::Context ctx, ast::ExpressionPtr tree) {
        ENFORCE_NO_TIMER(!nestedBlockCounts.empty());
        nestedBlockCounts.back() += 1;
        return tree;
    }

    ast::ExpressionPtr postTransformBlock(core::Context ctx, ast::ExpressionPtr tree) {
        ENFORCE_NO_TIMER(!nestedBlockCounts.empty());
        nestedBlockCounts.back() -= 1;
        return tree;
    }

    ast::ExpressionPtr preTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);
        switch (send.fun.rawId()) {
            case core::Names::typeAlias().rawId():
            case core::Names::typeMember().rawId():
            case core::Names::typeTemplate().rawId():
                break;

            default:
                extendClassOfDepth(send);
                return tree;
        }

        if (send.fun == core::Names::typeAlias()) {
            // don't track dependencies if this is some other method named `type_alias`
            if (!isT(send.recv)) {
                extendClassOfDepth(send);
                return tree;
            }
        }

        trackDependencies_ = true;
        classOfDepth_.clear();
        dependencies_.clear();
        return tree;
    }

    ast::ExpressionPtr postTransformConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
        auto &lit = ast::cast_tree_nonnull<ast::ConstantLit>(tree);

        if (trackDependencies_) {
            core::SymbolRef symbol = lit.symbol.data(ctx)->dealias(ctx);
            if (symbol == core::Symbols::T()) {
                return tree;
            }

            if (symbol.isClassOrModule()) {
                // This is the same as the implementation of T::Generic.[] in calls.cc
                // NOTE: the type members of these symbols will only be depended on during payload construction, as
                // after that their bounds will have been fully resolved.
                if (symbol == core::Symbols::T_Array()) {
                    symbol = core::Symbols::Array();
                } else if (symbol == core::Symbols::T_Hash()) {
                    symbol = core::Symbols::Hash();
                } else if (symbol == core::Symbols::T_Enumerable()) {
                    symbol = core::Symbols::Enumerable();
                } else if (symbol == core::Symbols::T_Enumerator()) {
                    symbol = core::Symbols::Enumerator();
                } else if (symbol == core::Symbols::T_Range()) {
                    symbol = core::Symbols::Range();
                } else if (symbol == core::Symbols::T_Set()) {
                    symbol = core::Symbols::Set();
                }

                // crawl up uses of `T.class_of` to find the right singleton symbol.
                // This is for cases like `T.class_of(T.class_of(A))`.
                for (auto it = classOfDepth_.rbegin(); it != classOfDepth_.rend() && *it; ++it) {
                    // ignore this as a potential dependency if the singleton
                    // doesn't exist -- this is an indication that there are no type
                    // members on the singleton.
                    symbol = symbol.data(ctx)->lookupSingletonClass(ctx);
                    if (!symbol.exists()) {
                        return tree;
                    }
                }

                if (!symbol.data(ctx)->typeMembers().empty()) {
                    dependencies_.emplace_back(symbol);
                }
            } else if (symbol.data(ctx)->isTypeAlias()) {
                dependencies_.emplace_back(symbol);
            }
        }

        return tree;
    }

    ast::ExpressionPtr postTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);

        switch (send.fun.rawId()) {
            case core::Names::typeMember().rawId():
            case core::Names::typeTemplate().rawId():
            case core::Names::typeAlias().rawId():
                trackDependencies_ = false;
                break;

            default:
                if (trackDependencies_) {
                    classOfDepth_.pop_back();
                }
                break;
        }

        if (auto *id = ast::cast_tree<ast::ConstantLit>(send.recv)) {
            if (id->symbol != core::Symbols::T() && id->symbol != core::Symbols::T_NonForcingConstants()) {
                return tree;
            }

            switch (send.fun.rawId()) {
                case core::Names::let().rawId():
                case core::Names::bind().rawId():
                case core::Names::uncheckedLet().rawId():
                case core::Names::assertType().rawId():
                case core::Names::cast().rawId(): {
                    if (send.args.size() < 2) {
                        return tree;
                    }

                    ResolveCastItem item;
                    item.file = ctx.file;

                    // Compute the containing class when translating the type,
                    // as there's a very good chance this has been called from a
                    // method context.
                    item.owner = ctx.owner.enclosingClass(ctx);

                    auto typeExpr = ast::MK::KeepForTypechecking(std::move(send.args[1]));
                    auto expr = std::move(send.args[0]);
                    auto cast =
                        ast::make_expression<ast::Cast>(send.loc, core::Types::todo(), std::move(expr), send.fun);
                    item.cast = ast::cast_tree<ast::Cast>(cast);
                    item.typeArg = &ast::cast_tree_nonnull<ast::Send>(typeExpr).args[0];

                    // We should be able to resolve simple casts immediately.
                    if (!tryResolveSimpleClassCastItem(ctx.withOwner(item.owner), item)) {
                        todoResolveCastItems_.emplace_back(move(item));
                    }

                    return ast::MK::InsSeq1(send.loc, move(typeExpr), move(cast));
                }
                case core::Names::revealType().rawId():
                case core::Names::absurd().rawId(): {
                    // These errors do not match up with our "upper error levels are super sets
                    // of errors from lower levels" claim. This is ONLY an error in lower levels.

                    string_view doWhat;
                    if (send.fun == core::Names::revealType()) {
                        doWhat = "reveal types";
                    } else if (send.fun == core::Names::absurd()) {
                        doWhat = "check exhaustiveness";
                    } else {
                        doWhat = "resolve strings to constants";
                    }

                    auto fun = fmt::format("T.{}", send.fun.show(ctx));
                    if (ctx.file.data(ctx).strictLevel <= core::StrictLevel::False) {
                        if (auto e = ctx.beginError(send.loc, core::errors::Resolver::RevealTypeInUntypedFile)) {
                            e.setHeader("`{}` can only {} in `{}` files (or higher)", fun, doWhat, "# typed: true");
                        }
                    }
                    return tree;
                }
                case core::Names::nonForcingIsA_p().rawId():
                    validateNonForcingIsA(ctx, send);
                    return tree;
                default:
                    return tree;
            }
        } else if (send.recv.isSelfReference()) {
            if (send.fun != core::Names::aliasMethod()) {
                return tree;
            }

            if (send.args.size() != 2) {
                return tree;
            }

            InlinedVector<core::NameRef, 2> args;
            for (auto &arg : send.args) {
                auto lit = ast::cast_tree<ast::Literal>(arg);
                if (lit == nullptr || !lit->isSymbol(ctx)) {
                    continue;
                }
                core::NameRef name = lit->asSymbol(ctx);

                args.emplace_back(name);
            }
            if (args.size() != 2) {
                return tree;
            }

            auto fromName = args[0];
            auto toName = args[1];
            auto owner = methodOwner(ctx);

            todoMethodAliasItems_.emplace_back(ResolveMethodAliasItem{
                ctx.file,
                owner,
                send.loc,
                send.args[1].loc(),
                toName,
                fromName,
            });
        }

        return tree;
    }

    ast::ExpressionPtr postTransformAssign(core::Context ctx, ast::ExpressionPtr tree) {
        auto &asgn = ast::cast_tree_nonnull<ast::Assign>(tree);
        if (handleFieldDeclaration(ctx, asgn)) {
            return tree;
        }

        auto *id = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (id == nullptr || !id->symbol.exists()) {
            return tree;
        }

        auto sym = id->symbol;
        auto data = sym.data(ctx);

        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        if (send && (data->isTypeAlias() || data->isTypeMember())) {
            ENFORCE(!data->isTypeMember() || send->recv.isSelfReference());

            // This is for a special case that happens with the generation of
            // reflection.rbi: it re-creates the type aliases of the payload,
            // without the knowledge that they are type aliases. The manifestation
            // of this, is that there are entries like:
            //
            // > module T
            // >   Boolean = T.let(nil, T.untyped)
            // > end
            if (data->isTypeAlias() && send->fun == core::Names::let()) {
                todoUntypedResultTypes_.emplace_back(sym);
                return tree;
            }

            ENFORCE(send->fun == core::Names::typeAlias() || send->fun == core::Names::typeMember() ||
                    send->fun == core::Names::typeTemplate());

            auto owner = sym.data(ctx)->owner;

            dependencies_.emplace_back(owner.data(ctx)->superClass());

            for (auto mixin : owner.data(ctx)->mixins()) {
                dependencies_.emplace_back(mixin);
            }

            todoAssigns_.emplace_back(ResolveAssignItem{ctx.owner, sym, send, std::move(dependencies_), ctx.file});
        } else if (data->isStaticField()) {
            ResolveStaticFieldItem job{ctx.file, sym.asFieldRef(), &asgn};
            auto resultType = tryResolveStaticField(ctx, job);
            if (resultType != core::Types::todo()) {
                todoResolveSimpleStaticFieldItems_.emplace_back(
                    ResolveSimpleStaticFieldItem{sym.asFieldRef(), resultType});
            } else {
                todoResolveStaticFieldItems_.emplace_back(move(job));
            }
        }

        if (send == nullptr) {
            return tree;
        }

        trackDependencies_ = false;
        dependencies_.clear();
        classOfDepth_.clear();

        return tree;
    }

    static vector<ast::ParsedFile> run(core::GlobalState &gs, vector<ast::ParsedFile> trees, WorkerPool &workers) {
        Timer timeit(gs.tracer(), "resolver.type_params");

        auto inputq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
        auto outputq = make_shared<BlockingBoundedQueue<ResolveTypeMembersAndFieldsResult>>(trees.size());
        for (auto &tree : trees) {
            inputq->push(move(tree), 1);
        }
        trees.clear();

        workers.multiplexJob("resolveTypeParamsWalk", [&gs, inputq, outputq]() -> void {
            Timer timeit(gs.tracer(), "resolveTypeParamsWalkWorker");
            ResolveTypeMembersAndFieldsWalk walk;
            ResolveTypeMembersAndFieldsResult output;
            ast::ParsedFile job;
            for (auto result = inputq->try_pop(job); !result.done(); result = inputq->try_pop(job)) {
                if (result.gotItem()) {
                    core::Context ctx(gs, core::Symbols::root(), job.file);
                    job.tree = ast::TreeMap::apply(ctx, walk, std::move(job.tree));
                    output.files.emplace_back(move(job));
                }
            }
            if (!output.files.empty()) {
                output.todoAssigns = move(walk.todoAssigns_);
                output.todoAttachedClassItems = move(walk.todoAttachedClassItems_);
                output.todoUntypedResultTypes = move(walk.todoUntypedResultTypes_);
                output.todoResolveCastItems = move(walk.todoResolveCastItems_);
                output.todoResolveFieldItems = move(walk.todoResolveFieldItems_);
                output.todoResolveStaticFieldItems = move(walk.todoResolveStaticFieldItems_);
                output.todoResolveSimpleStaticFieldItems = move(walk.todoResolveSimpleStaticFieldItems_);
                output.todoMethodAliasItems = move(walk.todoMethodAliasItems_);
                auto count = output.files.size();
                outputq->push(move(output), count);
            }
        });

        vector<ast::ParsedFile> combinedFiles;
        // The following items are not flattened; it'd be expensive to do so on large projects (they contain every
        // field/method alias/etc for the entire workspace!)
        vector<vector<ResolveAssignItem>> combinedTodoAssigns;
        vector<vector<ResolveAttachedClassItem>> combinedTodoAttachedClassItems;
        vector<vector<core::SymbolRef>> combinedTodoUntypedResultTypes;
        vector<vector<ResolveCastItem>> combinedTodoResolveCastItems;
        vector<vector<ResolveFieldItem>> combinedTodoResolveFieldItems;
        vector<vector<ResolveStaticFieldItem>> combinedTodoResolveStaticFieldItems;
        vector<vector<ResolveSimpleStaticFieldItem>> combinedTodoResolveSimpleStaticFieldItems;
        vector<vector<ResolveMethodAliasItem>> combinedTodoMethodAliasItems;

        {
            ResolveTypeMembersAndFieldsResult threadResult;
            for (auto result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
                 !result.done();
                 result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
                if (result.gotItem()) {
                    combinedFiles.insert(combinedFiles.end(), make_move_iterator(threadResult.files.begin()),
                                         make_move_iterator(threadResult.files.end()));
                    combinedTodoAssigns.emplace_back(move(threadResult.todoAssigns));
                    combinedTodoAttachedClassItems.emplace_back(move(threadResult.todoAttachedClassItems));
                    combinedTodoUntypedResultTypes.emplace_back(move(threadResult.todoUntypedResultTypes));
                    combinedTodoResolveCastItems.emplace_back(move(threadResult.todoResolveCastItems));
                    combinedTodoResolveFieldItems.emplace_back(move(threadResult.todoResolveFieldItems));
                    combinedTodoAttachedClassItems.emplace_back(move(threadResult.todoAttachedClassItems));
                    combinedTodoAttachedClassItems.emplace_back(move(threadResult.todoAttachedClassItems));
                    combinedTodoAttachedClassItems.emplace_back(move(threadResult.todoAttachedClassItems));
                    combinedTodoResolveStaticFieldItems.emplace_back(move(threadResult.todoResolveStaticFieldItems));
                    combinedTodoResolveSimpleStaticFieldItems.emplace_back(
                        move(threadResult.todoResolveSimpleStaticFieldItems));
                    combinedTodoMethodAliasItems.emplace_back(move(threadResult.todoMethodAliasItems));
                }
            }
        }

        // Put files into a consistent order for subsequent passes.
        fast_sort(combinedFiles, [](auto &a, auto &b) -> bool { return a.file.id() < b.file.id(); });

        for (auto &threadTodo : combinedTodoUntypedResultTypes) {
            for (auto sym : threadTodo) {
                sym.data(gs)->resultType = core::Types::untypedUntracked();
            }
        }

        vector<bool> resolvedAttachedClasses(gs.classAndModulesUsed());
        for (auto &threadTodo : combinedTodoAttachedClassItems) {
            for (auto &job : threadTodo) {
                core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                resolveAttachedClass(ctx, job.klass, resolvedAttachedClasses);
            }
        }

        // Resolve simple field declarations. Required so that `type_alias` can refer to an enum value type
        // (which is a static field). This is stronger than we need (we really only need the enum types)
        // but there's no particular reason to delay here.
        for (auto &threadTodo : combinedTodoResolveSimpleStaticFieldItems) {
            for (auto &job : threadTodo) {
                job.sym.data(gs)->resultType = job.resultType;
            }
        }

        // loop over any out-of-order type_member/type_alias references
        bool progress = true;
        while (progress && !combinedTodoAssigns.empty()) {
            progress = false;
            auto it = std::remove_if(
                combinedTodoAssigns.begin(), combinedTodoAssigns.end(), [&](vector<ResolveAssignItem> &threadTodos) {
                    auto origSize = threadTodos.size();
                    auto threadTodoIt =
                        std::remove_if(threadTodos.begin(), threadTodos.end(), [&](ResolveAssignItem &job) -> bool {
                            core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                            return resolveJob(ctx, job, resolvedAttachedClasses);
                        });
                    threadTodos.erase(threadTodoIt, threadTodos.end());
                    progress = progress || threadTodos.size() != origSize;
                    return threadTodos.empty();
                });
            combinedTodoAssigns.erase(it, combinedTodoAssigns.end());
        }

        // If there was a step with no progress, there's a cycle in the
        // type member/alias declarations. This is handled by reporting an error
        // at `typed: false`, and marking all of the involved type
        // members/aliases as T.untyped.
        if (!combinedTodoAssigns.empty()) {
            for (auto &threadTodos : combinedTodoAssigns) {
                for (auto &job : threadTodos) {
                    auto data = job.lhs.data(gs);

                    if (data->isTypeMember()) {
                        data->resultType = core::make_type<core::LambdaParam>(job.lhs.asTypeMemberRef(),
                                                                              core::Types::untypedUntracked(),
                                                                              core::Types::untypedUntracked());
                    } else {
                        data->resultType = core::Types::untypedUntracked();
                    }

                    if (auto e = gs.beginError(data->loc(), core::errors::Resolver::TypeMemberCycle)) {
                        auto flavor = data->isTypeAlias() ? "alias" : "member";
                        e.setHeader("Type {} `{}` is involved in a cycle", flavor, job.lhs.show(gs));
                    }
                }
            }
        }

        // Compute the resultType of all classes.
        computeExternalTypes(gs);

        // Resolve the remaining casts and fields.
        for (auto &threadTodos : combinedTodoResolveCastItems) {
            for (auto &job : threadTodos) {
                core::Context ctx(gs, job.owner, job.file);
                resolveCastItem(ctx, job);
            }
        }
        for (auto &threadTodos : combinedTodoResolveFieldItems) {
            for (auto &job : threadTodos) {
                core::MutableContext ctx(gs, job.owner, job.file);
                resolveField(ctx, job);
            }
        }
        for (auto &threadTodos : combinedTodoResolveStaticFieldItems) {
            for (auto &job : threadTodos) {
                core::Context ctx(gs, job.sym, job.file);
                if (auto resultType = resolveStaticField(ctx, job)) {
                    job.sym.data(gs)->resultType = resultType;
                }
            }
        }
        for (auto &threadTodos : combinedTodoMethodAliasItems) {
            for (auto &job : threadTodos) {
                core::MutableContext ctx(gs, job.owner, job.file);
                resolveMethodAlias(ctx, job);
            }
        }

        return combinedFiles;
    }
};

class ResolveSignaturesWalk {
public:
    struct ResolveSignatureJob {
        core::ClassOrModuleRef owner;
        ast::MethodDef *mdef;
        core::LocOffsets loc;
        ParsedSig sig;
    };

    struct OverloadedMethodSignature {
        core::LocOffsets loc;
        ParsedSig sig;
        // N.B.: Unused if method has multiple signatures but is not overloaded.
        vector<bool> argsToKeep;
    };

    struct ResolveMultiSignatureJob {
        core::ClassOrModuleRef owner;
        ast::MethodDef *mdef;
        bool isOverloaded;
        InlinedVector<OverloadedMethodSignature, 2> sigs;
    };

    struct ResolveFileSignatures {
        core::FileRef file;
        vector<ResolveSignatureJob> sigs;
        vector<ResolveMultiSignatureJob> multiSigs;
    };

    struct ResolveSignaturesWalkResult {
        vector<ResolveFileSignatures> fileSigs;
        vector<ast::ParsedFile> trees;
    };

    vector<ResolveSignatureJob> signatureJobs;
    vector<ResolveMultiSignatureJob> multiSignatureJobs;

private:
    static ast::Local const *getArgLocal(core::Context ctx, const core::ArgInfo &argSym, const ast::MethodDef &mdef,
                                         int pos, bool isOverloaded) {
        if (!isOverloaded) {
            return ast::MK::arg2Local(mdef.args[pos]);
        } else {
            // we cannot rely on method and symbol arguments being aligned, as method could have more arguments.
            // we roundtrip through original symbol that is stored in mdef.
            auto internalNameToLookFor = argSym.name;
            auto originalArgIt = absl::c_find_if(mdef.symbol.data(ctx)->arguments(),
                                                 [&](const auto &arg) { return arg.name == internalNameToLookFor; });
            ENFORCE(originalArgIt != mdef.symbol.data(ctx)->arguments().end());
            auto realPos = originalArgIt - mdef.symbol.data(ctx)->arguments().begin();
            return ast::MK::arg2Local(mdef.args[realPos]);
        }
    }

    static void handleAbstractMethod(core::Context ctx, ast::MethodDef &mdef) {
        if (mdef.symbol.data(ctx)->isAbstract()) {
            if (!ast::isa_tree<ast::EmptyTree>(mdef.rhs)) {
                if (auto e = ctx.beginError(mdef.rhs.loc(), core::errors::Resolver::AbstractMethodWithBody)) {
                    e.setHeader("Abstract methods must not contain any code in their body");
                    e.replaceWith("Delete the body", core::Loc(ctx.file, mdef.rhs.loc()), "");
                }

                mdef.rhs = ast::MK::EmptyTree();
            }
            if (!mdef.symbol.enclosingClass(ctx).data(ctx)->isClassOrModuleAbstract()) {
                if (auto e = ctx.beginError(mdef.loc, core::errors::Resolver::AbstractMethodOutsideAbstract)) {
                    e.setHeader("Before declaring an abstract method, you must mark your class/module "
                                "as abstract using `abstract!` or `interface!`");
                }
            }

            // Rewrite the empty body of the abstract method to forward all arguments to `super`, mirroring the
            // behavior of the runtime.
            ast::Send::ARGS_store args;

            auto argIdx = -1;
            auto numPosArgs = 0;
            for (auto &arg : mdef.args) {
                ++argIdx;

                const ast::Local *local = nullptr;
                if (auto *opt = ast::cast_tree<ast::OptionalArg>(arg)) {
                    local = ast::cast_tree<ast::Local>(opt->expr);
                } else {
                    local = ast::cast_tree<ast::Local>(arg);
                }

                auto &info = mdef.symbol.data(ctx)->arguments()[argIdx];
                if (info.flags.isKeyword) {
                    args.emplace_back(ast::MK::Symbol(local->loc, info.name));
                    args.emplace_back(local->deepCopy());
                } else if (info.flags.isRepeated || info.flags.isBlock) {
                    // Explicitly skip for now.
                    // Involves synthesizing a call to callWithSplat, callWithBlock, or
                    // callWithSplatAndBlock
                } else {
                    args.emplace_back(local->deepCopy());
                    ++numPosArgs;
                }
            }

            auto self = ast::MK::Self(mdef.loc);
            mdef.rhs = ast::MK::Send(mdef.loc, std::move(self), core::Names::super(), numPosArgs, std::move(args));
        } else if (mdef.symbol.enclosingClass(ctx).data(ctx)->isClassOrModuleInterface()) {
            if (auto e = ctx.beginError(mdef.loc, core::errors::Resolver::ConcreteMethodInInterface)) {
                e.setHeader("All methods in an interface must be declared abstract");
            }
        }
    }

    static void fillInInfoFromSig(core::MutableContext ctx, core::MethodRef method, core::LocOffsets exprLoc,
                                  ParsedSig &sig, bool isOverloaded, const ast::MethodDef &mdef) {
        ENFORCE(isOverloaded || mdef.symbol == method);
        ENFORCE(isOverloaded || method.data(ctx)->arguments().size() == mdef.args.size());

        if (!sig.seen.returns && !sig.seen.void_) {
            if (auto e = ctx.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`: No return type specified. Specify one with .returns()", "sig");
            }
        }
        if (sig.seen.returns && sig.seen.void_) {
            if (auto e = ctx.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`: Don't use both .returns() and .void", "sig");
            }
        }

        if (sig.seen.abstract) {
            method.data(ctx)->setAbstract();
        }
        if (sig.seen.incompatibleOverride) {
            method.data(ctx)->setIncompatibleOverride();
        }
        if (!sig.typeArgs.empty()) {
            method.data(ctx)->setGenericMethod();
            for (auto &typeSpec : sig.typeArgs) {
                if (typeSpec.type) {
                    auto name = ctx.state.freshNameUnique(core::UniqueNameKind::TypeVarName, typeSpec.name, 1);
                    auto sym = ctx.state.enterTypeArgument(typeSpec.loc, method, name, core::Variance::CoVariant);
                    auto asTypeVar = core::cast_type<core::TypeVar>(typeSpec.type);
                    ENFORCE(asTypeVar != nullptr);
                    asTypeVar->sym = sym;
                    sym.data(ctx)->resultType = typeSpec.type;
                }
            }
        }
        if (sig.seen.overridable) {
            method.data(ctx)->setOverridable();
        }
        if (sig.seen.override_) {
            method.data(ctx)->setOverride();
        }
        if (sig.seen.final) {
            method.data(ctx)->setFinalMethod();
        }
        if (sig.seen.bind) {
            method.data(ctx)->setReBind(sig.bind);
        }

        auto methodInfo = method.data(ctx);

        // Is this a signature for a method defined with argument forwarding syntax?
        if (methodInfo->arguments().size() >= 3) {
            // To match, the definition must have been desugared with at least 3 parameters named
            // `<fwd-args>`, `<fwd-kwargs>` and `<fwd-block>`
            auto len = methodInfo->arguments().size();
            auto l1 = getArgLocal(ctx, methodInfo->arguments()[len - 3], mdef, len - 3, isOverloaded)->localVariable;
            auto l2 = getArgLocal(ctx, methodInfo->arguments()[len - 2], mdef, len - 2, isOverloaded)->localVariable;
            auto l3 = getArgLocal(ctx, methodInfo->arguments()[len - 1], mdef, len - 1, isOverloaded)->localVariable;
            if (l1._name == core::Names::fwdArgs() && l2._name == core::Names::fwdKwargs() &&
                l3._name == core::Names::fwdBlock()) {
                if (auto e = ctx.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Unsupported `{}` for argument forwarding syntax", "sig");
                    e.addErrorLine(methodInfo->loc(), "Method declares argument forwarding here");
                    e.addErrorNote("Rewrite the method as `def {}(*args, **kwargs, &blk)` to use a signature",
                                   method.show(ctx));
                }
                return;
            }
        }

        // Get the parameters order from the signature
        vector<ParsedSig::ArgSpec> sigParams(sig.argTypes);

        vector<ast::Local const *> defParams; // Parameters order from the method declaration
        bool seenOptional = false;

        methodInfo->resultType = sig.returns;
        int i = -1;
        for (auto &arg : methodInfo->arguments()) {
            ++i;
            auto local = getArgLocal(ctx, arg, mdef, i, isOverloaded);
            auto treeArgName = local->localVariable._name;
            ENFORCE(local != nullptr);

            // Check that optional keyword parameters are after all the required ones
            bool isKwd = arg.flags.isKeyword;
            bool isReq = !arg.flags.isBlock && !arg.flags.isRepeated && !arg.flags.isDefault;
            if (isKwd && !isReq) {
                seenOptional = true;
            } else if (isKwd && seenOptional && isReq) {
                if (auto e = ctx.state.beginError(arg.loc, core::errors::Resolver::BadParameterOrdering)) {
                    e.setHeader("Malformed `{}`. Required parameter `{}` must be declared before all the optional ones",
                                "sig", treeArgName.show(ctx));
                    e.addErrorLine(core::Loc(ctx.file, exprLoc), "Signature");
                }
            }

            defParams.push_back(local);

            auto spec = absl::c_find_if(sig.argTypes, [&](const auto &spec) { return spec.name == treeArgName; });

            if (spec != sig.argTypes.end()) {
                ENFORCE(spec->type != nullptr);
                arg.type = std::move(spec->type);
                arg.loc = spec->loc;
                arg.rebind = spec->rebind;
                sig.argTypes.erase(spec);
            } else {
                if (arg.type == nullptr) {
                    arg.type = core::Types::untyped(ctx, method);
                }

                // We silence the "type not specified" error when a sig does not mention the synthesized block arg.
                bool isBlkArg = arg.name == core::Names::blkArg();
                if (!isOverloaded && !isBlkArg && (sig.seen.params || sig.seen.returns || sig.seen.void_)) {
                    // Only error if we have any types
                    if (auto e = ctx.state.beginError(arg.loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`. Type not specified for argument `{}`", "sig",
                                    treeArgName.show(ctx));
                        e.addErrorLine(core::Loc(ctx.file, exprLoc), "Signature");
                    }
                }
            }

            if (isOverloaded && arg.flags.isKeyword) {
                if (auto e = ctx.state.beginError(arg.loc, core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Malformed `{}`. Overloaded functions cannot have keyword arguments:  `{}`", "sig",
                                treeArgName.show(ctx));
                }
            }
        }

        for (const auto &spec : sig.argTypes) {
            if (auto e = ctx.state.beginError(spec.loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Unknown argument name `{}`", spec.name.show(ctx));
            }
        }

        // Check params ordering match between signature and definition
        if (sig.argTypes.empty()) {
            int j = 0;
            for (const auto &spec : sigParams) {
                auto *param = defParams[j];
                auto sname = spec.name;
                auto dname = param->localVariable._name;
                // TODO(jvilk): Do we need to check .show? Typically NameRef equality is equal to string equality.
                if (sname != dname && sname.show(ctx) != dname.show(ctx)) {
                    if (auto e = ctx.beginError(param->loc, core::errors::Resolver::BadParameterOrdering)) {
                        e.setHeader("Bad parameter ordering for `{}`, expected `{}` instead", dname.show(ctx),
                                    sname.show(ctx));
                        e.addErrorLine(spec.loc, "Expected index in signature:");
                    }
                }
                j++;
            }
        }

        // Later passes are going to separate the sig and the method definition.
        // Record some information in the sig call itself so that we can reassociate
        // them later.
        //
        // Note that the sig still needs to send to a method called "sig" so that
        // code completion in LSP works.  We change the receiver, below, so that
        // sigs that don't pass through here still reflect the user's intent.
        auto *send = sig.origSend;
        auto &origArgs = send->args;
        auto *self = ast::cast_tree<ast::Local>(send->args[0]);
        if (self == nullptr) {
            return;
        }

        // We distinguish "user-written" sends by checking for self.
        // T::Sig::WithoutRuntime.sig wouldn't have any runtime effect that we need
        // to record later.
        if (self->localVariable != core::LocalVariable::selfVariable()) {
            return;
        }

        auto *cnst = ast::cast_tree<ast::ConstantLit>(send->recv);
        ENFORCE(cnst != nullptr, "sig send receiver must be a ConstantLit if we got a ParsedSig from the send");

        cnst->symbol = core::Symbols::Sorbet_Private_Static_ResolvedSig();

        origArgs.emplace_back(mdef.flags.isSelfMethod ? ast::MK::True(send->loc) : ast::MK::False(send->loc));
        origArgs.emplace_back(ast::MK::Symbol(send->loc, method.data(ctx)->name));
        send->numPosArgs += 2;
    }

    // Force errors from any signatures that didn't attach to methods.
    // `lastSigs` will always be empty after this function is called.
    void processLeftoverSigs(core::Context ctx, InlinedVector<ast::Send *, 1> &lastSigs) {
        if (!lastSigs.empty()) {
            // These sigs won't have been parsed, as there was no methods to
            // attach them to -- parse them here manually to force any errors.
            for (auto sig : lastSigs) {
                auto allowSelfType = true;
                auto allowRebind = false;
                auto allowTypeMember = true;
                TypeSyntax::parseSig(
                    ctx, *sig, nullptr,
                    TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, core::Symbols::untyped()});
            }

            if (auto e = ctx.beginError(lastSigs[0]->loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`. No method def following it", "sig");
            }

            lastSigs.clear();
        }
    }

    void processClassBody(core::Context ctx, ast::ClassDef &klass) {
        InlinedVector<ast::Send *, 1> lastSigs;
        for (auto &stat : klass.rhs) {
            processStatement(ctx, stat, lastSigs);
        }

        processLeftoverSigs(ctx, lastSigs);

        auto toRemove = remove_if(klass.rhs.begin(), klass.rhs.end(),
                                  [](ast::ExpressionPtr &stat) -> bool { return stat == nullptr; });
        klass.rhs.erase(toRemove, klass.rhs.end());
    }

    void processInSeq(core::Context ctx, ast::InsSeq &seq) {
        InlinedVector<ast::Send *, 1> lastSigs;

        // Explicitly check in the contxt of the class, not <static-init>
        auto classCtx = ctx.withOwner(ctx.owner.enclosingClass(ctx));

        for (auto &stat : seq.stats) {
            processStatement(classCtx, stat, lastSigs);
        }
        if (!ast::isa_tree<ast::EmptyTree>(seq.expr)) {
            processStatement(classCtx, seq.expr, lastSigs);
        }

        processLeftoverSigs(classCtx, lastSigs);

        auto toRemove = remove_if(seq.stats.begin(), seq.stats.end(),
                                  [](ast::ExpressionPtr &stat) -> bool { return stat == nullptr; });
        seq.stats.erase(toRemove, seq.stats.end());
    }

    ParsedSig parseSig(core::Context ctx, core::ClassOrModuleRef sigOwner, ast::Send &send, ast::MethodDef &mdef) {
        auto allowSelfType = true;
        auto allowRebind = false;
        auto allowTypeMember = true;
        return TypeSyntax::parseSig(ctx.withOwner(sigOwner), send, nullptr,
                                    TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, mdef.symbol});
    }

    void processStatement(core::Context ctx, ast::ExpressionPtr &stat, InlinedVector<ast::Send *, 1> &lastSigs) {
        typecase(
            stat,

            [&](ast::Send &send) {
                if (TypeSyntax::isSig(ctx, send)) {
                    if (!lastSigs.empty()) {
                        if (!ctx.permitOverloadDefinitions(ctx.file)) {
                            if (auto e = ctx.beginError(lastSigs[0]->loc, core::errors::Resolver::OverloadNotAllowed)) {
                                e.setHeader("Unused type annotation. No method def before next annotation");
                                e.addErrorLine(core::Loc(ctx.file, send.loc),
                                               "Type annotation that will be used instead");
                            }
                        }
                    }

                    lastSigs.emplace_back(&send);
                    return;
                }

                if (send.args.size() == 1 &&
                    (send.fun == core::Names::public_() || send.fun == core::Names::private_() ||
                     send.fun == core::Names::privateClassMethod() || send.fun == core::Names::protected_())) {
                    processStatement(ctx, send.args[0], lastSigs);
                    return;
                }
            },

            [&](ast::MethodDef &mdef) {
                if (debug_mode) {
                    bool hasSig = !lastSigs.empty();
                    bool rewriten = mdef.flags.isRewriterSynthesized;
                    bool isRBI = ctx.file.data(ctx).isRBI();
                    if (hasSig) {
                        categoryCounterInc("method.sig", "true");
                    } else {
                        categoryCounterInc("method.sig", "false");
                    }
                    if (rewriten) {
                        categoryCounterInc("method.dsl", "true");
                    } else {
                        categoryCounterInc("method.dsl", "false");
                    }
                    if (isRBI) {
                        categoryCounterInc("method.rbi", "true");
                    } else {
                        categoryCounterInc("method.rbi", "false");
                    }
                    if (hasSig && !isRBI && !rewriten) {
                        counterInc("types.sig.human");
                    }
                }

                if (!lastSigs.empty()) {
                    if (ctx.file.data(ctx).originalSigil == core::StrictLevel::None &&
                        !lastSigs.front()->flags.isRewriterSynthesized) {
                        auto loc = lastSigs.front()->loc;
                        if (auto e = ctx.beginError(loc, core::errors::Resolver::SigInFileWithoutSigil)) {
                            e.setHeader("To use `{}`, this file must declare an explicit `{}` sigil (found: "
                                        "none). If you're not sure which one to use, start with `{}`",
                                        "sig", "# typed:", "# typed: false");
                        }
                    }

                    // process signatures in the context of either the current
                    // class, or the current singleton class, depending on if
                    // the current method is a self method.
                    core::ClassOrModuleRef sigOwner;
                    if (mdef.flags.isSelfMethod) {
                        sigOwner = ctx.owner.data(ctx)->lookupSingletonClass(ctx);
                        // namer ensures that all method owners are defined.
                        ENFORCE_NO_TIMER(sigOwner.exists());
                    } else {
                        sigOwner = ctx.owner.asClassOrModuleRef();
                    }

                    if (lastSigs.size() == 1) {
                        auto &lastSig = lastSigs.front();
                        signatureJobs.emplace_back(ResolveSignatureJob{ctx.owner.asClassOrModuleRef(), &mdef,
                                                                       lastSig->loc,
                                                                       parseSig(ctx, sigOwner, *lastSig, mdef)});
                    } else {
                        bool isOverloaded = ctx.permitOverloadDefinitions(ctx.file);
                        InlinedVector<OverloadedMethodSignature, 2> sigs;
                        for (auto &lastSig : lastSigs) {
                            auto sig = parseSig(ctx, sigOwner, *lastSig, mdef);
                            vector<bool> argsToKeep;
                            if (isOverloaded) {
                                for (auto &argTree : mdef.args) {
                                    const auto local = ast::MK::arg2Local(argTree);
                                    auto treeArgName = local->localVariable._name;
                                    ENFORCE(local != nullptr);
                                    argsToKeep.emplace_back(absl::c_find_if(sig.argTypes, [&](auto &spec) {
                                                                return spec.name == treeArgName;
                                                            }) != sig.argTypes.end());
                                }
                            }
                            sigs.emplace_back(OverloadedMethodSignature{lastSig->loc, move(sig), move(argsToKeep)});
                        }

                        multiSignatureJobs.emplace_back(ResolveMultiSignatureJob{ctx.owner.asClassOrModuleRef(), &mdef,
                                                                                 isOverloaded, std::move(sigs)});
                    }

                    lastSigs.clear();
                } else {
                    handleAbstractMethod(ctx, mdef);
                }
            },
            [&](const ast::ClassDef &cdef) {
                // Leave in place
            },

            [&](const ast::EmptyTree &e) { stat.reset(nullptr); },

            [&](const ast::ExpressionPtr &e) {});
    }

public:
    static void resolveMultiSignatureJob(core::MutableContext ctx, ResolveMultiSignatureJob &job) {
        auto &sigs = job.sigs;
        auto &mdef = *job.mdef;
        ENFORCE_NO_TIMER(sigs.size() > 1);

        prodCounterInc("types.sig.count");

        bool isOverloaded = job.isOverloaded;
        auto originalName = mdef.symbol.data(ctx)->name;
        if (isOverloaded) {
            ctx.state.mangleRenameSymbol(mdef.symbol, originalName);
        }

        int i = -1;
        for (auto &sig : sigs) {
            i++;
            core::MethodRef overloadSym;
            if (isOverloaded) {
                overloadSym = ctx.state.enterNewMethodOverload(core::Loc(ctx.file, sig.loc), mdef.symbol, originalName,
                                                               i, sig.argsToKeep);
                overloadSym.data(ctx)->setMethodVisibility(mdef.symbol.data(ctx)->methodVisibility());
                if (i != sigs.size() - 1) {
                    overloadSym.data(ctx)->setOverloaded();
                }
            } else {
                overloadSym = mdef.symbol;
            }
            fillInInfoFromSig(ctx, overloadSym, sig.loc, sig.sig, isOverloaded, mdef);
        }
        handleAbstractMethod(ctx, mdef);
    }
    static void resolveSignatureJob(core::MutableContext ctx, ResolveSignatureJob &job) {
        prodCounterInc("types.sig.count");
        auto &mdef = *job.mdef;
        bool isOverloaded = false;
        fillInInfoFromSig(ctx, mdef.symbol, job.loc, job.sig, isOverloaded, mdef);
        handleAbstractMethod(ctx, mdef);
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        processClassBody(ctx.withOwner(klass.symbol), klass);
        return tree;
    }

    ast::ExpressionPtr postTransformInsSeq(core::Context ctx, ast::ExpressionPtr tree) {
        processInSeq(ctx, ast::cast_tree_nonnull<ast::InsSeq>(tree));
        return tree;
    }
};

class ResolveSanityCheckWalk {
public:
    ast::ExpressionPtr postTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        ENFORCE(original.symbol != core::Symbols::todo(), "These should have all been resolved: {}",
                tree.toString(ctx));
        if (original.symbol == core::Symbols::root()) {
            ENFORCE(ctx.state.lookupStaticInitForFile(core::Loc(ctx.file, original.loc)).exists());
        } else {
            ENFORCE(ctx.state.lookupStaticInitForClass(original.symbol).exists());
        }
        return tree;
    }
    ast::ExpressionPtr postTransformMethodDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        ENFORCE(original.symbol != core::Symbols::todoMethod(), "These should have all been resolved: {}",
                tree.toString(ctx));
        return tree;
    }
    ast::ExpressionPtr postTransformUnresolvedConstantLit(core::MutableContext ctx, ast::ExpressionPtr tree) {
        ENFORCE(false, "These should have all been removed: {}", tree.toString(ctx));
        return tree;
    }
    ast::ExpressionPtr postTransformUnresolvedIdent(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::UnresolvedIdent>(tree);
        ENFORCE(original.kind != ast::UnresolvedIdent::Kind::Local, "{} should have been removed by local_vars",
                tree.toString(ctx));
        return tree;
    }
    ast::ExpressionPtr postTransformConstantLit(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ConstantLit>(tree);
        ENFORCE(ResolveConstantsWalk::isAlreadyResolved(ctx, original));
        return tree;
    }
};
}; // namespace

ast::ParsedFilesOrCancelled Resolver::run(core::GlobalState &gs, vector<ast::ParsedFile> trees, WorkerPool &workers) {
    const auto &epochManager = *gs.epochManager;
    trees = ResolveConstantsWalk::resolveConstants(gs, std::move(trees), workers);
    if (epochManager.wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled();
    }
    finalizeAncestors(gs);
    if (epochManager.wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled();
    }
    finalizeSymbols(gs);
    if (epochManager.wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled();
    }
    trees = ResolveTypeMembersAndFieldsWalk::run(gs, std::move(trees), workers);
    if (epochManager.wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled();
    }

    auto result = resolveSigs(gs, std::move(trees), workers);
    if (!result.hasResult()) {
        return result;
    }
    sanityCheck(gs, result.result());

    return result;
}

ast::ParsedFilesOrCancelled Resolver::resolveSigs(core::GlobalState &gs, vector<ast::ParsedFile> trees,
                                                  WorkerPool &workers) {
    Timer timeit(gs.tracer(), "resolver.sigs_vars_and_flatten");
    auto inputq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(trees.size());
    auto outputq = make_shared<BlockingBoundedQueue<ResolveSignaturesWalk::ResolveSignaturesWalkResult>>(trees.size());

    for (auto &tree : trees) {
        inputq->push(move(tree), 1);
    }

    workers.multiplexJob("resolveSignaturesWalk", [&gs, inputq, outputq]() -> void {
        ResolveSignaturesWalk walk;
        ResolveSignaturesWalk::ResolveSignaturesWalkResult output;
        ast::ParsedFile job;
        for (auto result = inputq->try_pop(job); !result.done(); result = inputq->try_pop(job)) {
            if (result.gotItem()) {
                core::Context ctx(gs, core::Symbols::root(), job.file);
                job.tree = ast::ShallowMap::apply(ctx, walk, std::move(job.tree));
                if (!walk.signatureJobs.empty() || !walk.multiSignatureJobs.empty()) {
                    output.fileSigs.emplace_back(ResolveSignaturesWalk::ResolveFileSignatures{
                        job.file, move(walk.signatureJobs), move(walk.multiSignatureJobs)});
                }
                output.trees.emplace_back(move(job));
            }
        }
        if (!output.trees.empty()) {
            auto count = output.trees.size();
            outputq->push(move(output), count);
        }
    });

    vector<ResolveSignaturesWalk::ResolveFileSignatures> combinedFileJobs;
    vector<ast::ParsedFile> combinedTrees;
    {
        ResolveSignaturesWalk::ResolveSignaturesWalkResult threadResult;
        for (auto result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                combinedTrees.insert(combinedTrees.end(), make_move_iterator(threadResult.trees.begin()),
                                     make_move_iterator(threadResult.trees.end()));
                combinedFileJobs.insert(combinedFileJobs.end(), make_move_iterator(threadResult.fileSigs.begin()),
                                        make_move_iterator(threadResult.fileSigs.end()));
            }
        }
    }

    // We need to define sigs in a stable order since, when there are conflicting sigs in multiple RBI files, the last
    // sig 'wins'.
    fast_sort(
        combinedFileJobs,
        [&](const ResolveSignaturesWalk::ResolveFileSignatures &left,
            const ResolveSignaturesWalk::ResolveFileSignatures &right) -> bool { return left.file < right.file; });

    {
        Timer timeit(gs.tracer(), "resolver.resolve_sigs");
        for (auto &file : combinedFileJobs) {
            for (auto &job : file.sigs) {
                core::MutableContext ctx(gs, job.owner, file.file);
                ResolveSignaturesWalk::resolveSignatureJob(ctx, job);
            }
            for (auto &job : file.multiSigs) {
                core::MutableContext ctx(gs, job.owner, file.file);
                ResolveSignaturesWalk::resolveMultiSignatureJob(ctx, job);
            }
        }
    }

    return move(combinedTrees);
}

void Resolver::sanityCheck(core::GlobalState &gs, vector<ast::ParsedFile> &trees) {
    if (debug_mode) {
        Timer timeit(gs.tracer(), "resolver.sanity_check");
        ResolveSanityCheckWalk sanity;
        for (auto &tree : trees) {
            core::MutableContext ctx(gs, core::Symbols::root(), tree.file);
            tree.tree = ast::TreeMap::apply(ctx, sanity, std::move(tree.tree));
        }
    }
}

ast::ParsedFilesOrCancelled Resolver::runIncremental(core::GlobalState &gs, vector<ast::ParsedFile> trees) {
    auto workers = WorkerPool::create(0, gs.tracer());
    trees = ResolveConstantsWalk::resolveConstants(gs, std::move(trees), *workers);
    // NOTE: Linearization does not need to be recomputed as we do not mutate mixins() during incremental resolve.
    DEBUG_ONLY(for (auto i = 1; i < gs.classAndModulesUsed(); i++) {
        core::ClassOrModuleRef sym(gs, i);
        // If class is not marked as 'linearization computed', then we added a mixin to it since the last slow path.
        ENFORCE_NO_TIMER(sym.data(gs)->isClassOrModuleLinearizationComputed(), sym.toString(gs));
    })
    trees = ResolveTypeMembersAndFieldsWalk::run(gs, std::move(trees), *workers);
    auto result = resolveSigs(gs, std::move(trees), *workers);
    if (!result.hasResult()) {
        return result;
    }
    sanityCheck(gs, result.result());
    // This check is FAR too slow to run on large codebases, especially with sanitizers on.
    // But it can be super useful to uncomment when debugging certain issues.
    // ctx.state.sanityCheck();

    return result;
}

vector<ast::ParsedFile> Resolver::runConstantResolution(core::GlobalState &gs, vector<ast::ParsedFile> trees,
                                                        WorkerPool &workers) {
    trees = ResolveConstantsWalk::resolveConstants(gs, std::move(trees), workers);
    sanityCheck(gs, trees);

    return trees;
}

} // namespace sorbet::resolver
