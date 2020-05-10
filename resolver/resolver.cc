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
        ast::ConstantLit *out;
        bool resolutionFailed = false;

        ResolutionItem() = default;
        ResolutionItem(ResolutionItem &&rhs) noexcept = default;
        ResolutionItem &operator=(ResolutionItem &&rhs) noexcept = default;

        ResolutionItem(const ResolutionItem &rhs) = delete;
        const ResolutionItem &operator=(const ResolutionItem &rhs) = delete;
    };
    struct AncestorResolutionItem {
        ast::ConstantLit *ancestor;
        core::SymbolRef klass;
        core::FileRef file;

        bool isSuperclass; // true if superclass, false for mixin

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
        ast::TreePtr *rhs;

        TypeAliasResolutionItem(TypeAliasResolutionItem &&) noexcept = default;
        TypeAliasResolutionItem &operator=(TypeAliasResolutionItem &&rhs) noexcept = default;

        TypeAliasResolutionItem(const TypeAliasResolutionItem &) = delete;
        const TypeAliasResolutionItem &operator=(const TypeAliasResolutionItem &) = delete;
    };

    vector<ResolutionItem> todo_;
    vector<AncestorResolutionItem> todoAncestors_;
    vector<ClassAliasResolutionItem> todoClassAliases_;
    vector<TypeAliasResolutionItem> todoTypeAliases_;

    static core::SymbolRef resolveLhs(core::Context ctx, shared_ptr<Nesting> nesting, core::NameRef name) {
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

        ast::TreePtr postTransformConstantLit(core::Context ctx, ast::TreePtr tree) {
            auto &original = ast::ref_tree<ast::ConstantLit>(tree);
            seenUnresolved |= !isAlreadyResolved(ctx, original);
            return tree;
        };
    };

    static bool isFullyResolved(core::Context ctx, const ast::TreePtr &expression) {
        ResolutionChecker checker;
        ast::TreePtr dummy(expression.get());
        dummy = ast::TreeMap::apply(ctx, checker, std::move(dummy));
        ENFORCE(dummy == expression);
        dummy.release();
        return !checker.seenUnresolved;
    }

    static core::SymbolRef resolveConstant(core::Context ctx, shared_ptr<Nesting> nesting,
                                           const ast::UnresolvedConstantLit &c, bool &resolutionFailed) {
        if (ast::isa_tree<ast::EmptyTree>(c.scope)) {
            core::SymbolRef result = resolveLhs(ctx, nesting, c.cnst);
            return result;
        }
        if (auto *id = ast::cast_tree_const<ast::ConstantLit>(c.scope)) {
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
        auto &original = ast::ref_tree<ast::UnresolvedConstantLit>(job.out->original);

        auto resolved = resolveConstant(ctx.withOwner(job.scope->scope), job.scope, original, job.resolutionFailed);
        if (resolved.exists() && resolved.data(ctx)->isTypeAlias()) {
            if (resolved.data(ctx)->resultType == nullptr) {
                // This is actually a use-site error, but we limit ourselves to emitting it once by checking resultType
                auto loc = resolved.data(ctx)->loc();
                if (auto e = ctx.state.beginError(loc, core::errors::Resolver::RecursiveTypeAlias)) {
                    e.setHeader("Unable to resolve right hand side of type alias `{}`", resolved.data(ctx)->show(ctx));
                    e.addErrorLine(core::Loc(ctx.file, job.out->original->loc), "Type alias used here");
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
            if (auto e = ctx.beginError(job.out->original->loc, core::errors::Resolver::StubConstant)) {
                e.setHeader("Unable to resolve constant `{}`", original.cnst.show(ctx));

                auto suggestScope = job.out->resolutionScopes.front();
                if (customAutogenError) {
                    e.addErrorSection(
                        core::ErrorSection("If this constant is generated by Autogen, you "
                                           "may need to re-generate the .rbi. Try running:\n"
                                           "  scripts/bin/remote-script sorbet/shim_generation/autogen.rb"));
                } else if (suggestDidYouMean && suggestScope.exists() && suggestScope.data(ctx)->isClassOrModule()) {
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
        auto &original = ast::ref_tree<ast::UnresolvedConstantLit>(job.out->original);
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
        core::SymbolRef enclosingTypeMember;
        core::SymbolRef enclosingClass = job.lhs.data(ctx)->enclosingClass(ctx);
        while (enclosingClass != core::Symbols::root()) {
            auto typeMembers = enclosingClass.data(ctx)->typeMembers();
            if (!typeMembers.empty()) {
                enclosingTypeMember = typeMembers[0];
                break;
            }
            enclosingClass = enclosingClass.data(ctx)->owner.data(ctx)->enclosingClass(ctx);
        }
        auto &rhs = *job.rhs;
        if (enclosingTypeMember.exists()) {
            if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::TypeAliasInGenericClass)) {
                e.setHeader("Type aliases are not allowed in generic classes");
                e.addErrorLine(enclosingTypeMember.data(ctx)->loc(), "Here is enclosing generic member");
            }
            job.lhs.data(ctx)->resultType = core::Types::untyped(ctx, job.lhs);
            return true;
        }
        if (isFullyResolved(ctx, rhs)) {
            // this todo will be resolved during ResolveTypeMembersWalk below
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
                e.replaceWith("Declare as type alias", core::Loc(ctx.file, it.rhs->loc), "T.type_alias {{{}}}",
                              core::Loc(ctx.file, it.rhs->loc).source(ctx));
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

        core::SymbolRef uaSym =
            ctx.state.enterMethodSymbol(core::Loc::none(), item.klass, core::Names::unresolvedAncestors());
        core::TypePtr resultType = uaSym.data(ctx)->resultType;
        if (!resultType) {
            uaSym.data(ctx)->resultType = core::TupleType::build(ctx, {ancestorType});
        } else if (auto tt = core::cast_type<core::TupleType>(resultType.get())) {
            tt->elems.push_back(ancestorType);
        } else {
            ENFORCE(false);
        }
    }

    static core::SymbolRef stubSymbolForAncestor(const AncestorResolutionItem &item) {
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

        if (!resolved.data(ctx)->isClassOrModule()) {
            if (!lastRun) {
                return false;
            }
            if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::DynamicSuperclass)) {
                e.setHeader("Superclasses and mixins may only use class aliases like `{}`", "A = Integer");
            }
            resolved = stubSymbolForAncestor(job);
        }

        if (resolved == job.klass) {
            if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::CircularDependency)) {
                e.setHeader("Circular dependency: `{}` is a parent of itself", job.klass.data(ctx)->show(ctx));
                e.addErrorLine(resolved.data(ctx)->loc(), "Class definition");
            }
            resolved = stubSymbolForAncestor(job);
        } else if (resolved.data(ctx)->derivesFrom(ctx, job.klass)) {
            if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::CircularDependency)) {
                e.setHeader("Circular dependency: `{}` and `{}` are declared as parents of each other",
                            job.klass.data(ctx)->show(ctx), resolved.data(ctx)->show(ctx));
                e.addErrorLine(job.klass.data(ctx)->loc(), "One definition");
                e.addErrorLine(resolved.data(ctx)->loc(), "Other definition");
            }
            resolved = stubSymbolForAncestor(job);
        }

        bool ancestorPresent = true;
        if (job.isSuperclass) {
            if (resolved == core::Symbols::todo()) {
                // No superclass specified
                ancestorPresent = false;
            } else if (!job.klass.data(ctx)->superClass().exists() ||
                       job.klass.data(ctx)->superClass() == core::Symbols::todo() ||
                       job.klass.data(ctx)->superClass() == resolved) {
                job.klass.data(ctx)->setSuperClass(resolved);
            } else {
                if (auto e = ctx.beginError(job.ancestor->loc, core::errors::Resolver::RedefinitionOfParents)) {
                    e.setHeader("Parent of class `{}` redefined from `{}` to `{}`", job.klass.data(ctx)->show(ctx),
                                job.klass.data(ctx)->superClass().show(ctx), resolved.show(ctx));
                }
            }
        } else {
            ENFORCE(resolved.data(ctx)->isClassOrModule());
            job.klass.data(ctx)->addMixin(resolved);
        }

        if (ancestorPresent) {
            saveAncestorTypeForHashing(ctx, job);
        }
        return true;
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

    void transformAncestor(core::Context ctx, core::SymbolRef klass, ast::TreePtr &ancestor,
                           bool isSuperclass = false) {
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

        if (auto *cnst = ast::cast_tree<ast::ConstantLit>(ancestor)) {
            auto sym = cnst->symbol;
            if (sym.exists() && sym.data(ctx)->isTypeAlias()) {
                if (auto e = ctx.beginError(cnst->loc, core::errors::Resolver::DynamicSuperclass)) {
                    e.setHeader("Superclasses and mixins may not be type aliases");
                }
                return;
            }
            ENFORCE(sym.exists() ||
                    ast::isa_tree<ast::ConstantLit>(ast::ref_tree<ast::UnresolvedConstantLit>(cnst->original).scope) ||
                    ast::isa_tree<ast::EmptyTree>(ast::ref_tree<ast::UnresolvedConstantLit>(cnst->original).scope));
            if (isSuperclass && sym == core::Symbols::todo()) {
                return;
            }
            job.ancestor = cnst;
        } else if (ancestor->isSelfReference()) {
            auto loc = ancestor->loc;
            auto enclosingClass = ctx.owner.data(ctx)->enclosingClass(ctx);
            auto nw = ast::MK::UnresolvedConstant(loc, std::move(ancestor), enclosingClass.data(ctx)->name);
            auto out = ast::make_tree<ast::ConstantLit>(loc, enclosingClass, std::move(nw));
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

    ast::TreePtr preTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        nesting_ = make_unique<Nesting>(std::move(nesting_), ast::ref_tree<ast::ClassDef>(tree).symbol);
        return tree;
    }

    ast::TreePtr postTransformUnresolvedConstantLit(core::Context ctx, ast::TreePtr tree) {
        auto &c = ast::ref_tree<ast::UnresolvedConstantLit>(tree);
        if (ast::isa_tree<ast::UnresolvedConstantLit>(c.scope)) {
            c.scope = postTransformUnresolvedConstantLit(ctx, std::move(c.scope));
        }
        auto loc = c.loc;
        auto out = ast::make_tree<ast::ConstantLit>(loc, core::Symbols::noSymbol(), std::move(tree));
        ResolutionItem job{nesting_, ctx.file, ast::cast_tree<ast::ConstantLit>(out)};
        if (resolveJob(ctx, job)) {
            categoryCounterInc("resolve.constants.nonancestor", "firstpass");
        } else {
            todo_.emplace_back(std::move(job));
        }
        return out;
    }

    ast::TreePtr postTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto &original = ast::ref_tree<ast::ClassDef>(tree);

        core::SymbolRef klass = original.symbol;

        for (auto &ancst : original.ancestors) {
            bool isSuperclass = (original.kind == ast::ClassDef::Kind::Class && &ancst == &original.ancestors.front() &&
                                 !klass.data(ctx)->isSingletonClass(ctx));
            transformAncestor(isSuperclass ? ctx : ctx.withOwner(klass), klass, ancst, isSuperclass);
        }

        auto singleton = klass.data(ctx)->lookupSingletonClass(ctx);
        for (auto &ancst : original.singletonAncestors) {
            ENFORCE(singleton.exists());
            transformAncestor(ctx.withOwner(klass), singleton, ancst);
        }

        nesting_ = nesting_->parent;
        return tree;
    }

    ast::TreePtr postTransformAssign(core::Context ctx, ast::TreePtr tree) {
        auto &asgn = ast::ref_tree<ast::Assign>(tree);

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
                auto temporaryUntyped = ast::MK::Block0(asgn.lhs->loc, ast::MK::Untyped(asgn.lhs->loc));
                send->block = std::move(temporaryUntyped);

                // because we're synthesizing a fake "untyped" here and actually adding it to the AST, we won't report
                // an arity mismatch for `T.untyped` in the future, so report the arity mismatch now
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeAlias)) {
                    e.setHeader("No block given to `{}`", "T.type_alias");
                    CorrectTypeAlias::eagerToLazy(ctx, e, send);
                }
            }
            auto &block = ast::ref_tree<ast::Block>(send->block);
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
                ResolveWalkResult result{move(constants.todo_), move(constants.todoAncestors_),
                                         move(constants.todoClassAliases_), move(constants.todoTypeAliases_),
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
            return locCompare(core::Loc(lhs.file, (*lhs.rhs)->loc), core::Loc(rhs.file, (*rhs.rhs)->loc));
        });
        fast_sort(trees, [](const auto &lhs, const auto &rhs) -> bool {
            return locCompare(core::Loc(lhs.file, lhs.tree->loc), core::Loc(rhs.file, rhs.tree->loc));
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

class ResolveTypeMembersWalk {
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

    vector<ResolveAssignItem> todoAssigns_;

    // State for tracking type usage inside of a type alias or type member
    // definition
    bool trackDependencies_ = false;
    vector<bool> classOfDepth_;
    vector<core::SymbolRef> dependencies_;

    void extendClassOfDepth(ast::Send &send) {
        if (trackDependencies_) {
            classOfDepth_.emplace_back(isT(send.recv) && send.fun == core::Names::classOf());
        }
    }

    static bool isT(const ast::TreePtr &expr) {
        auto *tMod = ast::cast_tree_const<ast::ConstantLit>(expr);
        return tMod && tMod->symbol == core::Symbols::T();
    }

    static bool isTodo(core::TypePtr type) {
        auto *todo = core::cast_type<core::ClassType>(type.get());
        return todo != nullptr && todo->symbol == core::Symbols::todo();
    }

    static bool isLHSResolved(core::MutableContext ctx, core::SymbolRef sym) {
        if (sym.data(ctx)->isTypeMember()) {
            auto *lambdaParam = core::cast_type<core::LambdaParam>(sym.data(ctx)->resultType.get());
            ENFORCE(lambdaParam != nullptr);

            // both bounds are set to todo in the namer, so it's sufficient to
            // just check one here.
            return !isTodo(lambdaParam->lowerBound);
        } else {
            return !isTodo(sym.data(ctx)->resultType);
        }
    }

    static bool isGenericResolved(core::MutableContext ctx, core::SymbolRef sym) {
        if (sym.data(ctx)->isClassOrModule()) {
            return absl::c_all_of(sym.data(ctx)->typeMembers(),
                                  [&](core::SymbolRef tm) { return isLHSResolved(ctx, tm); });
        } else {
            return isLHSResolved(ctx, sym);
        }
    }

    static void resolveTypeMember(core::MutableContext ctx, core::SymbolRef lhs, ast::Send *rhs) {
        auto data = lhs.data(ctx);
        auto owner = data->owner;

        core::LambdaParam *parentType = nullptr;
        auto parentMember = owner.data(ctx)->superClass().data(ctx)->findMember(ctx, data->name);
        if (parentMember.exists()) {
            if (parentMember.data(ctx)->isTypeMember()) {
                parentType = core::cast_type<core::LambdaParam>(parentMember.data(ctx)->resultType.get());
                ENFORCE(parentType != nullptr);
            } else if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::ParentTypeBoundsMismatch)) {
                const auto parentShow = parentMember.data(ctx)->show(ctx);
                e.setHeader("`{}` is a type member but `{}` is not a type member", data->show(ctx), parentShow);
                e.addErrorLine(parentMember.data(ctx)->loc(), "`{}` definition", parentShow);
            }
        }

        // Initialize the resultType to a LambdaParam with default bounds
        auto lambdaParam = core::make_type<core::LambdaParam>(lhs, core::Types::bottom(), core::Types::top());
        data->resultType = lambdaParam;
        auto *memberType = core::cast_type<core::LambdaParam>(lambdaParam.get());

        // When no args are supplied, this implies that the upper and lower
        // bounds of the type parameter are top and bottom.
        ast::Hash *hash = nullptr;
        if (rhs->args.size() == 1) {
            hash = ast::cast_tree<ast::Hash>(rhs->args[0]);
        } else if (rhs->args.size() == 2) {
            hash = ast::cast_tree<ast::Hash>(rhs->args[1]);
        }

        if (hash) {
            int i = -1;
            for (auto &keyExpr : hash->keys) {
                i++;
                auto lit = ast::cast_tree<ast::Literal>(keyExpr);
                if (lit && lit->isSymbol(ctx)) {
                    ParsedSig emptySig;
                    auto allowSelfType = true;
                    auto allowRebind = false;
                    auto allowTypeMember = false;
                    core::TypePtr resTy =
                        TypeSyntax::getResultType(ctx, hash->values[i], emptySig,
                                                  TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, lhs});

                    switch (lit->asSymbol(ctx)._id) {
                        case core::Names::fixed()._id:
                            memberType->lowerBound = resTy;
                            memberType->upperBound = resTy;
                            break;

                        case core::Names::lower()._id:
                            memberType->lowerBound = resTy;
                            break;

                        case core::Names::upper()._id:
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
                                parentType->lowerBound->show(ctx), memberType->lowerBound->show(ctx));
                }
            }
            if (!core::Types::isSubType(ctx, memberType->upperBound, parentType->upperBound)) {
                if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::ParentTypeBoundsMismatch)) {
                    e.setHeader("upper bound `{}` is not a subtype of parent upper bound `{}`",
                                memberType->upperBound->show(ctx), parentType->upperBound->show(ctx));
                }
            }
        }

        // Ensure that the new lower bound is a subtype of the upper
        // bound. This will be a no-op in the case that the type member
        // is fixed.
        if (!core::Types::isSubType(ctx, memberType->lowerBound, memberType->upperBound)) {
            if (auto e = ctx.beginError(rhs->loc, core::errors::Resolver::InvalidTypeMemberBounds)) {
                e.setHeader("`{}` is not a subtype of `{}`", memberType->lowerBound->show(ctx),
                            memberType->upperBound->show(ctx));
            }
        }

        // Once the owner has had all of its type members resolved, resolve the
        // AttachedClass on its singleton.
        if (isGenericResolved(ctx, owner)) {
            resolveAttachedClass(ctx, owner);
        }
    }

    static void resolveAttachedClass(core::MutableContext ctx, core::SymbolRef sym) {
        ENFORCE(sym.data(ctx)->isClassOrModule());

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

        auto *lambdaParam = core::cast_type<core::LambdaParam>(attachedClass.data(ctx)->resultType.get());
        ENFORCE(lambdaParam != nullptr);

        if (isTodo(lambdaParam->lowerBound)) {
            lambdaParam->upperBound = sym.data(ctx)->externalType(ctx);
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
                resolveAttachedClass(ctx, singleton);
            }
        }
    }

    static void resolveTypeAlias(core::MutableContext ctx, core::SymbolRef lhs, ast::Send *rhs) {
        // this is provided by ResolveConstantsWalk
        ENFORCE(ast::isa_tree<ast::Block>(rhs->block));
        auto &block = ast::ref_tree<ast::Block>(rhs->block);
        ENFORCE(block.body);

        auto allowSelfType = true;
        auto allowRebind = false;
        auto allowTypeMember = true;
        lhs.data(ctx)->resultType = TypeSyntax::getResultType(
            ctx, block.body, ParsedSig{}, TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, lhs});
    }

    static bool resolveJob(core::MutableContext ctx, ResolveAssignItem &job) {
        ENFORCE(job.lhs.data(ctx)->isTypeAlias() || job.lhs.data(ctx)->isTypeMember());

        if (isLHSResolved(ctx, job.lhs)) {
            return true;
        }

        auto it = std::remove_if(job.dependencies.begin(), job.dependencies.end(),
                                 [&](core::SymbolRef dep) { return isGenericResolved(ctx, dep); });
        job.dependencies.erase(it, job.dependencies.end());
        if (!job.dependencies.empty()) {
            return false;
        }

        if (job.lhs.data(ctx)->isTypeMember()) {
            auto superclass = job.lhs.data(ctx)->owner.data(ctx)->superClass();
            if (!isGenericResolved(ctx, superclass)) {
                return false;
            }

            resolveTypeMember(ctx.withOwner(job.owner), job.lhs, job.rhs);
        } else {
            resolveTypeAlias(ctx.withOwner(job.owner), job.lhs, job.rhs);
        }

        return true;
    }

