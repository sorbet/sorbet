#include "core/errors/resolver.h"
#include "ast/Helpers.h"
#include "ast/Trees.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/Error.h"
#include "core/Names.h"
#include "core/StrictLevel.h"
#include "core/core.h"
#include "resolver/resolver.h"
#include "resolver/type_syntax.h"

#include "absl/strings/str_cat.h"
#include "common/Timer.h"
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
 * - FlattenWalk
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
 * After this pass:
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
        ast::ConstantLit *out;

        ResolutionItem() = default;
        ResolutionItem(ResolutionItem &&rhs) noexcept = default;
        ResolutionItem &operator=(ResolutionItem &&rhs) noexcept = default;

        ResolutionItem(const ResolutionItem &rhs) = delete;
        const ResolutionItem &operator=(const ResolutionItem &rhs) = delete;
    };
    struct AncestorResolutionItem {
        ast::ConstantLit *ancestor;
        core::SymbolRef klass;

        bool isSuperclass; // true if superclass, false for mixin

        AncestorResolutionItem() = default;
        AncestorResolutionItem(AncestorResolutionItem &&rhs) noexcept = default;
        AncestorResolutionItem &operator=(AncestorResolutionItem &&rhs) noexcept = default;

        AncestorResolutionItem(const AncestorResolutionItem &rhs) = delete;
        const AncestorResolutionItem &operator=(const AncestorResolutionItem &rhs) = delete;
    };

    struct ClassAliasResolutionItem {
        core::SymbolRef lhs;
        ast::ConstantLit *rhs;

        ClassAliasResolutionItem() = default;
        ClassAliasResolutionItem(ClassAliasResolutionItem &&) noexcept = default;
        ClassAliasResolutionItem &operator=(ClassAliasResolutionItem &&rhs) noexcept = default;

        ClassAliasResolutionItem(const ClassAliasResolutionItem &) = delete;
        const ClassAliasResolutionItem &operator=(const ClassAliasResolutionItem &) = delete;
    };

    struct TypeAliasResolutionItem {
        core::SymbolRef lhs;
        ast::Expression *rhs;

        TypeAliasResolutionItem(core::SymbolRef lhs, ast::Expression *rhs) : lhs(lhs), rhs(rhs) {}
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

        unique_ptr<ast::ConstantLit> postTransformConstantLit(core::Context ctx,
                                                              unique_ptr<ast::ConstantLit> original) {
            seenUnresolved |= !isAlreadyResolved(ctx, *original);
            return original;
        };
    };

    static bool isFullyResolved(core::Context ctx, const ast::Expression *expression) {
        ResolutionChecker checker;
        unique_ptr<ast::Expression> dummy(const_cast<ast::Expression *>(expression));
        dummy = ast::TreeMap::apply(ctx, checker, std::move(dummy));
        ENFORCE(dummy.get() == expression);
        dummy.release();
        return !checker.seenUnresolved;
    }

    static core::SymbolRef resolveConstant(core::Context ctx, shared_ptr<Nesting> nesting,
                                           const unique_ptr<ast::UnresolvedConstantLit> &c) {
        if (ast::isa_tree<ast::EmptyTree>(c->scope.get())) {
            core::SymbolRef result = resolveLhs(ctx, nesting, c->cnst);
            return result;
        }
        ast::Expression *resolvedScope = c->scope.get();
        if (auto *id = ast::cast_tree<ast::ConstantLit>(resolvedScope)) {
            auto sym = id->symbol;
            if (sym.exists() && sym.data(ctx)->isTypeAlias()) {
                if (auto e = ctx.state.beginError(c->loc, core::errors::Resolver::ConstantInTypeAlias)) {
                    e.setHeader("Resolving constants through type aliases is not supported");
                }
                return core::Symbols::untyped();
            }
            if (!id->symbol.exists()) {
                // TODO: try to resolve if not resolved.
                return core::Symbols::noSymbol();
            }
            core::SymbolRef resolved = id->symbol.data(ctx)->dealias(ctx);
            core::SymbolRef result = resolved.data(ctx)->findMember(ctx, c->cnst);
            return result;
        } else {
            if (auto e = ctx.state.beginError(c->loc, core::errors::Resolver::DynamicConstant)) {
                e.setHeader("Dynamic constant references are unsupported");
            }
            return core::Symbols::untyped();
        }
    }

    // We have failed to resolve the constant. We'll need to report the error and stub it so that we can proceed
    static void stubOneConstantNesting(core::MutableContext ctx, ResolutionItem &job) {
        auto resolved = resolveConstant(ctx.withOwner(job.scope->scope), job.scope, job.out->original);
        if (resolved.exists() && resolved.data(ctx)->isTypeAlias()) {
            if (resolved.data(ctx)->resultType == nullptr) {
                // This is actually a use-site error, but we limit ourselves to emitting it once by checking resultType
                auto loc = resolved.data(ctx)->loc();
                if (auto e = ctx.state.beginError(loc, core::errors::Resolver::RecursiveTypeAlias)) {
                    e.setHeader("Unable to resolve right hand side of type alias `{}`", resolved.data(ctx)->show(ctx));
                    e.addErrorLine(job.out->original->loc, "Type alias used here");
                }
                resolved.data(ctx)->resultType = core::Types::untyped(ctx, resolved);
            }
            job.out->symbol = resolved;
            return;
        }
        ENFORCE(!resolved.exists());

        core::SymbolRef scope;
        if (job.out->symbol.exists()) {
            scope = job.out->symbol.data(ctx)->dealias(ctx);
        } else if (auto *id = ast::cast_tree<ast::ConstantLit>(job.out->original->scope.get())) {
            scope = id->symbol.data(ctx)->dealias(ctx);
        } else {
            scope = job.scope->scope;
        }

        auto customAutogenError = job.out->original->cnst == core::Symbols::Subclasses().data(ctx)->name;
        if (!scope.data(ctx)->isClass()) {
            // most likely an unresolved alias. Well, fill it in and emit an error
            if (auto e = ctx.state.beginError(job.out->original->loc, core::errors::Resolver::StubConstant)) {
                e.setHeader("Unable to resolve constant `{}`", job.out->original->cnst.show(ctx));
            }
            // as we're going to start adding definitions into it, we need some class to piggy back on
            scope =
                ctx.state.enterClassSymbol(job.out->loc, scope.data(ctx)->owner,
                                           ctx.state.enterNameConstant(ctx.state.freshNameUnique(
                                               core::UniqueNameKind::ResolverMissingClass, scope.data(ctx)->name, 1)));
            // as we've name-mangled the class, we have to manually create the next level of resolution
            auto createdSym = ctx.state.enterClassSymbol(job.out->loc, scope, job.out->original->cnst);
            job.out->symbol = createdSym;
        } else if (!scope.data(ctx)->derivesFrom(ctx, core::Symbols::StubClass()) || customAutogenError) {
            if (auto e = ctx.state.beginError(job.out->original->loc, core::errors::Resolver::StubConstant)) {
                e.setHeader("Unable to resolve constant `{}`", job.out->original->cnst.show(ctx));

                if (customAutogenError) {
                    e.addErrorSection(
                        core::ErrorSection("If this constant is generated by Autogen, you "
                                           "may need to re-generate the .rbi. Try running:\n"
                                           "  scripts/bin/remote-script sorbet/shim_generation/autogen.rb"));
                } else {
                    auto suggested = scope.data(ctx)->findMemberFuzzyMatch(ctx, job.out->original->cnst);
                    if (suggested.size() > 3) {
                        suggested.resize(3);
                    }
                    if (!suggested.empty()) {
                        vector<core::ErrorLine> lines;
                        for (auto suggestion : suggested) {
                            lines.emplace_back(core::ErrorLine::from(suggestion.symbol.data(ctx)->loc(),
                                                                     "Did you mean: `{}`?",
                                                                     suggestion.symbol.show(ctx)));
                        }
                        e.addErrorSection(core::ErrorSection(lines));
                    }
                }
            }
        }

        core::SymbolRef stub = ctx.state.enterClassSymbol(job.out->loc, scope, job.out->original->cnst);
        stub.data(ctx)->setSuperClass(core::Symbols::StubClass());
        stub.data(ctx)->resultType = core::Types::untypedUntracked();
        stub.data(ctx)->setIsModule(false);
        stub.data(ctx)->singletonClass(ctx).data(ctx)->resultType =
            core::Types::untypedUntracked(); // override result type of externalResultType. See also comment in
                                             // type_syntax.cc that intentionally side-steps this
    }

    static bool resolveJob(core::Context ctx, ResolutionItem &job) {
        if (isAlreadyResolved(ctx, *job.out)) {
            return true;
        }
        auto resolved = resolveConstant(ctx.withOwner(job.scope->scope), job.scope, job.out->original);
        if (!resolved.exists()) {
            return false;
        }
        if (resolved.data(ctx)->isTypeAlias()) {
            if (resolved.data(ctx)->resultType != nullptr) {
                // A TypeAliasResolutionItem job completed successfully,
                // or we forced the type alias this constant refers to to resolve.
                job.out->symbol = resolved;
                return true;
            }
            return false;
        }

        job.out->symbol = resolved;
        return true;
    }

    static bool resolveTypeAliasJob(core::MutableContext ctx, TypeAliasResolutionItem &job) {
        if (isFullyResolved(ctx, job.rhs)) {
            job.lhs.data(ctx)->resultType = TypeSyntax::getResultType(ctx, *(job.rhs), ParsedSig{}, true, job.lhs);
            return true;
        }

        return false;
    }

    static bool forceResolveJob(core::MutableContext ctx, ResolutionItem &job) {
        if (resolveJob(ctx, job)) {
            return true;
        }

        int depth = 0;
        while (depth < 256) {
            stubOneConstantNesting(ctx, job);
            if (resolveJob(ctx, job)) {
                return true;
            }
            depth++;
        }

        Exception::raise("Too many recursive calls trying to resolve constant:\n{}\n{}\n{}",
                         job.out->original->cnst.show(ctx), job.out->original->loc.file().data(ctx).path(),
                         job.out->original->loc.toString(ctx));
    }

    static bool resolveClassAliasJob(core::MutableContext ctx, ClassAliasResolutionItem &it) {
        auto rhsSym = it.rhs->symbol;
        if (!rhsSym.exists()) {
            return false;
        }

        auto rhsData = rhsSym.data(ctx);
        if (rhsData->isTypeAlias()) {
            if (auto e = ctx.state.beginError(it.rhs->loc, core::errors::Resolver::ReassignsTypeAlias)) {
                e.setHeader("Reassigning a type alias is not allowed");
                e.addErrorLine(rhsData->loc(), "Originally defined here");
                e.replaceWith(it.rhs->loc, "T.type_alias({})", it.rhs->loc.source(ctx));
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
            if (auto e = ctx.state.beginError(job.ancestor->loc, core::errors::Resolver::DynamicSuperclass)) {
                e.setHeader("Superclasses and mixin to unresolved type alias");
            }
            resolved = core::Symbols::StubAncestor();
        } else {
            resolved = ancestorSym.data(ctx)->dealias(ctx);
        }

        if (!resolved.data(ctx)->isClass()) {
            if (!lastRun) {
                return false;
            }
            if (auto e = ctx.state.beginError(job.ancestor->loc, core::errors::Resolver::DynamicSuperclass)) {
                e.setHeader("Superclasses and mixins may only use `T.type_alias`es to classes");
            }
            resolved = core::Symbols::StubAncestor();
        }

        if (resolved == job.klass || resolved.data(ctx)->derivesFrom(ctx, job.klass)) {
            if (auto e = ctx.state.beginError(job.ancestor->loc, core::errors::Resolver::CircularDependency)) {
                e.setHeader("Circular dependency: `{}` and `{}` are declared as parents of each other",
                            job.klass.data(ctx)->show(ctx), resolved.data(ctx)->show(ctx));
            }
            resolved = core::Symbols::StubAncestor();
        }

        if (job.isSuperclass) {
            if (resolved == core::Symbols::todo()) {
                // No superclass specified
            } else if (!job.klass.data(ctx)->superClass().exists() ||
                       job.klass.data(ctx)->superClass() == core::Symbols::todo() ||
                       job.klass.data(ctx)->superClass() == resolved) {
                job.klass.data(ctx)->setSuperClass(resolved);
            } else {
                if (auto e = ctx.state.beginError(job.ancestor->loc, core::errors::Resolver::RedefinitionOfParents)) {
                    e.setHeader("Class parents redefined for class `{}`", job.klass.data(ctx)->show(ctx));
                }
            }
        } else {
            ENFORCE(resolved.data(ctx)->isClass());
            job.klass.data(ctx)->mixins().emplace_back(resolved);
        }

        return true;
    }

    void transformAncestor(core::MutableContext ctx, core::SymbolRef klass, unique_ptr<ast::Expression> &ancestor,
                           bool isSuperclass = false) {
        if (auto *constScope = ast::cast_tree<ast::UnresolvedConstantLit>(ancestor.get())) {
            unique_ptr<ast::UnresolvedConstantLit> inner(constScope);
            ancestor.release();
            auto scopeTmp = nesting_;
            if (isSuperclass) {
                nesting_ = nesting_->parent;
            }
            ancestor = postTransformUnresolvedConstantLit(ctx, std::move(inner));
            nesting_ = scopeTmp;
        }
        AncestorResolutionItem job;
        job.klass = klass;
        job.isSuperclass = isSuperclass;

        if (auto *cnst = ast::cast_tree<ast::ConstantLit>(ancestor.get())) {
            auto sym = cnst->symbol;
            if (sym.exists() && sym.data(ctx)->isTypeAlias()) {
                if (auto e = ctx.state.beginError(cnst->loc, core::errors::Resolver::DynamicSuperclass)) {
                    e.setHeader("Superclasses and mixins may not be type aliases");
                }
                return;
            }
            ENFORCE(sym.exists() || ast::isa_tree<ast::ConstantLit>(cnst->original->scope.get()) ||
                    ast::isa_tree<ast::EmptyTree>(cnst->original->scope.get()));
            if (isSuperclass && sym == core::Symbols::todo()) {
                return;
            }
            job.ancestor = cnst;
        } else if (ancestor->isSelfReference()) {
            auto loc = ancestor->loc;
            auto enclosingClass = ctx.owner.data(ctx)->enclosingClass(ctx);
            auto nw = make_unique<ast::UnresolvedConstantLit>(loc, std::move(ancestor), enclosingClass.data(ctx)->name);
            auto out = make_unique<ast::ConstantLit>(loc, enclosingClass, std::move(nw));
            job.ancestor = out.get();
            ancestor = std::move(out);
        } else if (ast::isa_tree<ast::EmptyTree>(ancestor.get())) {
            return;
        } else {
            ENFORCE(false, "Namer should have not allowed this");
        }

        if (resolveAncestorJob(ctx, job, false)) {
            categoryCounterInc("resolve.constants.ancestor", "firstpass");
        } else {
            todoAncestors_.emplace_back(std::move(job));
        }
    }

public:
    ResolveConstantsWalk(core::MutableContext ctx) : nesting_(make_unique<Nesting>(nullptr, core::Symbols::root())) {}

    unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
        nesting_ = make_unique<Nesting>(std::move(nesting_), original->symbol);
        return original;
    }

    unique_ptr<ast::Expression> postTransformUnresolvedConstantLit(core::MutableContext ctx,
                                                                   unique_ptr<ast::UnresolvedConstantLit> c) {
        if (auto *constScope = ast::cast_tree<ast::UnresolvedConstantLit>(c->scope.get())) {
            unique_ptr<ast::UnresolvedConstantLit> inner(constScope);
            c->scope.release();
            c->scope = postTransformUnresolvedConstantLit(ctx, std::move(inner));
        }
        auto loc = c->loc;
        auto out = make_unique<ast::ConstantLit>(loc, core::Symbols::noSymbol(), std::move(c));
        ResolutionItem job{nesting_, out.get()};
        if (resolveJob(ctx, job)) {
            categoryCounterInc("resolve.constants.nonancestor", "firstpass");
        } else {
            todo_.emplace_back(std::move(job));
        }
        return out;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
        core::SymbolRef klass = original->symbol;

        for (auto &ancst : original->ancestors) {
            bool isSuperclass = (original->kind == ast::Class && &ancst == &original->ancestors.front() &&
                                 !klass.data(ctx)->isSingletonClass(ctx));
            transformAncestor(isSuperclass ? ctx : ctx.withOwner(klass), klass, ancst, isSuperclass);
        }

        auto singleton = klass.data(ctx)->singletonClass(ctx);
        for (auto &ancst : original->singletonAncestors) {
            transformAncestor(ctx.withOwner(klass), singleton, ancst);
        }

        nesting_ = nesting_->parent;
        return original;
    }

    unique_ptr<ast::Expression> postTransformAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        auto *id = ast::cast_tree<ast::ConstantLit>(asgn->lhs.get());
        if (id == nullptr || !id->symbol.dataAllowingNone(ctx)->isStaticField()) {
            return asgn;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn->rhs.get());
        if (send != nullptr && send->fun == core::Names::typeAlias() && send->args.size() == 1) {
            core::SymbolRef enclosingTypeMember;
            core::SymbolRef enclosingClass = ctx.owner.data(ctx)->enclosingClass(ctx);
            while (enclosingClass != core::Symbols::root()) {
                auto typeMembers = enclosingClass.data(ctx)->typeMembers();
                if (!typeMembers.empty()) {
                    enclosingTypeMember = typeMembers[0];
                    break;
                }
                enclosingClass = enclosingClass.data(ctx)->owner.data(ctx)->enclosingClass(ctx);
            }
            if (enclosingTypeMember.exists()) {
                if (auto e = ctx.state.beginError(id->loc, core::errors::Resolver::TypeAliasInGenericClass)) {
                    e.setHeader("Type aliases are not allowed in generic classes");
                    e.addErrorLine(enclosingTypeMember.data(ctx)->loc(), "Here is enclosing generic member");
                    auto sym = id->symbol;
                    sym.data(ctx)->resultType = core::Types::untyped(ctx, sym);
                }
            } else {
                auto typeAliasItem = TypeAliasResolutionItem{id->symbol, send->args[0].get()};
                if (resolveTypeAliasJob(ctx, typeAliasItem)) {
                    categoryCounterInc("resolve.constants.typealiases", "firstpass");
                } else {
                    this->todoTypeAliases_.emplace_back(std::move(typeAliasItem));
                }

                // We also enter a ResolutionItem for the lhs of a type alias so even if the type alias isn't used,
                // we'll still emit a warning when the rhs of a type alias doesn't resolve.
                auto item = ResolutionItem{nesting_, id};
                if (resolveJob(ctx, item)) {
                    categoryCounterInc("resolve.constants.typealiases", "firstpass");
                } else {
                    this->todo_.emplace_back(std::move(item));
                }
            }
            return asgn;
        }

        auto *rhs = ast::cast_tree<ast::ConstantLit>(asgn->rhs.get());
        if (rhs == nullptr) {
            return asgn;
        }

        auto item = ClassAliasResolutionItem{id->symbol, rhs};

        if (resolveClassAliasJob(ctx, item)) {
            categoryCounterInc("resolve.constants.aliases", "firstpass");
        } else {
            // TODO(perf) currently, by construction the last item in resolve todo list is the one this alias depends on
            // We may be able to get some perf by using this
            this->todoClassAliases_.emplace_back(std::move(item));
        }
        return asgn;
    }

    static bool compareLocs(core::Context ctx, core::Loc lhs, core::Loc rhs) {
        core::StrictLevel left = core::StrictLevel::Strong;
        core::StrictLevel right = core::StrictLevel::Strong;

        if (lhs.file().exists()) {
            left = lhs.file().data(ctx).strictLevel;
        }
        if (rhs.file().exists()) {
            right = rhs.file().data(ctx).strictLevel;
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
        while (scope->original && (scope = ast::cast_tree<ast::ConstantLit>(scope->original->scope.get()))) {
            depth += 1;
        }
        return depth;
    }

    static vector<ast::ParsedFile> resolveConstants(core::MutableContext ctx, vector<ast::ParsedFile> trees) {
        Timer timeit(ctx.state.errorQueue->logger, "resolver.resolve_constants");
        ResolveConstantsWalk constants(ctx);

        for (auto &tree : trees) {
            tree.tree = ast::TreeMap::apply(ctx, constants, std::move(tree.tree));
        }

        auto todo = std::move(constants.todo_);
        auto todoAncestors = std::move(constants.todoAncestors_);
        auto todoClassAliases = std::move(constants.todoClassAliases_);
        auto todoTypeAliases = std::move(constants.todoTypeAliases_);
        bool progress = true;

        while (!(todo.empty() && todoAncestors.empty()) && progress) {
            counterInc("resolve.constants.retries");
            {
                // This is an optimization. The order should not matter semantically
                // We try to resolve most ancestors second because this makes us much more likely to resolve everything
                // else.
                int origSize = todoAncestors.size();
                auto it =
                    remove_if(todoAncestors.begin(), todoAncestors.end(), [ctx](AncestorResolutionItem &job) -> bool {
                        return resolveAncestorJob(ctx, job, false);
                    });
                todoAncestors.erase(it, todoAncestors.end());
                progress = (origSize != todoAncestors.size());
                categoryCounterAdd("resolve.constants.ancestor", "retry", origSize - todoAncestors.size());
            }
            {
                int origSize = todo.size();
                auto it = remove_if(todo.begin(), todo.end(),
                                    [ctx](ResolutionItem &job) -> bool { return resolveJob(ctx, job); });
                todo.erase(it, todo.end());
                progress = progress || (origSize != todo.size());
                categoryCounterAdd("resolve.constants.nonancestor", "retry", origSize - todo.size());
            }
            {
                // This is an optimization. The order should not matter semantically
                // This is done as a "pre-step" because the first iteration of this effectively ran in TreeMap.
                // every item in todoClassAliases implicitly depends on an item in item in todo
                // there would be no point in running the todoClassAliases step before todo

                int origSize = todoClassAliases.size();
                auto it =
                    remove_if(todoClassAliases.begin(), todoClassAliases.end(),
                              [ctx](ClassAliasResolutionItem &it) -> bool { return resolveClassAliasJob(ctx, it); });
                todoClassAliases.erase(it, todoClassAliases.end());
                progress = progress || (origSize != todoClassAliases.size());
                categoryCounterAdd("resolve.constants.aliases", "retry", origSize - todoClassAliases.size());
            }
            {
                int origSize = todoTypeAliases.size();
                auto it =
                    remove_if(todoTypeAliases.begin(), todoTypeAliases.end(),
                              [ctx](TypeAliasResolutionItem &it) -> bool { return resolveTypeAliasJob(ctx, it); });
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
        fast_sort(todo, [ctx](const auto &lhs, const auto &rhs) -> bool {
            if (lhs.out->loc == rhs.out->loc) {
                return constantDepth(lhs.out) < constantDepth(rhs.out);
            }
            return compareLocs(ctx, lhs.out->loc, rhs.out->loc);
        });

        fast_sort(todoAncestors, [ctx](const auto &lhs, const auto &rhs) -> bool {
            if (lhs.ancestor->loc == rhs.ancestor->loc) {
                return constantDepth(lhs.ancestor) < constantDepth(rhs.ancestor);
            }
            return compareLocs(ctx, lhs.ancestor->loc, rhs.ancestor->loc);
        });

        // Note that this is missing alias stubbing, thus resolveJob needs to be able to handle missing aliases.

        for (auto &job : todo) {
            auto resolved = forceResolveJob(ctx, job);
            ENFORCE(resolved);
        }

        for (auto &job : todoAncestors) {
            auto resolved = resolveAncestorJob(ctx, job, true);
            if (!resolved) {
                resolved = resolveAncestorJob(ctx, job, true);
                ENFORCE(resolved);
            }
        }

        return trees;
    }
};

class ResolveSignaturesWalk {
private:
    void fillInInfoFromSig(core::MutableContext ctx, core::SymbolRef method, ast::Send *send, bool isOverloaded,
                           const ast::MethodDef &mdef) {
        ENFORCE(mdef.symbol.data(ctx)->isOverloaded() || mdef.symbol == method);
        ENFORCE(mdef.symbol.data(ctx)->isOverloaded() || method.data(ctx)->arguments().size() == mdef.args.size());
        auto exprLoc = send->loc;

        auto sig = TypeSyntax::parseSig(ctx, send, nullptr, true, method);

        if (!sig.seen.returns && !sig.seen.void_) {
            if (auto e = ctx.state.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`: No return type specified. Specify one with .returns()", "sig");
            }
        }
        if (sig.seen.returns && sig.seen.void_) {
            if (auto e = ctx.state.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`: Don't use both .returns() and .void", "sig");
            }
        }

        if (sig.seen.abstract) {
            method.data(ctx)->setAbstract();
        }
        if (sig.seen.implementation) {
            method.data(ctx)->setImplementation();
        }
        if (sig.seen.generated) {
            method.data(ctx)->setHasGeneratedSig();
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
        auto methodInfo = method.data(ctx);

        methodInfo->resultType = sig.returns;
        auto argIt = mdef.args.begin();
        for (auto it = methodInfo->arguments().begin(); it != methodInfo->arguments().end(); /* nothing */) {
            core::SymbolRef arg = *it;
            const auto &argTree = *argIt;
            const auto local = ast::MK::arg2Local(argTree.get());
            auto treeArgName = local->localVariable._name;
            ENFORCE(local != nullptr);
            auto spec = absl::c_find_if(sig.argTypes, [&](auto &spec) { return spec.name == treeArgName; });
            if (spec != sig.argTypes.end()) {
                ENFORCE(spec->type != nullptr);
                arg.data(ctx)->resultType = spec->type;
                arg.data(ctx)->addLoc(ctx, spec->loc);
                arg.data(ctx)->setReBind(spec->rebind);
                sig.argTypes.erase(spec);
                ++it;
                ++argIt;
            } else if (isOverloaded) {
                if (arg.data(ctx)->isBlockArgument()) {
                    // It's ok for an overloaded sig to not mention the implicit block arg:
                    // the method argument symbol for the block should still appear on the overload,
                    // but we should make the loc for it not exist.
                    const auto blkLoc = core::Loc::none(arg.data(ctx)->loc().file());
                    arg.data(ctx)->addLoc(ctx, blkLoc);
                    ++it;
                } else {
                    it = methodInfo->arguments().erase(it);
                }
                ++argIt;
            } else if (arg.data(ctx)->resultType != nullptr) {
                ++it;
                ++argIt;
            } else {
                arg.data(ctx)->resultType = core::Types::untyped(ctx, arg);
                // We silence the "type not specified" error when a sig does not mention the synthesized block arg.
                bool isBlkArg = arg.data(ctx)->name == core::Names::blkArg();
                if (!isBlkArg && (sig.seen.params || sig.seen.returns || sig.seen.void_)) {
                    // Only error if we have any types
                    if (auto e = ctx.state.beginError(arg.data(ctx)->loc(),
                                                      core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`. Type not specified for argument `{}`", "sig",
                                    treeArgName.show(ctx));
                        e.addErrorLine(send->block->loc, "Signature");
                    }
                }
                ++it;
                ++argIt;
            }

            if (isOverloaded && arg.data(ctx)->isKeyword()) {
                if (auto e =
                        ctx.state.beginError(arg.data(ctx)->loc(), core::errors::Resolver::InvalidMethodSignature)) {
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
    }

    // In order to check a default argument that looks like
    //
    //     sig {params(x: T)}
    //     def foo(x: <expr>)
    //       ...
    //     end
    //
    // we elaborate the method definition to
    //
    //     def foo(x: <expr>)
    //       T.let(<expr>, T)
    //       ...
    //     end
    //
    // which will then get checked later on in the pipeline.
    void injectOptionalArgs(core::MutableContext ctx, ast::MethodDef *mdef) {
        ast::InsSeq::STATS_store lets;

        if (mdef->symbol.data(ctx)->isAbstract()) {
            // TODO(jez) Check that abstract methods don't have defined bodies earlier (currently done in infer)
            // so that we can unblock checking default arguments of abstract methods
            return;
        }

        int i = -1;
        for (auto argSym : mdef->symbol.data(ctx)->arguments()) {
            i++;
            auto &argExp = mdef->args[i];
            auto argType = argSym.data(ctx)->resultType;

            if (auto *optArgExp = ast::cast_tree<ast::OptionalArg>(argExp.get())) {
                // Using optArgExp's loc will make errors point to the arg list, even though the T.let is in the body.
                auto let = make_unique<ast::Cast>(optArgExp->loc, argType, optArgExp->default_->deepCopy(),
                                                  core::Names::let());
                lets.emplace_back(std::move(let));
            }
        }

        if (lets.size() > 0) {
            auto loc = mdef->rhs->loc;
            mdef->rhs = ast::MK::InsSeq(loc, std::move(lets), std::move(mdef->rhs));
        }
    }

    void processClassBody(core::MutableContext ctx, unique_ptr<ast::ClassDef> &klass) {
        InlinedVector<ast::Send *, 1> lastSigs;
        for (auto &stat : klass->rhs) {
            processStatement(ctx, stat, lastSigs);
        }
        if (!lastSigs.empty()) {
            if (auto e = ctx.state.beginError(lastSigs[0]->loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`. No method def following it", "sig");
            }
        }

        auto toRemove = remove_if(klass->rhs.begin(), klass->rhs.end(),
                                  [](unique_ptr<ast::Expression> &stat) -> bool { return stat.get() == nullptr; });
        klass->rhs.erase(toRemove, klass->rhs.end());
    }

    void processInSeq(core::MutableContext ctx, unique_ptr<ast::InsSeq> &seq) {
        InlinedVector<ast::Send *, 1> lastSigs;
        for (auto &stat : seq->stats) {
            processStatement(ctx, stat, lastSigs);
        }
        if (!ast::isa_tree<ast::EmptyTree>(seq->expr.get())) {
            processStatement(ctx, seq->expr, lastSigs);
        }
        if (!lastSigs.empty()) {
            if (auto e = ctx.state.beginError(lastSigs[0]->loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`. No method def following it", "sig");
            }
        }
        auto toRemove = remove_if(seq->stats.begin(), seq->stats.end(),
                                  [](unique_ptr<ast::Expression> &stat) -> bool { return stat.get() == nullptr; });
        seq->stats.erase(toRemove, seq->stats.end());
    }

    void processStatement(core::MutableContext ctx, unique_ptr<ast::Expression> &stat,
                          InlinedVector<ast::Send *, 1> &lastSigs) {
        typecase(
            stat.get(),

            [&](ast::Send *send) {
                if (TypeSyntax::isSig(ctx, send)) {
                    if (!lastSigs.empty()) {
                        if (!ctx.permitOverloadDefinitions()) {
                            if (auto e = ctx.state.beginError(lastSigs[0]->loc,
                                                              core::errors::Resolver::InvalidMethodSignature)) {
                                e.setHeader("Unused type annotation. No method def before next annotation");
                                e.addErrorLine(send->loc, "Type annotation that will be used instead");
                            }
                        }
                    }

                    // parsing the sig will transform the sig into a format we can use
                    TypeSyntax::parseSig(ctx, send, nullptr, true, core::Symbols::untyped());

                    lastSigs.emplace_back(send);
                    return;
                }
                return;
            },

            [&](ast::MethodDef *mdef) {
                if (debug_mode) {
                    bool hasSig = !lastSigs.empty();
                    bool DSL = mdef->isDSLSynthesized();
                    bool isRBI = mdef->loc.file().data(ctx).isRBI();
                    if (hasSig) {
                        categoryCounterInc("method.sig", "true");
                    } else {
                        categoryCounterInc("method.sig", "false");
                    }
                    if (DSL) {
                        categoryCounterInc("method.dsl", "true");
                    } else {
                        categoryCounterInc("method.dsl", "false");
                    }
                    if (isRBI) {
                        categoryCounterInc("method.rbi", "true");
                    } else {
                        categoryCounterInc("method.rbi", "false");
                    }
                    if (hasSig && !isRBI && !DSL) {
                        counterInc("types.sig.human");
                    }
                }

                if (!lastSigs.empty()) {
                    prodCounterInc("types.sig.count");

                    bool isOverloaded = lastSigs.size() > 1 && ctx.permitOverloadDefinitions();

                    if (isOverloaded) {
                        mdef->symbol.data(ctx)->setOverloaded();
                        int i = 1;

                        while (i < lastSigs.size()) {
                            auto overload = ctx.state.enterNewMethodOverload(lastSigs[i]->loc, mdef->symbol, i);
                            fillInInfoFromSig(ctx, overload, lastSigs[i], isOverloaded, *mdef);
                            if (i + 1 < lastSigs.size()) {
                                overload.data(ctx)->setOverloaded();
                            }
                            i++;
                        }
                    }

                    fillInInfoFromSig(ctx, mdef->symbol, lastSigs[0], isOverloaded, *mdef);

                    if (!isOverloaded) {
                        injectOptionalArgs(ctx, mdef);
                    }

                    // OVERLOAD
                    lastSigs.clear();
                }

                if (mdef->symbol.data(ctx)->isAbstract()) {
                    if (!ast::isa_tree<ast::EmptyTree>(mdef->rhs.get())) {
                        if (auto e =
                                ctx.state.beginError(mdef->rhs->loc, core::errors::Resolver::AbstractMethodWithBody)) {
                            e.setHeader("Abstract methods must not contain any code in their body");
                        }

                        mdef->rhs = ast::MK::EmptyTree();
                    }
                    if (!mdef->symbol.data(ctx)->enclosingClass(ctx).data(ctx)->isClassAbstract()) {
                        if (auto e = ctx.state.beginError(mdef->loc,
                                                          core::errors::Resolver::AbstractMethodOutsideAbstract)) {
                            e.setHeader("Before declaring an abstract method, you must mark your class/module "
                                        "as abstract using `abstract!` or `interface!`");
                        }
                    }
                } else if (mdef->symbol.data(ctx)->enclosingClass(ctx).data(ctx)->isClassInterface()) {
                    if (auto e = ctx.state.beginError(mdef->loc, core::errors::Resolver::ConcreteMethodInInterface)) {
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
    core::TypePtr resolveConstantType(core::MutableContext ctx, unique_ptr<ast::Expression> &expr,
                                      core::SymbolRef ofSym) {
        core::TypePtr result;
        typecase(
            expr.get(), [&](ast::Literal *a) { result = a->value; },
            [&](ast::Cast *cast) {
                if (cast->cast != core::Names::let()) {
                    if (auto e = ctx.state.beginError(cast->loc, core::errors::Resolver::ConstantAssertType)) {
                        e.setHeader("Use T.let() to specify the type of constants");
                    }
                }
                result = cast->type;
            },
            [&](ast::InsSeq *outer) { result = resolveConstantType(ctx, outer->expr, ofSym); },
            [&](ast::Expression *expr) {
                result = core::Types::untyped(ctx, ofSym);
                if (auto *send = ast::cast_tree<ast::Send>(expr)) {
                    if (send->fun == core::Names::typeAlias()) {
                        // short circuit if this is a type alias
                        return;
                    }
                }
                if (auto e = ctx.state.beginError(expr->loc, core::errors::Resolver::ConstantMissingTypeAnnotation)) {
                    e.setHeader("Constants must have type annotations with T.let() when specifying '# typed: strict'");
                }
            });
        return result;
    }

    bool handleDeclaration(core::MutableContext ctx, unique_ptr<ast::Assign> &asgn) {
        auto *uid = ast::cast_tree<ast::UnresolvedIdent>(asgn->lhs.get());
        if (uid == nullptr) {
            return false;
        }

        if (uid->kind != ast::UnresolvedIdent::Instance && uid->kind != ast::UnresolvedIdent::Class) {
            return false;
        }
        ast::Expression *recur = asgn->rhs.get();
        while (auto outer = ast::cast_tree<ast::InsSeq>(recur)) {
            recur = outer->expr.get();
        }

        auto *cast = ast::cast_tree<ast::Cast>(recur);
        if (cast == nullptr) {
            return false;
        } else if (cast->cast != core::Names::let()) {
            if (auto e = ctx.state.beginError(cast->loc, core::errors::Resolver::ConstantAssertType)) {
                e.setHeader("Use T.let() to specify the type of constants");
            }
        }

        core::SymbolRef scope;
        if (uid->kind == ast::UnresolvedIdent::Class) {
            if (!ctx.owner.data(ctx)->isClass()) {
                if (auto e = ctx.state.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                    e.setHeader("Class variables must be declared at class scope");
                }
            }

            scope = ctx.owner.data(ctx)->enclosingClass(ctx);
        } else {
            if (ctx.owner.data(ctx)->isClass()) {
                // Declaring a class instance variable
            } else {
                // Inside a method; declaring a normal instance variable
                if (ctx.owner.data(ctx)->name != core::Names::initialize()) {
                    if (auto e = ctx.state.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                        e.setHeader("Instance variables must be declared inside `initialize`");
                    }
                }
            }
            scope = ctx.selfClass();
        }

        auto prior = scope.data(ctx)->findMember(ctx, uid->name);
        if (prior.exists()) {
            if (core::Types::equiv(ctx, prior.data(ctx)->resultType, cast->type)) {
                // We already have a symbol for this field, and it matches what we already saw, so we can short circuit.
                return true;
            } else {
                if (auto e = ctx.state.beginError(uid->loc, core::errors::Resolver::DuplicateVariableDeclaration)) {
                    e.setHeader("Redeclaring variable `{}` with mismatching type", uid->name.data(ctx)->show(ctx));
                    e.addErrorLine(prior.data(ctx)->loc(), "Previous declaration is here:");
                }
                return false;
            }
        }
        core::SymbolRef var;

        if (uid->kind == ast::UnresolvedIdent::Class) {
            var = ctx.state.enterStaticFieldSymbol(uid->loc, scope, uid->name);
        } else {
            var = ctx.state.enterFieldSymbol(uid->loc, scope, uid->name);
        }

        var.data(ctx)->resultType = cast->type;
        return true;
    }

    core::SymbolRef methodOwner(core::MutableContext ctx) {
        core::SymbolRef owner = ctx.owner.data(ctx)->enclosingClass(ctx);
        if (owner == core::Symbols::root()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }

public:
    unique_ptr<ast::Assign> postTransformAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        if (handleDeclaration(ctx, asgn)) {
            return asgn;
        }

        auto *id = ast::cast_tree<ast::ConstantLit>(asgn->lhs.get());
        if (id == nullptr || !id->symbol.exists()) {
            return asgn;
        }

        auto sym = id->symbol;
        auto data = sym.data(ctx);
        if (data->isTypeAlias()) {
            return asgn;
        }

        if (data->isTypeMember()) {
            ENFORCE(data->isFixed());
            auto send = ast::cast_tree<ast::Send>(asgn->rhs.get());
            ENFORCE(send->recv->isSelfReference());
            ENFORCE(send->fun == core::Names::typeMember() || send->fun == core::Names::typeTemplate());
            int arg;
            if (send->args.size() == 1) {
                arg = 0;
            } else if (send->args.size() == 2) {
                arg = 1;
            } else {
                Exception::raise("Wrong arg count");
            }

            auto *hash = ast::cast_tree<ast::Hash>(send->args[arg].get());
            if (hash) {
                int i = -1;
                for (auto &keyExpr : hash->keys) {
                    i++;
                    auto lit = ast::cast_tree<ast::Literal>(keyExpr.get());
                    if (lit && lit->isSymbol(ctx) && lit->asSymbol(ctx) == core::Names::fixed()) {
                        ParsedSig emptySig;
                        data->resultType = TypeSyntax::getResultType(ctx, *(hash->values[i]), emptySig, false, sym);
                    }
                }
            }
        } else if (data->isStaticField() && data->resultType == nullptr) {
            data->resultType = resolveConstantType(ctx, asgn->rhs, sym);
        }

        return asgn;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
        processClassBody(ctx.withOwner(original->symbol), original);
        return original;
    }

    unique_ptr<ast::Expression> postTransformInsSeq(core::MutableContext ctx, unique_ptr<ast::InsSeq> original) {
        processInSeq(ctx, original);
        return original;
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> send) {
        if (auto *id = ast::cast_tree<ast::ConstantLit>(send->recv.get())) {
            if (id->symbol != core::Symbols::T()) {
                return send;
            }
            switch (send->fun._id) {
                case core::Names::let()._id:
                case core::Names::assertType()._id:
                case core::Names::cast()._id: {
                    if (send->args.size() < 2) {
                        return send;
                    }

                    auto expr = std::move(send->args[0]);
                    ParsedSig emptySig;
                    auto type =
                        TypeSyntax::getResultType(ctx, *(send->args[1]), emptySig, false, core::Symbols::noSymbol());
                    return ast::MK::InsSeq1(send->loc, ast::MK::KeepForTypechecking(std::move(send->args[1])),
                                            make_unique<ast::Cast>(send->loc, type, std::move(expr), send->fun));
                }
                default:
                    return send;
            }
        } else if (send->recv.get()->isSelfReference()) {
            if (send->fun != core::Names::aliasMethod()) {
                return send;
            }

            vector<core::NameRef> args;
            for (auto &arg : send->args) {
                auto lit = ast::cast_tree<ast::Literal>(arg.get());
                if (lit == nullptr || !lit->isSymbol(ctx)) {
                    continue;
                }
                core::NameRef name = lit->asSymbol(ctx);

                args.emplace_back(name);
            }
            if (send->args.size() != 2) {
                return send;
            }
            if (args.size() != 2) {
                return send;
            }

            auto fromName = args[0];
            auto toName = args[1];

            auto owner = methodOwner(ctx);
            core::SymbolRef toMethod = owner.data(ctx)->findMember(ctx, toName);
            if (!toMethod.exists()) {
                if (auto e = ctx.state.beginError(send->args[1]->loc, core::errors::Resolver::BadAliasMethod)) {
                    e.setHeader("Can't make method alias from `{}` to non existing method `{}`", fromName.show(ctx),
                                toName.show(ctx));
                }
                toMethod = core::Symbols::RubyTyper_badAliasMethodStub();
            }

            core::SymbolRef fromMethod = owner.data(ctx)->findMemberNoDealias(ctx, fromName);
            if (fromMethod.exists() && fromMethod.data(ctx)->dealias(ctx) != toMethod) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::BadAliasMethod)) {
                    auto dealiased = fromMethod.data(ctx)->dealias(ctx);
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
                return send;
            }

            core::SymbolRef alias = ctx.state.enterMethodSymbol(send->loc, owner, fromName);
            alias.data(ctx)->resultType = core::make_type<core::AliasType>(toMethod);

            return send;
        } else {
            return send;
        }
    }
};

class FlattenWalk {
private:
    bool isDefinition(core::MutableContext ctx, const unique_ptr<ast::Expression> &what) {
        if (ast::isa_tree<ast::MethodDef>(what.get())) {
            return true;
        }
        if (ast::isa_tree<ast::ClassDef>(what.get())) {
            return true;
        }

        if (auto asgn = ast::cast_tree<ast::Assign>(what.get())) {
            return ast::isa_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
        }
        return false;
    }

    unique_ptr<ast::Expression> extractClassInit(core::MutableContext ctx, unique_ptr<ast::ClassDef> &klass) {
        ast::InsSeq::STATS_store inits;

        for (auto it = klass->rhs.begin(); it != klass->rhs.end(); /* nothing */) {
            if (isDefinition(ctx, *it)) {
                ++it;
                continue;
            }
            inits.emplace_back(std::move(*it));
            it = klass->rhs.erase(it);
        }

        if (inits.empty()) {
            return nullptr;
        }
        if (inits.size() == 1) {
            return std::move(inits.front());
        }
        return make_unique<ast::InsSeq>(klass->declLoc, std::move(inits), make_unique<ast::EmptyTree>());
    }

public:
    FlattenWalk() {
        newMethodSet();
    }
    ~FlattenWalk() {
        ENFORCE(methodScopes.empty());
        ENFORCE(classes.empty());
        ENFORCE(classStack.empty());
    }

    unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> classDef) {
        newMethodSet();
        classStack.emplace_back(classes.size());
        classes.emplace_back();

        auto inits = extractClassInit(ctx, classDef);
        if (inits == nullptr) {
            return classDef;
        }

        core::SymbolRef sym;
        if (classDef->symbol == core::Symbols::root()) {
            // Every file may have its own top-level code, so uniqify the names.
            //
            // NOTE(nelhage): In general, we potentially need to do this for
            // every class, since Ruby allows reopening classes. However, since
            // pay-server bans that behavior, this should be OK here.
            sym = ctx.state.staticInitForFile(inits->loc);
        } else {
            sym = ctx.state.staticInitForClass(classDef->symbol, inits->loc);
        }
        ENFORCE(!sym.data(ctx)->arguments().empty(), "<static-init> method should already have a block arg symbol: {}",
                sym.data(ctx)->show(ctx));
        ENFORCE(sym.data(ctx)->arguments().back().data(ctx)->isBlockArgument(),
                "Last argument symbol is not a block arg: {}" + sym.data(ctx)->show(ctx));

        // Synthesize a block argument for this <static-init> block. This is rather fiddly,
        // because we have to know exactly what invariants desugar and namer set up about
        // methods and block arguments before us.
        auto blkLoc = core::Loc::none(inits->loc.file());
        core::LocalVariable blkLocalVar(core::Names::blkArg(), 0);
        ast::MethodDef::ARGS_store args;
        args.emplace_back(make_unique<ast::Local>(blkLoc, blkLocalVar));

        auto init = make_unique<ast::MethodDef>(inits->loc, inits->loc, sym, core::Names::staticInit(), std::move(args),
                                                std::move(inits), true);

        classDef->rhs.emplace_back(std::move(init));

        return classDef;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> methodDef) {
        auto &methods = curMethodSet();
        methods.stack.emplace_back(methods.methods.size());
        methods.methods.emplace_back();
        return methodDef;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> classDef) {
        ENFORCE(!classStack.empty());
        ENFORCE(classes.size() > classStack.back());
        ENFORCE(classes[classStack.back()] == nullptr);

        classDef->rhs = addMethods(ctx, std::move(classDef->rhs));
        classes[classStack.back()] = std::move(classDef);
        classStack.pop_back();
        return make_unique<ast::EmptyTree>();
    };

    unique_ptr<ast::Expression> postTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> methodDef) {
        auto &methods = curMethodSet();
        ENFORCE(!methods.stack.empty());
        ENFORCE(methods.methods.size() > methods.stack.back());
        ENFORCE(methods.methods[methods.stack.back()] == nullptr);

        methods.methods[methods.stack.back()] = std::move(methodDef);
        methods.stack.pop_back();
        return make_unique<ast::EmptyTree>();
    };

    unique_ptr<ast::Expression> addClasses(core::MutableContext ctx, unique_ptr<ast::Expression> tree) {
        if (classes.empty()) {
            ENFORCE(sortedClasses().empty());
            return tree;
        }
        if (classes.size() == 1 && (ast::cast_tree<ast::EmptyTree>(tree.get()) != nullptr)) {
            // It was only 1 class to begin with, put it back
            return std::move(sortedClasses()[0]);
        }

        auto insSeq = ast::cast_tree<ast::InsSeq>(tree.get());
        if (insSeq == nullptr) {
            ast::InsSeq::STATS_store stats;
            auto sorted = sortedClasses();
            stats.insert(stats.begin(), make_move_iterator(sorted.begin()), make_move_iterator(sorted.end()));
            return ast::MK::InsSeq(tree->loc, std::move(stats), std::move(tree));
        }

        for (auto &clas : sortedClasses()) {
            ENFORCE(!!clas);
            insSeq->stats.emplace_back(std::move(clas));
        }
        return tree;
    }

    unique_ptr<ast::Expression> addMethods(core::MutableContext ctx, unique_ptr<ast::Expression> tree) {
        auto &methods = curMethodSet().methods;
        if (methods.empty()) {
            ENFORCE(popCurMethodDefs().empty());
            return tree;
        }
        if (methods.size() == 1 && (ast::cast_tree<ast::EmptyTree>(tree.get()) != nullptr)) {
            // It was only 1 method to begin with, put it back
            unique_ptr<ast::Expression> methodDef = std::move(popCurMethodDefs()[0]);
            return methodDef;
        }

        auto insSeq = ast::cast_tree<ast::InsSeq>(tree.get());
        if (insSeq == nullptr) {
            ast::InsSeq::STATS_store stats;
            tree = make_unique<ast::InsSeq>(tree->loc, std::move(stats), std::move(tree));
            return addMethods(ctx, std::move(tree));
        }

        for (auto &method : popCurMethodDefs()) {
            ENFORCE(!!method);
            insSeq->stats.emplace_back(std::move(method));
        }
        return tree;
    }

private:
    vector<unique_ptr<ast::ClassDef>> sortedClasses() {
        ENFORCE(classStack.empty());
        auto ret = std::move(classes);
        classes.clear();
        return ret;
    }

    ast::ClassDef::RHS_store addMethods(core::MutableContext ctx, ast::ClassDef::RHS_store rhs) {
        if (curMethodSet().methods.size() == 1 && rhs.size() == 1 &&
            (ast::cast_tree<ast::EmptyTree>(rhs[0].get()) != nullptr)) {
            // It was only 1 method to begin with, put it back
            rhs.pop_back();
            rhs.emplace_back(std::move(popCurMethodDefs()[0]));
            return rhs;
        }
        for (auto &method : popCurMethodDefs()) {
            ENFORCE(method.get() != nullptr);
            rhs.emplace_back(std::move(method));
        }
        return rhs;
    }

    vector<unique_ptr<ast::MethodDef>> popCurMethodDefs() {
        auto ret = std::move(curMethodSet().methods);
        ENFORCE(curMethodSet().stack.empty());
        popCurMethodSet();
        return ret;
    };

    struct Methods {
        vector<unique_ptr<ast::MethodDef>> methods;
        vector<int> stack;
        Methods() = default;
    };
    void newMethodSet() {
        methodScopes.emplace_back();
    }
    Methods &curMethodSet() {
        ENFORCE(!methodScopes.empty());
        return methodScopes.back();
    }
    void popCurMethodSet() {
        ENFORCE(!methodScopes.empty());
        methodScopes.pop_back();
    }

    // We flatten nested classes and methods into a flat list. We want to sort
    // them by their starts, so that `class A; class B; end; end` --> `class A;
    // end; class B; end`.
    //
    // In order to make TreeMap work out, we can't remove them from the AST
    // until the `postTransform*` hook. Appending them to a list at that point
    // would result in an "bottom-up" ordering, so instead we store a stack of
    // "where does the next definition belong" into `classStack` and
    // `methodScopes.stack`, which we push onto in the `preTransform* hook, and
    // pop from in the `postTransform` hook.

    vector<Methods> methodScopes;
    vector<unique_ptr<ast::ClassDef>> classes;
    vector<int> classStack;
};

class ResolveMixesInClassMethodsWalk {
    void processMixesInClassMethods(core::MutableContext ctx, ast::Send *send) {
        if (!ctx.owner.data(ctx)->isClass() || !ctx.owner.data(ctx)->isClassModule()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` can only be declared inside a module, not a class", send->fun.data(ctx)->show(ctx));
            }
            // Keep processing it anyways
        }

        if (send->args.size() != 1) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Wrong number of arguments to `{}`: Expected: `{}`, got: `{}`",
                            send->fun.data(ctx)->show(ctx), 1, send->args.size());
            }
            return;
        }
        auto *front = send->args.front().get();
        auto *id = ast::cast_tree<ast::ConstantLit>(front);
        if (id == nullptr || !id->symbol.exists() || !id->symbol.data(ctx)->isClass()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Argument to `{}` must be statically resolvable to a module",
                            send->fun.data(ctx)->show(ctx));
            }
            return;
        }
        if (id->symbol.data(ctx)->isClassClass()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` is a class, not a module; Only modules may be mixins",
                            id->symbol.data(ctx)->show(ctx));
            }
            return;
        }
        if (id->symbol == ctx.owner) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Must not pass your self to `{}`", send->fun.data(ctx)->show(ctx));
            }
            return;
        }
        auto existing = ctx.owner.data(ctx)->findMember(ctx, core::Names::classMethods());
        if (existing.exists() && existing != id->symbol) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Redeclaring `{}` from module `{}` to module `{}`", send->fun.data(ctx)->show(ctx),
                            existing.data(ctx)->show(ctx), id->symbol.data(ctx)->show(ctx));
            }
            return;
        }
        ctx.owner.data(ctx)->members[core::Names::classMethods()] = id->symbol;
    }

public:
    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> original) {
        if (original->recv->isSelfReference() && original->fun == core::Names::mixesInClassMethods()) {
            processMixesInClassMethods(ctx, original.get());
            return ast::MK::EmptyTree();
        }
        return original;
    }
};

class ResolveSanityCheckWalk {
public:
    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
        ENFORCE(original->symbol != core::Symbols::todo(), "These should have all been resolved: {}",
                original->toString(ctx));
        return original;
    }
    unique_ptr<ast::Expression> postTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> original) {
        ENFORCE(original->symbol != core::Symbols::todo(), "These should have all been resolved: {}",
                original->toString(ctx));
        return original;
    }
    unique_ptr<ast::Expression> postTransformUnresolvedConstantLit(core::MutableContext ctx,
                                                                   unique_ptr<ast::UnresolvedConstantLit> original) {
        ENFORCE(false, "These should have all been removed: {}", original->toString(ctx));
        return original;
    }
    unique_ptr<ast::Expression> postTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> original) {
        ENFORCE(original->symbol != core::Symbols::todo(), "These should have all been resolved: {}",
                original->toString(ctx));
        return original;
    }
    unique_ptr<ast::ConstantLit> postTransformConstantLit(core::MutableContext ctx,
                                                          unique_ptr<ast::ConstantLit> original) {
        ENFORCE(ResolveConstantsWalk::isAlreadyResolved(ctx, *original));
        return original;
    }
};
}; // namespace