public:
    ast::TreePtr preTransformClassDef(core::MutableContext ctx, ast::TreePtr tree) {
        auto &klass = ast::ref_tree<ast::ClassDef>(tree);

        // If this is a class with no type members defined, resolve attached
        // class immediately. Otherwise, it will be resolved once all type
        // members have been resolved as well.
        if (isGenericResolved(ctx, klass.symbol)) {
            resolveAttachedClass(ctx, klass.symbol);
        }

        return tree;
    }

    ast::TreePtr preTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto &send = ast::ref_tree<ast::Send>(tree);
        switch (send.fun._id) {
            case core::Names::typeAlias()._id:
            case core::Names::typeMember()._id:
            case core::Names::typeTemplate()._id:
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

    ast::TreePtr postTransformConstantLit(core::MutableContext ctx, ast::TreePtr tree) {
        auto &lit = ast::ref_tree<ast::ConstantLit>(tree);

        if (trackDependencies_) {
            core::SymbolRef symbol = lit.symbol.data(ctx)->dealias(ctx);
            if (symbol == core::Symbols::T()) {
                return tree;
            }

            if (symbol.data(ctx)->isClassOrModule()) {
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

    ast::TreePtr postTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto &send = ast::ref_tree<ast::Send>(tree);

        switch (send.fun._id) {
            case core::Names::typeMember()._id:
            case core::Names::typeTemplate()._id:
            case core::Names::typeAlias()._id:
                trackDependencies_ = false;
                break;

            default:
                if (trackDependencies_) {
                    classOfDepth_.pop_back();
                }
                break;
        }

        return tree;
    }

    ast::TreePtr postTransformAssign(core::MutableContext ctx, ast::TreePtr tree) {
        auto &asgn = ast::ref_tree<ast::Assign>(tree);

        auto *id = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (id == nullptr || !id->symbol.exists()) {
            return tree;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn.rhs);
        if (send == nullptr) {
            return tree;
        }

        auto sym = id->symbol;
        auto data = sym.data(ctx);
        if (data->isTypeAlias() || data->isTypeMember()) {
            ENFORCE(!data->isTypeMember() || send->recv->isSelfReference());

            // This is for a special case that happens with the generation of
            // reflection.rbi: it re-creates the type aliases of the payload,
            // without the knowledge that they are type aliases. The manifestation
            // of this, is that there are entries like:
            //
            // > module T
            // >   Boolean = T.let(nil, T.untyped)
            // > end
            if (data->isTypeAlias() && send->fun == core::Names::let()) {
                data->resultType = core::Types::untypedUntracked();
                return tree;
            }

            ENFORCE(send->fun == core::Names::typeAlias() || send->fun == core::Names::typeMember() ||
                    send->fun == core::Names::typeTemplate());

            auto job = ResolveAssignItem{ctx.owner, sym, send, dependencies_, ctx.file};
            if (!resolveJob(ctx, job)) {
                todoAssigns_.emplace_back(std::move(job));
            }
        }

        trackDependencies_ = false;
        dependencies_.clear();
        classOfDepth_.clear();

        return tree;
    }

    static vector<ast::ParsedFile> run(core::GlobalState &gs, vector<ast::ParsedFile> trees) {
        ResolveTypeMembersWalk walk;
        Timer timeit(gs.tracer(), "resolver.type_params");

        for (auto &tree : trees) {
            core::MutableContext ctx(gs, core::Symbols::root(), tree.file);
            tree.tree = ast::ShallowMap::apply(ctx, walk, std::move(tree.tree));
        }

        // loop over any out-of-order type_member/type_alias references
        bool progress = true;
        while (progress && !walk.todoAssigns_.empty()) {
            auto origSize = walk.todoAssigns_.size();
            auto it = std::remove_if(walk.todoAssigns_.begin(), walk.todoAssigns_.end(), [&](ResolveAssignItem &job) {
                core::MutableContext ctx(gs, core::Symbols::root(), job.file);
                return resolveJob(ctx, job);
            });
            walk.todoAssigns_.erase(it, walk.todoAssigns_.end());
            progress = walk.todoAssigns_.size() != origSize;
        }

        // If there was a step with no progress, there's a cycle in the
        // type member/alias declarations. This is handled by reporting an error
        // at `typed: false`, and marking all of the involved type
        // members/aliases as T.untyped.
        if (!walk.todoAssigns_.empty()) {
            for (auto &job : walk.todoAssigns_) {
                auto data = job.lhs.data(gs);

                if (data->isTypeMember()) {
                    data->resultType = core::make_type<core::LambdaParam>(job.lhs, core::Types::untypedUntracked(),
                                                                          core::Types::untypedUntracked());
                } else {
                    data->resultType = core::Types::untypedUntracked();
                }

                if (auto e = gs.beginError(data->loc(), core::errors::Resolver::TypeMemberCycle)) {
                    auto flavor = data->isTypeAlias() ? "alias" : "member";
                    e.setHeader("Type {} `{}` is involved in a cycle", flavor, data->show(gs));
                }
            }
        }

        return trees;
    }
};

class ResolveSignaturesWalk {
private:
    std::vector<int> nestedBlockCounts;

    ast::Local const *getArgLocal(core::Context ctx, const core::ArgInfo &argSym, const ast::MethodDef &mdef, int pos,
                                  bool isOverloaded) {
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

    void fillInInfoFromSig(core::MutableContext ctx, core::SymbolRef method, core::LocOffsets exprLoc, ParsedSig sig,
                           bool isOverloaded, const ast::MethodDef &mdef) {
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
                    auto asTypeVar = core::cast_type<core::TypeVar>(typeSpec.type.get());
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

        // Get the parameters order from the signature
        vector<ParsedSig::ArgSpec> sigParams;
        for (auto &spec : sig.argTypes) {
            sigParams.push_back(spec);
        }

        vector<ast::Local const *> defParams; // Parameters order from the method declaration
        bool seenOptional = false;

        auto methodInfo = method.data(ctx);
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
                arg.type = spec->type;
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

        for (auto spec : sig.argTypes) {
            if (auto e = ctx.state.beginError(spec.loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Unknown argument name `{}`", spec.name.show(ctx));
            }
        }

        // Check params ordering match between signature and definition
        if (sig.argTypes.empty()) {
            int j = 0;
            for (auto spec : sigParams) {
                auto param = defParams[j];
                auto sname = spec.name.show(ctx);
                auto dname = param->localVariable._name.show(ctx);
                if (sname != dname) {
                    if (auto e = ctx.beginError(param->loc, core::errors::Resolver::BadParameterOrdering)) {
                        e.setHeader("Bad parameter ordering for `{}`, expected `{}` instead", dname, sname);
                        e.addErrorLine(spec.loc, "Expected index in signature:");
                    }
                }
                j++;
            }
        }
    }

    // Force errors from any signatures that didn't attach to methods.
    // `lastSigs` will always be empty after this function is called.
    void processLeftoverSigs(core::MutableContext ctx, InlinedVector<ast::Send *, 1> &lastSigs) {
        if (!lastSigs.empty()) {
            // These sigs won't have been parsed, as there was no methods to
            // attach them to -- parse them here manually to force any errors.
            for (auto sig : lastSigs) {
                auto allowSelfType = true;
                auto allowRebind = false;
                auto allowTypeMember = true;
                TypeSyntax::parseSig(
                    ctx, sig, nullptr,
                    TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, core::Symbols::untyped()});
            }

            if (auto e = ctx.beginError(lastSigs[0]->loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`. No method def following it", "sig");
            }

            lastSigs.clear();
        }
    }

    void processClassBody(core::MutableContext ctx, ast::ClassDef &klass) {
        InlinedVector<ast::Send *, 1> lastSigs;
        for (auto &stat : klass.rhs) {
            processStatement(ctx, stat, lastSigs);
        }

        processLeftoverSigs(ctx, lastSigs);

        auto toRemove =
            remove_if(klass.rhs.begin(), klass.rhs.end(), [](ast::TreePtr &stat) -> bool { return stat == nullptr; });
        klass.rhs.erase(toRemove, klass.rhs.end());
    }

    void processInSeq(core::MutableContext ctx, ast::InsSeq &seq) {
        InlinedVector<ast::Send *, 1> lastSigs;

        // Explicitly check in the contxt of the class, not <static-init>
        auto classCtx = ctx.withOwner(ctx.owner.data(ctx)->enclosingClass(ctx));

        for (auto &stat : seq.stats) {
            processStatement(classCtx, stat, lastSigs);
        }
        if (!ast::isa_tree<ast::EmptyTree>(seq.expr)) {
            processStatement(classCtx, seq.expr, lastSigs);
        }

        processLeftoverSigs(classCtx, lastSigs);

        auto toRemove =
            remove_if(seq.stats.begin(), seq.stats.end(), [](ast::TreePtr &stat) -> bool { return stat == nullptr; });
        seq.stats.erase(toRemove, seq.stats.end());
    }

    void processStatement(core::MutableContext ctx, ast::TreePtr &stat, InlinedVector<ast::Send *, 1> &lastSigs) {
        typecase(
            stat.get(),

            [&](ast::Send *send) {
                if (TypeSyntax::isSig(ctx, send)) {
                    if (!lastSigs.empty()) {
                        if (!ctx.permitOverloadDefinitions(ctx.file)) {
                            if (auto e = ctx.beginError(lastSigs[0]->loc, core::errors::Resolver::OverloadNotAllowed)) {
                                e.setHeader("Unused type annotation. No method def before next annotation");
                                e.addErrorLine(core::Loc(ctx.file, send->loc),
                                               "Type annotation that will be used instead");
                            }
                        }
                    }

                    lastSigs.emplace_back(send);
                    return;
                }

                if (send->args.size() == 1 &&
                    (send->fun == core::Names::public_() || send->fun == core::Names::private_() ||
                     send->fun == core::Names::privateClassMethod() || send->fun == core::Names::protected_())) {
                    processStatement(ctx, send->args[0], lastSigs);
                    return;
                }
            },

            [&](ast::MethodDef *mdef) {
                if (debug_mode) {
                    bool hasSig = !lastSigs.empty();
                    bool rewriten = mdef->flags.isRewriterSynthesized;
                    bool isRBI = mdef->declLoc.file().data(ctx).isRBI();
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
                    prodCounterInc("types.sig.count");

                    auto loc = lastSigs[0]->loc;
                    if (ctx.file.data(ctx).originalSigil == core::StrictLevel::None &&
                        !lastSigs.front()->flags.isRewriterSynthesized) {
                        if (auto e = ctx.beginError(loc, core::errors::Resolver::SigInFileWithoutSigil)) {
                            e.setHeader("To use `{}`, this file must declare an explicit `{}` sigil (found: "
                                        "none). If you're not sure which one to use, start with `{}`",
                                        "sig", "# typed:", "# typed: false");
                        }
                    }

                    bool isOverloaded = lastSigs.size() > 1 && ctx.permitOverloadDefinitions(ctx.file);
                    auto originalName = mdef->symbol.data(ctx)->name;
                    if (isOverloaded) {
                        ctx.state.mangleRenameSymbol(mdef->symbol, originalName);
                    }
                    int i = 0;

                    // process signatures in the context of either the current
                    // class, or the current singleton class, depending on if
                    // the current method is a self method.
                    core::SymbolRef sigOwner;
                    if (mdef->flags.isSelfMethod) {
                        sigOwner = ctx.owner.data(ctx)->singletonClass(ctx);
                    } else {
                        sigOwner = ctx.owner;
                    }

                    while (i < lastSigs.size()) {
                        auto allowSelfType = true;
                        auto allowRebind = false;
                        auto allowTypeMember = true;
                        auto sig = TypeSyntax::parseSig(
                            ctx.withOwner(sigOwner), lastSigs[i], nullptr,
                            TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, mdef->symbol});
                        core::SymbolRef overloadSym;
                        if (isOverloaded) {
                            vector<int> argsToKeep;
                            int argId = -1;
                            for (auto &argTree : mdef->args) {
                                argId++;
                                const auto local = ast::MK::arg2Local(argTree);
                                auto treeArgName = local->localVariable._name;
                                ENFORCE(local != nullptr);
                                auto spec =
                                    absl::c_find_if(sig.argTypes, [&](auto &spec) { return spec.name == treeArgName; });
                                if (spec != sig.argTypes.end()) {
                                    argsToKeep.emplace_back(argId);
                                }
                            }
                            overloadSym = ctx.state.enterNewMethodOverload(core::Loc(ctx.file, lastSigs[i]->loc),
                                                                           mdef->symbol, originalName, i, argsToKeep);
                            if (i != lastSigs.size() - 1) {
                                overloadSym.data(ctx)->setOverloaded();
                            }
                        } else {
                            overloadSym = mdef->symbol;
                        }
                        fillInInfoFromSig(ctx, overloadSym, lastSigs[i]->loc, move(sig), isOverloaded, *mdef);
                        i++;
                    }

                    // OVERLOAD
                    lastSigs.clear();
                }

                if (mdef->symbol.data(ctx)->isAbstract()) {
                    if (!ast::isa_tree<ast::EmptyTree>(mdef->rhs)) {
                        if (auto e = ctx.beginError(mdef->rhs->loc, core::errors::Resolver::AbstractMethodWithBody)) {
                            e.setHeader("Abstract methods must not contain any code in their body");
                            e.replaceWith("Delete the body", core::Loc(ctx.file, mdef->rhs->loc), "");
                        }

                        mdef->rhs = ast::MK::EmptyTree();
                    }
                    if (!mdef->symbol.data(ctx)->enclosingClass(ctx).data(ctx)->isClassOrModuleAbstract()) {
                        if (auto e = ctx.beginError(mdef->loc, core::errors::Resolver::AbstractMethodOutsideAbstract)) {
                            e.setHeader("Before declaring an abstract method, you must mark your class/module "
                                        "as abstract using `abstract!` or `interface!`");
                        }
                    }

                    // Rewrite the empty body of the abstract method to forward all arguments to `super`, mirroring the
                    // behavior of the runtime.
                    ast::Send::ARGS_store args;

                    ast::Hash::ENTRY_store keywordArgKeys;
                    ast::Hash::ENTRY_store keywordArgVals;

                    auto argIdx = -1;
                    for (auto &arg : mdef->args) {
                        ++argIdx;

                        ast::Local *local = nullptr;
                        if (auto *opt = ast::cast_tree<ast::OptionalArg>(arg)) {
                            local = ast::cast_tree<ast::Local>(opt->expr);
                        } else {
                            local = ast::cast_tree<ast::Local>(arg);
                        }

                        auto &info = mdef->symbol.data(ctx)->arguments()[argIdx];
                        if (info.flags.isKeyword) {
                            keywordArgKeys.emplace_back(ast::MK::Symbol(local->loc, info.name));
                            keywordArgVals.emplace_back(local->deepCopy());
                        } else if (info.flags.isRepeated || info.flags.isBlock) {
                            // Explicitly skip for now.
                            // Involves synthesizing a call to callWithSplat, callWithBlock, or
                            // callWithSplatAndBlock
                        } else {
                            ENFORCE(keywordArgKeys.empty());
                            args.emplace_back(local->deepCopy());
                        }
                    }

                    if (!keywordArgKeys.empty()) {
                        args.emplace_back(
                            ast::MK::Hash(mdef->loc, std::move(keywordArgKeys), std::move(keywordArgVals)));
                    }

                    auto self = ast::MK::Self(mdef->loc);
                    mdef->rhs = ast::MK::Send(mdef->loc, std::move(self), core::Names::super(), std::move(args));
                } else if (mdef->symbol.data(ctx)->enclosingClass(ctx).data(ctx)->isClassOrModuleInterface()) {
                    if (auto e = ctx.beginError(mdef->loc, core::errors::Resolver::ConcreteMethodInInterface)) {
                        e.setHeader("All methods in an interface must be declared abstract");
                    }
                }
            },
            [&](ast::ClassDef *cdef) {
                // Leave in place
            },

            [&](ast::EmptyTree *e) { stat.reset(nullptr); },

            [&](ast::Expression *e) {});
    }

    // Resolve the type of the rhs of a constant declaration. This logic is
    // extremely simplistic; We only handle simple literals, and explicit casts.
    //
    // We don't handle array or hash literals, because intuiting the element
    // type (once we have generics) will be nontrivial.
    core::TypePtr resolveConstantType(core::Context ctx, ast::TreePtr &expr, core::SymbolRef ofSym) {
        core::TypePtr result;
        typecase(
            expr.get(), [&](ast::Literal *a) { result = a->value; },
            [&](ast::Cast *cast) {
                if (cast->cast != core::Names::let()) {
                    if (auto e = ctx.beginError(cast->loc, core::errors::Resolver::ConstantAssertType)) {
                        e.setHeader("Use `{}` to specify the type of constants", "T.let");
                    }
                }
                result = cast->type;
            },
            [&](ast::InsSeq *outer) { result = resolveConstantType(ctx, outer->expr, ofSym); },
            [&](ast::Expression *expr) {});
        return result;
    }

    bool handleDeclaration(core::MutableContext ctx, ast::Assign &asgn) {
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
            }
        }

        core::SymbolRef scope;
        if (uid->kind == ast::UnresolvedIdent::Kind::Class) {
            if (!ctx.owner.data(ctx)->isClassOrModule()) {
                if (auto e = ctx.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                    e.setHeader("The class variable `{}` must be declared at class scope", uid->name.show(ctx));
                }
            }

            scope = ctx.owner.data(ctx)->enclosingClass(ctx);
        } else {
            // we need to check nested block counts because we want all fields to be declared on top level of either
            // class or body, rather then nested in some block
            if (nestedBlockCounts.back() == 0 && ctx.owner.data(ctx)->isClassOrModule()) {
                // Declaring a class instance variable
            } else if (nestedBlockCounts.back() == 0 && ctx.owner.data(ctx)->name == core::Names::initialize()) {
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
                return true;
            } else {
                // We do some normalization here to ensure that the file / line we report the error on doesn't
                // depend on the order that we traverse files nor the order we traverse within a file.
                auto priorLoc = prior.data(ctx)->loc();
                core::Loc reportOn;
                core::Loc errorLine;
                core::Loc thisLoc = core::Loc(ctx.file, uid->loc);
                if (thisLoc.file() == priorLoc.file()) {
                    reportOn = thisLoc.beginPos() < priorLoc.beginPos() ? thisLoc : priorLoc;
                    errorLine = thisLoc.beginPos() < priorLoc.beginPos() ? priorLoc : thisLoc;
                } else {
                    reportOn = thisLoc.file() < priorLoc.file() ? thisLoc : priorLoc;
                    errorLine = thisLoc.file() < priorLoc.file() ? priorLoc : thisLoc;
                }

                if (auto e = ctx.state.beginError(reportOn, core::errors::Resolver::DuplicateVariableDeclaration)) {
                    e.setHeader("Redeclaring variable `{}` with mismatching type", uid->name.data(ctx)->show(ctx));
                    e.addErrorLine(errorLine, "Previous declaration is here:");
                }
                return false;
            }
        }
        core::SymbolRef var;

        if (uid->kind == ast::UnresolvedIdent::Kind::Class) {
            var = ctx.state.enterStaticFieldSymbol(core::Loc(ctx.file, uid->loc), scope, uid->name);
        } else {
            var = ctx.state.enterFieldSymbol(core::Loc(ctx.file, uid->loc), scope, uid->name);
        }

        var.data(ctx)->resultType = cast->type;
        return true;
    }

    void validateNonForcingIsA(core::Context ctx, const ast::Send &send) {
        constexpr string_view method = "T::NonForcingConstants.non_forcing_is_a?";

        if (send.args.size() != 2) {
            return;
        }

        auto stringLoc = send.args[1]->loc;

        auto *literalNode = ast::cast_tree_const<ast::Literal>(send.args[1]);
        if (literalNode == nullptr) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("`{}` only accepts string literals", method);
            }
            return;
        }

        auto literal = core::cast_type<core::LiteralType>(literalNode->value.get());
        if (literal == nullptr) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("`{}` only accepts string literals", method);
            }
            return;
        }

        if (literal->literalKind != core::LiteralType::LiteralTypeKind::String) {
            // Infer will report a type error
            return;
        }

        auto name = core::NameRef(ctx.state, literal->value);
        auto shortName = name.data(ctx)->shortName(ctx);
        if (shortName.empty()) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("The string given to `{}` must not be empty", method);
            }
            return;
        }

        auto parts = absl::StrSplit(shortName, "::");
        core::SymbolRef current;
        for (auto part : parts) {
            if (!current.exists()) {
                // First iteration
                if (part != "") {
                    if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                        e.setHeader(
                            "The string given to `{}` must be an absolute constant reference that starts with `{}`",
                            method, "::");
                    }
                    return;
                }

                current = core::Symbols::root();
            } else {
                auto member = ctx.state.lookupNameConstant(part);
                if (!member.exists()) {
                    if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                        auto prettyCurrent =
                            current == core::Symbols::root() ? "" : "::" + current.data(ctx)->show(ctx);
                        auto pretty = fmt::format("{}::{}", prettyCurrent, part);
                        e.setHeader("Unable to resolve constant `{}`", pretty);
                    }
                    return;
                }

                auto newCurrent = current.data(ctx)->findMember(ctx, member);
                if (!newCurrent.exists()) {
                    if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                        auto prettyCurrent =
                            current == core::Symbols::root() ? "" : "::" + current.data(ctx)->show(ctx);
                        auto pretty = fmt::format("{}::{}", prettyCurrent, part);
                        e.setHeader("Unable to resolve constant `{}`", pretty);
                    }
                    return;
                }
                current = newCurrent;
            }
        }

        ENFORCE(current.exists(), "Loop invariant violated");

        if (!current.data(ctx)->isClassOrModule()) {
            if (auto e = ctx.beginError(stringLoc, core::errors::Resolver::LazyResolve)) {
                e.setHeader("The string given to `{}` must resolve to a class or module", method);
                e.addErrorLine(current.data(ctx)->loc(), "Resolved to this constant");
            }
            return;
        }
    }

    core::SymbolRef methodOwner(core::Context ctx) {
        core::SymbolRef owner = ctx.owner.data(ctx)->enclosingClass(ctx);
        if (owner == core::Symbols::root()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }

public:
    ResolveSignaturesWalk() {
        nestedBlockCounts.emplace_back(0);
    }

    ast::TreePtr postTransformAssign(core::MutableContext ctx, ast::TreePtr tree) {
        auto &asgn = ast::ref_tree<ast::Assign>(tree);

        if (handleDeclaration(ctx, asgn)) {
            return tree;
        }

        auto *id = ast::cast_tree<ast::ConstantLit>(asgn.lhs);
        if (id == nullptr || !id->symbol.exists()) {
            return tree;
        }

        auto sym = id->symbol;
        auto data = sym.data(ctx);
        if (data->isTypeAlias() || data->isTypeMember()) {
            return tree;
        }

        if (data->isStaticField()) {
            if (data->resultType == nullptr) {
                data->resultType = resolveConstantType(ctx, asgn.rhs, sym);
                if (data->resultType == nullptr) {
                    // Instead of emitting an error now, emit an error in infer that has a proper type suggestion
                    auto rhs = move(asgn.rhs);
                    auto loc = rhs->loc;
                    asgn.rhs = ast::MK::Send1(loc, ast::MK::Constant(loc, core::Symbols::Magic()),
                                              core::Names::suggestType(), move(rhs));
                }
            } else if (!core::isa_type<core::AliasType>(data->resultType.get())) {
                // If we've already resolved a temporary constant, we still want to run resolveConstantType to
                // report errors (e.g. so that a stand-in untyped value won't suppress errors in subsequent
                // typechecking runs) but we only want to run this on constants that are value-level and not class
                // or type aliases. The check for isa_type<AliasType> makes sure that we skip aliases of the form `X
                // = Integer` and only run this over constant value assignments like `X = 5` or `Y = 5; X = Y`.
                if (resolveConstantType(ctx, asgn.rhs, sym) == nullptr) {
                    if (auto e = ctx.beginError(asgn.rhs->loc, core::errors::Resolver::ConstantMissingTypeAnnotation)) {
                        e.setHeader("Constants must have type annotations with `{}` when specifying `{}`", "T.let",
                                    "# typed: strict");
                    }
                }
            }
        }

        return tree;
    }

    ast::TreePtr preTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        nestedBlockCounts.emplace_back(0);
        return tree;
    }

    ast::TreePtr postTransformClassDef(core::MutableContext ctx, ast::TreePtr tree) {
        auto &klass = ast::ref_tree<ast::ClassDef>(tree);
        processClassBody(ctx.withOwner(klass.symbol), klass);
        return tree;
    }

    ast::TreePtr preTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        nestedBlockCounts.emplace_back(0);
        return tree;
    }

    ast::TreePtr postTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        nestedBlockCounts.pop_back();
        return tree;
    }

    ast::TreePtr preTransformBlock(core::Context ctx, ast::TreePtr tree) {
        nestedBlockCounts.back() += 1;
        return tree;
    }

    ast::TreePtr postTransformBlock(core::Context ctx, ast::TreePtr tree) {
        nestedBlockCounts.back() -= 1;
        return tree;
    }

    ast::TreePtr postTransformInsSeq(core::MutableContext ctx, ast::TreePtr tree) {
        processInSeq(ctx, ast::ref_tree<ast::InsSeq>(tree));
        return tree;
    }

    ast::TreePtr postTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto &send = ast::ref_tree<ast::Send>(tree);

        if (auto *id = ast::cast_tree<ast::ConstantLit>(send.recv)) {
            if (id->symbol != core::Symbols::T() && id->symbol != core::Symbols::T_NonForcingConstants()) {
                return tree;
            }
            switch (send.fun._id) {
                case core::Names::let()._id:
                case core::Names::assertType()._id:
                case core::Names::cast()._id: {
                    if (send.args.size() < 2) {
                        return tree;
                    }

                    // Compute the containing class when translating the type,
                    // as there's a very good chance this has been called from a
                    // method context.
                    core::SymbolRef ownerClass = ctx.owner.data(ctx)->enclosingClass(ctx);

                    auto expr = std::move(send.args[0]);
                    ParsedSig emptySig;
                    auto allowSelfType = true;
                    auto allowRebind = false;
                    auto allowTypeMember = true;
                    auto type = TypeSyntax::getResultType(
                        ctx.withOwner(ownerClass), send.args[1], emptySig,
                        TypeSyntaxArgs{allowSelfType, allowRebind, allowTypeMember, core::Symbols::noSymbol()});
                    return ast::MK::InsSeq1(send.loc, ast::MK::KeepForTypechecking(std::move(send.args[1])),
                                            ast::make_tree<ast::Cast>(send.loc, type, std::move(expr), send.fun));
                }
                case core::Names::revealType()._id:
                case core::Names::absurd()._id: {
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

                    auto fun = fmt::format("T.{}", send.fun.data(ctx)->show(ctx));
                    if (ctx.file.data(ctx).strictLevel <= core::StrictLevel::False) {
                        if (auto e = ctx.beginError(send.loc, core::errors::Resolver::RevealTypeInUntypedFile)) {
                            e.setHeader("`{}` can only {} in `{}` files (or higher)", fun, doWhat, "# typed: true");
                        }
                    }
                    return tree;
                }
                case core::Names::nonForcingIsA_p()._id:
                    validateNonForcingIsA(ctx, send);
                    return tree;
                default:
                    return tree;
            }
        } else if (send.recv.get()->isSelfReference()) {
            if (send.fun != core::Names::aliasMethod()) {
                return tree;
            }

            vector<core::NameRef> args;
            for (auto &arg : send.args) {
                auto lit = ast::cast_tree<ast::Literal>(arg);
                if (lit == nullptr || !lit->isSymbol(ctx)) {
                    continue;
                }
                core::NameRef name = lit->asSymbol(ctx);

                args.emplace_back(name);
            }
            if (send.args.size() != 2) {
                return tree;
            }
            if (args.size() != 2) {
                return tree;
            }

            auto fromName = args[0];
            auto toName = args[1];

            auto owner = methodOwner(ctx);
            core::SymbolRef toMethod = owner.data(ctx)->findMemberNoDealias(ctx, toName);
            if (toMethod.exists()) {
                toMethod = toMethod.data(ctx)->dealiasMethod(ctx);
            }

            if (!toMethod.exists()) {
                if (auto e = ctx.beginError(send.args[1]->loc, core::errors::Resolver::BadAliasMethod)) {
                    e.setHeader("Can't make method alias from `{}` to non existing method `{}`", fromName.show(ctx),
                                toName.show(ctx));
                }
                toMethod = core::Symbols::Sorbet_Private_Static_badAliasMethodStub();
            }

            core::SymbolRef fromMethod = owner.data(ctx)->findMemberNoDealias(ctx, fromName);
            if (fromMethod.exists() && fromMethod.data(ctx)->dealiasMethod(ctx) != toMethod) {
                if (auto e = ctx.beginError(send.loc, core::errors::Resolver::BadAliasMethod)) {
                    auto dealiased = fromMethod.data(ctx)->dealiasMethod(ctx);
                    if (fromMethod == dealiased) {
                        e.setHeader("Redefining the existing method `{}` as a method alias",
                                    fromMethod.data(ctx)->show(ctx));
                        e.addErrorLine(fromMethod.data(ctx)->loc(), "Previous definition");
                    } else {
                        e.setHeader("Redefining method alias `{}` from `{}` to `{}`", fromMethod.data(ctx)->show(ctx),
                                    dealiased.data(ctx)->show(ctx), toMethod.data(ctx)->show(ctx));
                        e.addErrorLine(fromMethod.data(ctx)->loc(), "Previous alias definition");
                        e.addErrorLine(dealiased.data(ctx)->loc(), "Previous alias pointed to");
                        e.addErrorLine(toMethod.data(ctx)->loc(), "Redefining alias to");
                    }
                }
                return tree;
            }

            // No need to make an alias when they're already the same symbol.
            if (fromMethod == toMethod) {
                return tree;
            }

            core::SymbolRef alias = ctx.state.enterMethodSymbol(core::Loc(ctx.file, send.loc), owner, fromName);
            alias.data(ctx)->resultType = core::make_type<core::AliasType>(toMethod);

            return tree;
        } else {
            return tree;
        }
    }
};