vector<ast::ParsedFile> Resolver::run(core::MutableContext ctx, vector<ast::ParsedFile> trees) {
    trees = ResolveConstantsWalk::resolveConstants(ctx, std::move(trees));
    finalizeAncestors(ctx.state);
    trees = resolveMixesInClassMethods(ctx, std::move(trees));
    finalizeSymbols(ctx.state);
    trees = resolveSigs(ctx, std::move(trees));
    validateSymbols(ctx.state);
    sanityCheck(ctx, trees);

    return trees;
}

vector<ast::ParsedFile> Resolver::resolveSigs(core::MutableContext ctx, vector<ast::ParsedFile> trees) {
    ResolveSignaturesWalk sigs;
    Timer timeit(ctx.state.errorQueue->logger, "resolver.sigs_vars_and_flatten");
    for (auto &tree : trees) {
        tree.tree = ast::TreeMap::apply(ctx, sigs, std::move(tree.tree));

        // declared in here since it holds onto state
        FlattenWalk flatten;
        tree.tree = ast::TreeMap::apply(ctx, flatten, std::move(tree.tree));
        tree.tree = flatten.addClasses(ctx, std::move(tree.tree));
        tree.tree = flatten.addMethods(ctx, std::move(tree.tree));
    }

    return trees;
}

vector<ast::ParsedFile> Resolver::resolveMixesInClassMethods(core::MutableContext ctx, vector<ast::ParsedFile> trees) {
    ResolveMixesInClassMethodsWalk mixesInClassMethods;
    Timer timeit(ctx.state.errorQueue->logger, "resolver.mixes_in_class_methods");
    for (auto &tree : trees) {
        tree.tree = ast::TreeMap::apply(ctx, mixesInClassMethods, std::move(tree.tree));
    }
    return trees;
}

void Resolver::sanityCheck(core::MutableContext ctx, vector<ast::ParsedFile> &trees) {
    if (debug_mode) {
        Timer timeit(ctx.state.errorQueue->logger, "resolver.sanity_check");
        ResolveSanityCheckWalk sanity;
        for (auto &tree : trees) {
            tree.tree = ast::TreeMap::apply(ctx, sanity, std::move(tree.tree));
        }
    }
}

vector<ast::ParsedFile> Resolver::runTreePasses(core::MutableContext ctx, vector<ast::ParsedFile> trees) {
    trees = ResolveConstantsWalk::resolveConstants(ctx, std::move(trees));
    trees = resolveMixesInClassMethods(ctx, std::move(trees));
    trees = resolveSigs(ctx, std::move(trees));
    sanityCheck(ctx, trees);
    // This check is FAR too slow to run on large codebases, especially with sanitizers on.
    // But it can be super useful to uncomment when debugging certain issues.
    // ctx.state.sanityCheck();

    return trees;
}

vector<ast::ParsedFile> Resolver::runConstantResolution(core::MutableContext ctx, vector<ast::ParsedFile> trees) {
    trees = ResolveConstantsWalk::resolveConstants(ctx, std::move(trees));
    sanityCheck(ctx, trees);

    return trees;
}

} // namespace sorbet::resolver