class ResolveMixesInClassMethodsWalk {
    void processMixesInClassMethods(core::MutableContext ctx, ast::Send &send) {
        if (!ctx.owner.data(ctx)->isClassOrModule() || !ctx.owner.data(ctx)->isClassOrModuleModule()) {
            if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` can only be declared inside a module, not a class", send.fun.data(ctx)->show(ctx));
            }
            // Keep processing it anyways
        }

        if (send.args.size() != 1) {
            if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Wrong number of arguments to `{}`: Expected: `{}`, got: `{}`",
                            send.fun.data(ctx)->show(ctx), 1, send.args.size());
            }
            return;
        }
        auto &front = send.args.front();
        auto *id = ast::cast_tree<ast::ConstantLit>(front);
        if (id == nullptr || !id->symbol.exists() || !id->symbol.data(ctx)->isClassOrModule()) {
            if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Argument to `{}` must be statically resolvable to a module",
                            send.fun.data(ctx)->show(ctx));
            }
            return;
        }
        if (id->symbol.data(ctx)->isClassOrModuleClass()) {
            if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` is a class, not a module; Only modules may be mixins",
                            id->symbol.data(ctx)->show(ctx));
            }
            return;
        }
        if (id->symbol == ctx.owner) {
            if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Must not pass your self to `{}`", send.fun.data(ctx)->show(ctx));
            }
            return;
        }
        auto existing = ctx.owner.data(ctx)->findMember(ctx, core::Names::classMethods());
        if (existing.exists() && existing != id->symbol) {
            if (auto e = ctx.beginError(send.loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Redeclaring `{}` from module `{}` to module `{}`", send.fun.data(ctx)->show(ctx),
                            existing.data(ctx)->show(ctx), id->symbol.data(ctx)->show(ctx));
            }
            return;
        }
        ctx.owner.data(ctx)->members()[core::Names::classMethods()] = id->symbol;
    }

public:
    ast::TreePtr postTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto &send = ast::ref_tree<ast::Send>(tree);
        if (send.recv->isSelfReference() && send.fun == core::Names::mixesInClassMethods()) {
            processMixesInClassMethods(ctx, send);
            return ast::MK::EmptyTree();
        }
        return tree;
    }
};

class ResolveSanityCheckWalk {
public:
    ast::TreePtr postTransformClassDef(core::MutableContext ctx, ast::TreePtr tree) {
        auto &original = ast::ref_tree<ast::ClassDef>(tree);
        ENFORCE(original.symbol != core::Symbols::todo(), "These should have all been resolved: {}",
                original.toString(ctx));
        if (original.symbol == core::Symbols::root()) {
            ENFORCE(ctx.state.lookupStaticInitForFile(core::Loc(ctx.file, original.loc)).exists());
        } else {
            ENFORCE(ctx.state.lookupStaticInitForClass(original.symbol).exists());
        }
        return tree;
    }
    ast::TreePtr postTransformMethodDef(core::MutableContext ctx, ast::TreePtr tree) {
        auto &original = ast::ref_tree<ast::MethodDef>(tree);
        ENFORCE(original.symbol != core::Symbols::todo(), "These should have all been resolved: {}",
                original.toString(ctx));
        return tree;
    }
    ast::TreePtr postTransformUnresolvedConstantLit(core::MutableContext ctx, ast::TreePtr tree) {
        auto &original = ast::ref_tree<ast::UnresolvedConstantLit>(tree);
        ENFORCE(false, "These should have all been removed: {}", original.toString(ctx));
        return tree;
    }
    ast::TreePtr postTransformUnresolvedIdent(core::MutableContext ctx, ast::TreePtr tree) {
        auto &original = ast::ref_tree<ast::UnresolvedIdent>(tree);
        ENFORCE(original.kind != ast::UnresolvedIdent::Kind::Local, "{} should have been removed by local_vars",
                original.toString(ctx));
        return tree;
    }
    ast::TreePtr postTransformConstantLit(core::MutableContext ctx, ast::TreePtr tree) {
        auto &original = ast::ref_tree<ast::ConstantLit>(tree);
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
    auto result = resolveMixesInClassMethods(gs, std::move(trees));
    if (!result.hasResult()) {
        return result;
    }
    trees = move(result.result());
    finalizeSymbols(gs);
    if (epochManager.wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled();
    }
    trees = ResolveTypeMembersWalk::run(gs, std::move(trees));
    if (epochManager.wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled();
    }
    result = resolveSigs(gs, std::move(trees));
    if (!result.hasResult()) {
        return result;
    }
    sanityCheck(gs, result.result());

    return result;
}

ast::ParsedFilesOrCancelled Resolver::resolveSigs(core::GlobalState &gs, vector<ast::ParsedFile> trees) {
    ResolveSignaturesWalk sigs;
    const auto &epochManager = gs.epochManager;
    Timer timeit(gs.tracer(), "resolver.sigs_vars_and_flatten");
    u4 count = 0;
    for (auto &tree : trees) {
        count++;
        // Don't check every turn of the loop. We want to be responsive to cancelation without harming throughput.
        if (count % 250 == 0 && epochManager->wasTypecheckingCanceled()) {
            return ast::ParsedFilesOrCancelled();
        }
        core::MutableContext ctx(gs, core::Symbols::root(), tree.file);
        tree.tree = ast::TreeMap::apply(ctx, sigs, std::move(tree.tree));
    }

    return trees;
}

ast::ParsedFilesOrCancelled Resolver::resolveMixesInClassMethods(core::GlobalState &gs, vector<ast::ParsedFile> trees) {
    ResolveMixesInClassMethodsWalk mixesInClassMethods;
    const auto &epochManager = gs.epochManager;
    Timer timeit(gs.tracer(), "resolver.mixes_in_class_methods");
    u4 count = 0;
    for (auto &tree : trees) {
        count++;
        // Don't check every turn of the loop. We want to be responsive to cancelation without harming throughput.
        if (count % 250 == 0 && epochManager->wasTypecheckingCanceled()) {
            return ast::ParsedFilesOrCancelled();
        }
        core::MutableContext ctx(gs, core::Symbols::root(), tree.file);
        tree.tree = ast::TreeMap::apply(ctx, mixesInClassMethods, std::move(tree.tree));
    }
    return trees;
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

ast::ParsedFilesOrCancelled Resolver::runTreePasses(core::GlobalState &gs, vector<ast::ParsedFile> trees) {
    auto workers = WorkerPool::create(0, gs.tracer());
    trees = ResolveConstantsWalk::resolveConstants(gs, std::move(trees), *workers);
    auto result = resolveMixesInClassMethods(gs, std::move(trees));
    if (!result.hasResult()) {
        return result;
    }
    trees = move(result.result());
    computeLinearization(gs);
    trees = ResolveTypeMembersWalk::run(gs, std::move(trees));
    result = resolveSigs(gs, std::move(trees));
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
