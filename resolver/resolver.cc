#include "core/errors/resolver.h"
#include "ast/Helpers.h"
#include "ast/Trees.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/Errors.h"
#include "core/Names.h"
#include "core/StrictLevel.h"
#include "core/core.h"
#include "resolver/resolver.h"
#include "resolver/type_syntax.h"

#include "absl/strings/str_cat.h"
#include "common/Timer.h"
#include "core/Symbols.h"

#include <algorithm> // find_if
#include <utility>
#include <vector>

using namespace std;

namespace sorbet::resolver {
namespace {

/*
 * Ruby supports resolving constants via your ancestors -- superclasses and
 * mixins. Since superclass and mixins are themselves constant references, we
 * thus may not be able to resolve certain constants until after we've typeAlias
 * others.
 *
 * To solve this, we collect any failed resolutions into a pair of TODO lists,
 * and repeatedly walk them until we succeed or stop making progress. In
 * practice this loop terminates after 3 or fewer passes on most real codebases.
 * We also tack defined type aliases in a separate map.
 *
 * This walk replaces ast::UnresolvedConstantLit nodes with either ast::ConstantLit nodes.
 * Successful resolutions are removed from the `todo_` lists.
 *
 * There are 4 items maintained by this fixed point computations:
 *  - list of constants to be typeAlias
 *  - list of ancestors to be filled that require constants to be typeAlias
 *  - map of class aliases
 *  - map of type aliases to their respective trees
 *
 * We track the latter ancestors separately for the dual reasons that (1) Upon
 * successful resolution, we need to do additional work (mutating the symbol
 * table to reflect the new ancestors) and (2) Resolving those constants
 * potentially renders additional constants resolvable, and so if any constant
 * on the second list succeeds, we need to keep looping in the outer loop.
 *
 * We also track aliases to constants that we were not yet able to resolve and
 * define those aliases when their right hand sides resolve successfully.
 *
 * The resolveConstants static method function at the bottom of this class
 * implements the iterate-to-fixpoint outer loop.
 */

class ResolveConstantsWalk {
private:
    struct Nesting {
        shared_ptr<Nesting> parent;
        core::SymbolRef scope;

        Nesting(shared_ptr<Nesting> parent, core::SymbolRef scope) : parent(move(parent)), scope(scope) {}
    };
    shared_ptr<Nesting> nesting_;

    struct ResolutionItem {
        shared_ptr<Nesting> scope;
        // setting out->typeAlias to non-nullptr indicates that the tree was already typeAlias.
        ast::ConstantLit *out;

        ResolutionItem() = default;
        ResolutionItem(ResolutionItem &&rhs) = default;
        ResolutionItem &operator=(ResolutionItem &&rhs) = default;

        ResolutionItem(const ResolutionItem &rhs) = delete;
        const ResolutionItem &operator=(const ResolutionItem &rhs) = delete;
    };
    struct AncestorResolutionItem {
        ast::ConstantLit *ancestor;
        core::SymbolRef klass;

        bool isSuperclass; // true if superclass, false for mixin

        AncestorResolutionItem() = default;
        AncestorResolutionItem(AncestorResolutionItem &&rhs) = default;
        AncestorResolutionItem &operator=(AncestorResolutionItem &&rhs) = default;

        AncestorResolutionItem(const AncestorResolutionItem &rhs) = delete;
        const AncestorResolutionItem &operator=(const AncestorResolutionItem &rhs) = delete;
    };

    struct ClassAliasResolutionItem {
        core::SymbolRef lhs;
        ast::ConstantLit *rhs;

        ClassAliasResolutionItem() = default;
        ClassAliasResolutionItem(ClassAliasResolutionItem &&) = default;
        ClassAliasResolutionItem &operator=(ClassAliasResolutionItem &&rhs) = default;

        ClassAliasResolutionItem(const ClassAliasResolutionItem &) = delete;
        const ClassAliasResolutionItem &operator=(const ClassAliasResolutionItem &) = delete;
    };

    vector<ResolutionItem> todo_;
    vector<AncestorResolutionItem> todo_ancestors_;
    vector<ClassAliasResolutionItem> todo_aliases_;
    using TypeAliasMap = UnorderedMap<core::SymbolRef, const ast::Expression *>;
    TypeAliasMap typeAliases;

    static core::SymbolRef resolveLhs(core::MutableContext ctx, shared_ptr<Nesting> nesting, core::NameRef name) {
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

    class ResolutionChecker {
    public:
        bool seenUnresolved = false;

        unique_ptr<ast::ConstantLit> postTransformConstantLit(core::MutableContext ctx,
                                                              unique_ptr<ast::ConstantLit> original) {
            seenUnresolved = seenUnresolved || (original->typeAlias == nullptr && !original->constantSymbol().exists());
            return original;
        };
    };

    static bool isFullyResolved(core::MutableContext ctx, const ast::Expression *expression) {
        ResolutionChecker checker;
        unique_ptr<ast::Expression> dummy(const_cast<ast::Expression *>(expression));
        dummy = ast::TreeMap::apply(ctx, checker, move(dummy));
        ENFORCE(dummy.get() == expression);
        dummy.release();
        return !checker.seenUnresolved;
    }

    static core::SymbolRef resolveConstant(core::MutableContext ctx, shared_ptr<Nesting> nesting,
                                           const unique_ptr<ast::UnresolvedConstantLit> &c,
                                           const TypeAliasMap &typeAliases, bool lastRun) {
        if (ast::isa_tree<ast::EmptyTree>(c->scope.get())) {
            core::SymbolRef result = resolveLhs(ctx, nesting, c->cnst);
            return result;
        }
        ast::Expression *resolvedScope = c->scope.get();
        if (auto *id = ast::cast_tree<ast::ConstantLit>(resolvedScope)) {
            if (id->typeAlias) {
                if (auto e = ctx.state.beginError(c->loc, core::errors::Resolver::ConstantInTypeAlias)) {
                    e.setHeader("Resolving constants through type aliases is not supported");
                }
                return core::Symbols::untyped();
            }
            if (!id->constantSymbol().exists()) {
                // TODO: try to resolve if not resolved.
                return core::Symbols::noSymbol();
            }
            core::SymbolRef resolved = id->constantSymbol().data(ctx)->dealias(ctx);
            core::SymbolRef result = resolved.data(ctx)->findMember(ctx, c->cnst);
            return result;
        } else {
            if (auto e = ctx.state.beginError(c->loc, core::errors::Resolver::DynamicConstant)) {
                e.setHeader("Dynamic constant references are unsupported `{}`", c->toString(ctx));
            }
            return core::Symbols::untyped();
        }
    }

    static bool resolveJob(core::MutableContext ctx, ResolutionItem &job, const TypeAliasMap &typeAliases, bool lastRun,
                           int depth = 0) {
        if (job.out->typeAlias || job.out->constantSymbol().exists()) {
            return true;
        }
        if (depth > 256) {
            Error::raise("Too many recursive calls trying to resolve constant:\n", job.out->original->cnst.show(ctx),
                         "\n", job.out->original->loc.file().data(ctx).path(), "\n",
                         job.out->original->loc.toString(ctx));
        }
        auto resolved =
            resolveConstant(ctx.withOwner(job.scope->scope), job.scope, job.out->original, typeAliases, lastRun);
        if (!resolved.exists()) {
            if (!lastRun) {
                return false;
            }

            ast::ConstantLit *inner = job.out;
            ast::ConstantLit *missingConstant = job.out;
            ast::ConstantLit *scopeRec;
            while (inner->original &&
                   (scopeRec = ast::cast_tree<ast::ConstantLit>(inner->original->scope.get())) != nullptr) {
                if (inner->typeAlias || inner->constantSymbol().exists()) {
                    break;
                }
                missingConstant = inner;
                inner = scopeRec;
            }
            core::SymbolRef scope;
            if (inner->constantSymbol().exists()) {
                scope = inner->constantSymbol().data(ctx)->dealias(ctx);
            } else if (auto *id = ast::cast_tree<ast::ConstantLit>(inner->original->scope.get())) {
                scope = id->constantSymbol().data(ctx)->dealias(ctx);
            } else {
                scope = job.scope->scope;
            }
            auto customAutogenError = missingConstant->original->cnst == core::Symbols::Subclasses().data(ctx)->name;
            if (scope.data(ctx)->isStaticField()) {
                // most likely an unresolved alias. Well, fill it in and emit an error
                if (auto e =
                        ctx.state.beginError(missingConstant->original->loc, core::errors::Resolver::StubConstant)) {
                    e.setHeader("Unable to resolve constant `{}`", missingConstant->original->cnst.show(ctx));
                }
                // as we're going to start adding definitions into it, we need some class to piggy back on
                scope = ctx.state.enterClassSymbol(
                    inner->loc, scope.data(ctx)->owner,
                    ctx.state.enterNameConstant(ctx.state.freshNameUnique(core::UniqueNameKind::ResolverMissingClass,
                                                                          scope.data(ctx)->name, 1)));
                // as we've name-mangled the class, we have to manually create the next level of resolution
                auto createdSym = ctx.state.enterClassSymbol(inner->loc, scope, missingConstant->original->cnst);
                missingConstant->setConstantSymbol(createdSym);
            } else if (!scope.data(ctx)->derivesFrom(ctx, core::Symbols::StubClass()) || customAutogenError) {
                if (auto e =
                        ctx.state.beginError(missingConstant->original->loc, core::errors::Resolver::StubConstant)) {
                    e.setHeader("Unable to resolve constant `{}`", missingConstant->original->cnst.show(ctx));

                    if (customAutogenError) {
                        e.addErrorSection(
                            core::ErrorSection("If this constant is generated by Autogen, you "
                                               "may need to re-generate the .rbi. Try running:\n"
                                               "  scripts/bin/remote-script sorbet/shim_generation/autogen.rb"));
                    } else {
                        auto suggested = scope.data(ctx)->findMemberFuzzyMatch(ctx, missingConstant->original->cnst);
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

            core::SymbolRef stub = ctx.state.enterClassSymbol(inner->loc, scope, missingConstant->original->cnst);
            stub.data(ctx)->superClass = core::Symbols::StubClass();
            stub.data(ctx)->resultType = core::Types::untypedUntracked();
            stub.data(ctx)->setIsModule(false);
            stub.data(ctx)->singletonClass(ctx); // force singleton class into existence.
            bool resolved = resolveJob(ctx, job, typeAliases, true, depth + 1);
            return resolved;
        }
        if (resolved.data(ctx)->isStaticField() && resolved.data(ctx)->isStaticTypeAlias()) {
            auto fnd = typeAliases.find(resolved);
            if (fnd != typeAliases.end()) {
                if (isFullyResolved(ctx, fnd->second)) {
                    auto ret = fnd->second->deepCopy();
                    if (ret) {
                        job.out->typeAlias = move(ret);
                        job.out->setAliasSymbol(resolved);
                        return true;
                    }
                }
                if (lastRun) {
                    if (auto e = ctx.state.beginError(resolved.data(ctx)->loc(),
                                                      core::errors::Resolver::RecursiveTypeAlias)) {
                        e.setHeader("Type alias expands to to an infinite type");
                    }
                    job.out->typeAlias = ast::MK::Constant(job.out->loc, core::Symbols::untyped());
                    job.out->setAliasSymbol(resolved);
                    return true;
                }
            }
            return false;
        }

        job.out->setConstantSymbol(resolved);
        return true;
    }

    static bool resolveAliasJob(core::MutableContext ctx, ClassAliasResolutionItem &it) {
        if (it.rhs->constantSymbol().exists()) {
            if (it.rhs->constantSymbol().data(ctx)->dealias(ctx) != it.lhs) {
                it.lhs.data(ctx)->resultType = make_unique<core::AliasType>(it.rhs->constantSymbol());
            } else {
                if (auto e =
                        ctx.state.beginError(it.lhs.data(ctx)->loc(), core::errors::Resolver::RecursiveClassAlias)) {
                    e.setHeader("Class alias aliases to itself");
                }
                it.lhs.data(ctx)->resultType = core::Types::untypedUntracked();
            }
            return true;
        }
        return false;
    }

    static bool resolveAncestorJob(core::MutableContext ctx, AncestorResolutionItem &job,
                                   const TypeAliasMap &typeAliases, bool lastRun) {
        if (!job.ancestor->typeAliasOrConstantSymbol().exists()) {
            return false;
        }

        core::SymbolRef resolved = job.ancestor->constantSymbol().data(ctx)->dealias(ctx);

        if (!resolved.data(ctx)->isClass()) {
            if (!lastRun) {
                return false;
            }
            if (auto e = ctx.state.beginError(job.ancestor->loc, core::errors::Resolver::DynamicSuperclass)) {
                e.setHeader("Superclasses and mixins must be statically typeAlias to classes");
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
            } else if (!job.klass.data(ctx)->superClass.exists() ||
                       job.klass.data(ctx)->superClass == core::Symbols::todo() ||
                       job.klass.data(ctx)->superClass == resolved) {
                job.klass.data(ctx)->superClass = resolved;
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
            ancestor = postTransformUnresolvedConstantLit(ctx, move(inner));
            nesting_ = scopeTmp;
        }
        AncestorResolutionItem job;
        job.klass = klass;
        job.isSuperclass = isSuperclass;

        if (auto *cnst = ast::cast_tree<ast::ConstantLit>(ancestor.get())) {
            ENFORCE(cnst->constantSymbol().exists() || ast::isa_tree<ast::ConstantLit>(cnst->original->scope.get()) ||
                    ast::isa_tree<ast::EmptyTree>(cnst->original->scope.get()));
            if (isSuperclass && cnst->typeAliasOrConstantSymbol() == core::Symbols::todo()) {
                return;
            }
            job.ancestor = cnst;
        } else if (auto *self = ast::cast_tree<ast::Self>(ancestor.get())) {
            auto loc = ancestor->loc;
            auto nw = make_unique<ast::UnresolvedConstantLit>(loc, move(ancestor), ctx.contextClass().data(ctx)->name);
            auto out = make_unique<ast::ConstantLit>(loc, ctx.contextClass(), move(nw), nullptr);
            job.ancestor = out.get();
            ancestor = move(out);
        } else {
            if (auto e = ctx.state.beginError(ancestor->loc, core::errors::Resolver::DynamicSuperclass)) {
                e.setHeader("Superclasses and mixins must be statically typeAlias");
            }
            return;
        }

        if (resolveAncestorJob(ctx, job, typeAliases, false)) {
            categoryCounterInc("resolve.constants.ancestor", "firstpass");
        } else {
            todo_ancestors_.emplace_back(move(job));
        }
    }

public:
    ResolveConstantsWalk(core::MutableContext ctx) : nesting_(make_unique<Nesting>(nullptr, core::Symbols::root())) {}

    unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
        nesting_ = make_unique<Nesting>(move(nesting_), original->symbol);
        return original;
    }

    unique_ptr<ast::Expression> postTransformUnresolvedConstantLit(core::MutableContext ctx,
                                                                   unique_ptr<ast::UnresolvedConstantLit> c) {
        if (auto *constScope = ast::cast_tree<ast::UnresolvedConstantLit>(c->scope.get())) {
            unique_ptr<ast::UnresolvedConstantLit> inner(constScope);
            c->scope.release();
            c->scope = postTransformUnresolvedConstantLit(ctx, move(inner));
        }
        auto loc = c->loc;
        auto out = make_unique<ast::ConstantLit>(loc, core::Symbols::noSymbol(), move(c), nullptr);
        ResolutionItem job{nesting_, out.get()};
        if (resolveJob(ctx, job, typeAliases, false)) {
            categoryCounterInc("resolve.constants.nonancestor", "firstpass");
        } else {
            todo_.emplace_back(move(job));
        }
        return out;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
        core::SymbolRef klass = original->symbol;

        for (auto &ancst : original->ancestors) {
            bool isSuperclass = (original->kind == ast::Class && &ancst == &original->ancestors.front());
            transformAncestor(ctx, klass, ancst, isSuperclass);
        }

        auto singleton = klass.data(ctx)->singletonClass(ctx);
        for (auto &ancst : original->singleton_ancestors) {
            transformAncestor(ctx, singleton, ancst);
        }

        nesting_ = move(nesting_->parent);
        return original;
    }

    unique_ptr<ast::Expression> postTransformAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        auto *id = ast::cast_tree<ast::ConstantLit>(asgn->lhs.get());
        if (id == nullptr || !id->typeAliasOrConstantSymbol().data(ctx, true)->isStaticField()) {
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
                }
            } else {
                typeAliases[id->constantSymbol()] = send->args[0].get();
            }
            return asgn;
        }

        auto *rhs = ast::cast_tree<ast::ConstantLit>(asgn->rhs.get());
        if (rhs == nullptr) {
            return asgn;
        }

        auto item = ClassAliasResolutionItem{id->constantSymbol(), rhs};

        if (resolveAliasJob(ctx, item)) {
            categoryCounterInc("resolve.constants.aliases", "firstpass");
        } else {
            // TODO(perf) currently, by construction the last item in resolve todo list is the one this alias depends on
            // We may be able to get some perf by using this
            this->todo_aliases_.emplace_back(move(item));
        }
        return asgn;
    }

    static bool compareLocs(core::Context ctx, core::Loc lhs, core::Loc rhs) {
        core::StrictLevel left = core::StrictLevel::Strong;
        core::StrictLevel right = core::StrictLevel::Strong;

        if (lhs.file().exists()) {
            left = lhs.file().data(ctx).strict;
        }
        if (rhs.file().exists()) {
            right = rhs.file().data(ctx).strict;
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

    static vector<unique_ptr<ast::Expression>> resolveConstants(core::MutableContext ctx,
                                                                vector<unique_ptr<ast::Expression>> trees) {
        Timer timeit(ctx.state.errorQueue->logger, "resolver.resolve_constants");
        ResolveConstantsWalk constants(ctx);

        for (auto &tree : trees) {
            tree = ast::TreeMap::apply(ctx, constants, move(tree));
        }

        auto todo = move(constants.todo_);
        auto todo_ancestors = move(constants.todo_ancestors_);
        auto todo_aliases = move(constants.todo_aliases_);
        const auto typeAliases = move(constants.typeAliases);
        bool progress = true;

        while (!(todo.empty() && todo_ancestors.empty()) && progress) {
            counterInc("resolve.constants.retries");
            {
                // This is an optimization. The order should not matter semantically
                // We try to resolve most ancestors second because this makes us much more likely to resolve everything
                // else.
                int orig_size = todo_ancestors.size();
                auto it = remove_if(todo_ancestors.begin(), todo_ancestors.end(),
                                    [ctx, &typeAliases](AncestorResolutionItem &job) -> bool {
                                        return resolveAncestorJob(ctx, job, typeAliases, false);
                                    });
                todo_ancestors.erase(it, todo_ancestors.end());
                progress = (orig_size != todo_ancestors.size());
                categoryCounterAdd("resolve.constants.ancestor", "retry", orig_size - todo_ancestors.size());
            }
            {
                int orig_size = todo.size();
                auto it = remove_if(todo.begin(), todo.end(), [ctx, &typeAliases](ResolutionItem &job) -> bool {
                    return resolveJob(ctx, job, typeAliases, false);
                });
                todo.erase(it, todo.end());
                progress = progress || (orig_size != todo.size());
                categoryCounterAdd("resolve.constants.nonancestor", "retry", orig_size - todo.size());
            }
            {
                // This is an optimization. The order should not matter semantically
                // This is done as a "pre-step" because the first iteration of this effectively ran in TreeMap.
                // every item in todo_aliases implicitly depends on an item in item in todo
                // there would be no point in running the todo_aliases step before todo

                int orig_size = todo_aliases.size();
                auto it = remove_if(todo_aliases.begin(), todo_aliases.end(),
                                    [ctx](ClassAliasResolutionItem &it) -> bool { return resolveAliasJob(ctx, it); });
                todo_aliases.erase(it, todo_aliases.end());
                progress = progress || (orig_size != todo_aliases.size());
                categoryCounterAdd("resolve.constants.aliases", "retry", orig_size - todo_aliases.size());
            }
        }
        // We can no longer resolve new constants. All the code below reports errors

        categoryCounterAdd("resolve.constants.nonancestor", "failure", todo.size());
        categoryCounterAdd("resolve.constants.ancestor", "failure", todo_ancestors.size());

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
            return compareLocs(ctx, lhs.out->loc, rhs.out->loc);
        });

        fast_sort(todo_ancestors, [ctx](const auto &lhs, const auto &rhs) -> bool {
            return compareLocs(ctx, lhs.ancestor->loc, rhs.ancestor->loc);
        });

        // Note that this is missing alias stubbing, thus resolveJob needs to be able to handle missing aliases.

        for (auto &job : todo) {
            auto resolved = resolveJob(ctx, job, typeAliases, true);
            ENFORCE(resolved);
        }

        for (auto &job : todo_ancestors) {
            auto resolved = resolveAncestorJob(ctx, job, typeAliases, true);
            if (!resolved) {
                resolved = resolveAncestorJob(ctx, job, typeAliases, true);
                ENFORCE(resolved);
            }
        }

        return trees;
    }
};

class ResolveSignaturesWalk {
private:
    void fillInInfoFromSig(core::MutableContext ctx, core::SymbolRef method, ast::Send *send, bool isOverloaded) {
        auto exprLoc = send->loc;

        auto sig = TypeSyntax::parseSig(ctx, send, nullptr, true, method);

        if (!sig.seen.returns && !sig.seen.void_) {
            if (sig.seen.params ||
                !(sig.seen.abstract || sig.seen.override_ || sig.seen.implementation || sig.seen.overridable)) {
                if (auto e = ctx.state.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Malformed `{}`: No return type specified. Specify one with .returns()", "sig");
                }
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
        if (!sig.typeArgs.empty()) {
            method.data(ctx)->setGenericMethod();
            for (auto &typeSpec : sig.typeArgs) {
                if (typeSpec.type) {
                    auto name = ctx.state.freshNameUnique(core::UniqueNameKind::TypeVarName, typeSpec.name, 1);
                    auto sym = ctx.state.enterTypeArgument(typeSpec.loc, method, name, core::Variance::CoVariant);
                    typeSpec.type->sym = sym;
                    sym.data(ctx)->resultType = typeSpec.type;
                }
            }
        }
        auto methodInfo = method.data(ctx);

        methodInfo->resultType = sig.returns;

        for (auto it = methodInfo->arguments().begin(); it != methodInfo->arguments().end(); /* nothing */) {
            core::SymbolRef arg = *it;
            auto spec = find_if(sig.argTypes.begin(), sig.argTypes.end(),
                                [&](auto &spec) { return spec.name == arg.data(ctx)->name; });
            if (spec != sig.argTypes.end()) {
                ENFORCE(spec->type != nullptr);
                arg.data(ctx)->resultType = spec->type;
                arg.data(ctx)->addLoc(ctx, spec->loc);
                sig.argTypes.erase(spec);
                ++it;
            } else if (isOverloaded) {
                it = methodInfo->arguments().erase(it);
            } else if (arg.data(ctx)->resultType != nullptr) {
                ++it;
            } else {
                arg.data(ctx)->resultType = core::Types::untyped(ctx, arg);
                if (sig.seen.params || sig.seen.returns || sig.seen.void_) {
                    // Only error if we have any types
                    if (auto e = ctx.state.beginError(arg.data(ctx)->loc(),
                                                      core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`. Type not specified for argument `{}`", "sig",
                                    arg.data(ctx)->name.toString(ctx));
                    }
                }
                ++it;
            }

            if (isOverloaded && arg.data(ctx)->isKeyword()) {
                if (auto e =
                        ctx.state.beginError(arg.data(ctx)->loc(), core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Malformed `{}`. Overloaded functions cannot have keyword arguments:  `{}`", "sig",
                                arg.data(ctx)->name.toString(ctx));
                }
            }
        }

        for (auto spec : sig.argTypes) {
            if (auto e = ctx.state.beginError(spec.loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Unknown argument name `{}`", spec.name.toString(ctx));
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
                lets.emplace_back(move(let));
            }
        }

        if (lets.size() > 0) {
            auto loc = mdef->rhs->loc;
            mdef->rhs = ast::MK::InsSeq(loc, move(lets), move(mdef->rhs));
        }
    }

    void processMixesInClassMethods(core::MutableContext ctx, ast::Send *send) {
        if (!ctx.owner.data(ctx)->isClassModule()) {
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
        if (id == nullptr || !id->constantSymbol().exists() || !id->constantSymbol().data(ctx)->isClass()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Argument to `{}` must be statically resolvable to a module",
                            send->fun.data(ctx)->show(ctx));
            }
            return;
        }
        if (id->constantSymbol().data(ctx)->isClassClass()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` is a class, not a module; Only modules may be mixins",
                            id->constantSymbol().data(ctx)->show(ctx));
            }
            return;
        }
        if (id->constantSymbol() == ctx.owner) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Must not pass your self to `{}`", send->fun.data(ctx)->show(ctx));
            }
            return;
        }
        auto existing = ctx.owner.data(ctx)->findMember(ctx, core::Names::classMethods());
        if (existing.exists()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` can only be declared once per module", send->fun.data(ctx)->show(ctx));
                e.addErrorLine(ctx.owner.data(ctx)->loc(), "Previous definition in this class");
            }
            return;
        }
        ctx.owner.data(ctx)->members[core::Names::classMethods()] = id->constantSymbol();
    }

    void processClassBody(core::MutableContext ctx, unique_ptr<ast::ClassDef> &klass) {
        InlinedVector<ast::Expression *, 1> lastSig;
        for (auto &stat : klass->rhs) {
            typecase(
                stat.get(),

                [&](ast::Send *send) {
                    if (TypeSyntax::isSig(ctx, send)) {
                        if (!lastSig.empty()) {
                            if (!ctx.permitOverloadDefinitions()) {
                                if (auto e = ctx.state.beginError(lastSig[0]->loc,
                                                                  core::errors::Resolver::InvalidMethodSignature)) {
                                    e.setHeader("Unused type annotation. No method def before next annotation");
                                    e.addErrorLine(send->loc, "Type annotation that will be used instead");
                                }
                            }
                        }

                        // parsing the sig will transform the sig into a format we can use
                        TypeSyntax::parseSig(ctx, send, nullptr, true, core::Symbols::untyped());

                        lastSig.emplace_back(stat.get());
                        return;
                    }

                    if (!ast::isa_tree<ast::Self>(send->recv.get())) {
                        return;
                    }

                    switch (send->fun._id) {
                        case core::Names::mixesInClassMethods()._id: {
                            processMixesInClassMethods(ctx, send);
                        } break;
                        default:
                            return;
                    }
                    stat.reset(nullptr);
                },

                [&](ast::MethodDef *mdef) {
                    if (debug_mode) {
                        bool hasSig = !lastSig.empty();
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

                    if (!lastSig.empty()) {
                        prodCounterInc("types.sig.count");

                        bool isOverloaded = lastSig.size() > 1 && ctx.permitOverloadDefinitions();

                        if (isOverloaded) {
                            mdef->symbol.data(ctx)->setOverloaded();
                            int i = 1;

                            while (i < lastSig.size()) {
                                auto overload = ctx.state.enterNewMethodOverload(lastSig[i]->loc, mdef->symbol, i);
                                fillInInfoFromSig(ctx, overload, ast::cast_tree<ast::Send>(lastSig[i]), isOverloaded);
                                if (i + 1 < lastSig.size()) {
                                    overload.data(ctx)->setOverloaded();
                                }
                                i++;
                            }
                        }

                        fillInInfoFromSig(ctx, mdef->symbol, ast::cast_tree<ast::Send>(lastSig[0]), isOverloaded);

                        // TODO(jez) Should we handle isOverloaded?
                        if (!isOverloaded) {
                            injectOptionalArgs(ctx, mdef);
                        }

                        // OVERLOAD
                        lastSig.clear();
                    }

                    if (mdef->symbol.data(ctx)->isAbstract()) {
                        if (!ast::isa_tree<ast::EmptyTree>(mdef->rhs.get())) {
                            if (auto e = ctx.state.beginError(mdef->rhs->loc,
                                                              core::errors::Resolver::AbstractMethodWithBody)) {
                                e.setHeader("Abstract methods must not contain any code in their body");
                            }

                            mdef->rhs = ast::MK::EmptyTree(mdef->rhs->loc);
                        }
                        if (!mdef->symbol.data(ctx)->enclosingClass(ctx).data(ctx)->isClassAbstract()) {
                            if (auto e = ctx.state.beginError(mdef->loc,
                                                              core::errors::Resolver::AbstractMethodOutsideAbstract)) {
                                e.setHeader("Before declaring an abstract method, you must mark your class/module "
                                            "as abstract using `abstract!` or `interface!`");
                            }
                        }
                    } else if (mdef->symbol.data(ctx)->enclosingClass(ctx).data(ctx)->isClassInterface()) {
                        if (auto e =
                                ctx.state.beginError(mdef->loc, core::errors::Resolver::ConcreteMethodInInterface)) {
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

        if (!lastSig.empty()) {
            if (auto e = ctx.state.beginError(lastSig[0]->loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `{}`. No method def following it", "sig");
            }
        }

        auto toRemove = remove_if(klass->rhs.begin(), klass->rhs.end(),
                                  [](unique_ptr<ast::Expression> &stat) -> bool { return stat.get() == nullptr; });

        klass->rhs.erase(toRemove, klass->rhs.end());
    }

    // Resolve the type of the rhs of a constant declaration. This logic is
    // extremely simplistic; We only handle simple literals, and explicit casts.
    //
    // We don't handle array or hash literals, because intuiting the element
    // type (once we have generics) will be nontrivial.
    shared_ptr<core::Type> resolveConstantType(core::MutableContext ctx, unique_ptr<ast::Expression> &expr,
                                               core::SymbolRef ofSym) {
        shared_ptr<core::Type> result;
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

            scope = ctx.contextClass();
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
            if (auto e = ctx.state.beginError(uid->loc, core::errors::Resolver::DuplicateVariableDeclaration)) {
                e.setHeader("Illegal variable redeclaration");
                e.addErrorLine(prior.data(ctx)->loc(), "Previous declaration is here:");
            }
            return false;
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

public:
    int sendCount = 0;

    unique_ptr<ast::Assign> postTransformAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        if (handleDeclaration(ctx, asgn)) {
            return asgn;
        }

        auto *id = ast::cast_tree<ast::ConstantLit>(asgn->lhs.get());
        if (id == nullptr || !id->constantSymbol().exists()) {
            return asgn;
        }

        auto data = id->constantSymbol().data(ctx);
        if (data->isTypeMember()) {
            ENFORCE(data->isFixed());
            auto send = ast::cast_tree<ast::Send>(asgn->rhs.get());
            auto recv = ast::cast_tree<ast::Self>(send->recv.get());
            ENFORCE(recv);
            ENFORCE(send->fun == core::Names::typeMember() || send->fun == core::Names::typeTemplate());
            int arg;
            if (send->args.size() == 1) {
                arg = 0;
            } else if (send->args.size() == 2) {
                arg = 1;
            } else {
                Error::raise("Wrong arg count");
            }

            auto *hash = ast::cast_tree<ast::Hash>(send->args[arg].get());
            if (hash) {
                int i = -1;
                for (auto &keyExpr : hash->keys) {
                    i++;
                    auto lit = ast::cast_tree<ast::Literal>(keyExpr.get());
                    if (lit && lit->isSymbol(ctx) && lit->asSymbol(ctx) == core::Names::fixed()) {
                        ParsedSig emptySig;
                        data->resultType =
                            TypeSyntax::getResultType(ctx, hash->values[i], emptySig, false, id->constantSymbol());
                    }
                }
            }
        } else if (data->isStaticField()) {
            if (data->resultType != nullptr) {
                return asgn;
            }
            data->resultType = resolveConstantType(ctx, asgn->rhs, id->constantSymbol());
        }

        return asgn;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
        processClassBody(ctx.withOwner(original->symbol), original);
        return original;
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> send) {
        auto *recv = send->recv.get();
        auto *id = ast::cast_tree<ast::ConstantLit>(recv);
        if (id == nullptr) {
            sendCount++;
            return send;
        }
        if (id->typeAliasOrConstantSymbol() != core::Symbols::T()) {
            sendCount++;
            return send;
        }
        switch (send->fun._id) {
            case core::Names::let()._id:
            case core::Names::assertType()._id:
            case core::Names::cast()._id: {
                if (send->args.size() < 2) {
                    return send;
                }

                auto expr = move(send->args[0]);
                ParsedSig emptySig;
                auto type = TypeSyntax::getResultType(ctx, send->args[1], emptySig, false, core::Symbols::noSymbol());
                return ast::MK::InsSeq1(send->loc, ast::MK::KeepForTypechecking(move(send->args[1])),
                                        make_unique<ast::Cast>(send->loc, type, move(expr), send->fun));
            }
            default:
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
            inits.emplace_back(move(*it));
            it = klass->rhs.erase(it);
        }

        if (inits.empty()) {
            return nullptr;
        }
        if (inits.size() == 1) {
            return move(inits.front());
        }
        return make_unique<ast::InsSeq>(klass->declLoc, move(inits), make_unique<ast::EmptyTree>(core::Loc::none()));
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
            sym = ctx.state.staticInitForFile(classDef->loc.file());
        } else {
            sym = ctx.state.enterMethodSymbol(inits->loc, classDef->symbol, core::Names::staticInit());
        }
        auto init = make_unique<ast::MethodDef>(inits->loc, inits->loc, sym, core::Names::staticInit(),
                                                ast::MethodDef::ARGS_store(), move(inits), true);
        classDef->rhs.emplace_back(move(init));

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

        auto loc = classDef->loc;
        classDef->rhs = addMethods(ctx, move(classDef->rhs));
        classes[classStack.back()] = move(classDef);
        classStack.pop_back();
        return make_unique<ast::EmptyTree>(loc);
    };

    unique_ptr<ast::Expression> postTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> methodDef) {
        auto &methods = curMethodSet();
        ENFORCE(!methods.stack.empty());
        ENFORCE(methods.methods.size() > methods.stack.back());
        ENFORCE(methods.methods[methods.stack.back()] == nullptr);

        auto loc = methodDef->loc;
        methods.methods[methods.stack.back()] = move(methodDef);
        methods.stack.pop_back();
        return make_unique<ast::EmptyTree>(loc);
    };

    unique_ptr<ast::Expression> addClasses(core::MutableContext ctx, unique_ptr<ast::Expression> tree) {
        if (classes.empty()) {
            ENFORCE(sortedClasses().empty());
            return tree;
        }
        if (classes.size() == 1 && (ast::cast_tree<ast::EmptyTree>(tree.get()) != nullptr)) {
            // It was only 1 class to begin with, put it back
            return move(sortedClasses()[0]);
        }

        auto insSeq = ast::cast_tree<ast::InsSeq>(tree.get());
        if (insSeq == nullptr) {
            ast::InsSeq::STATS_store stats;
            auto sorted = sortedClasses();
            stats.insert(stats.begin(), make_move_iterator(sorted.begin()), make_move_iterator(sorted.end()));
            return ast::MK::InsSeq(tree->loc, move(stats), move(tree));
        }

        for (auto &clas : sortedClasses()) {
            ENFORCE(!!clas);
            insSeq->stats.emplace_back(move(clas));
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
            unique_ptr<ast::Expression> methodDef = move(popCurMethodDefs()[0]);
            return methodDef;
        }

        auto insSeq = ast::cast_tree<ast::InsSeq>(tree.get());
        if (insSeq == nullptr) {
            ast::InsSeq::STATS_store stats;
            tree = make_unique<ast::InsSeq>(tree->loc, move(stats), move(tree));
            return addMethods(ctx, move(tree));
        }

        for (auto &method : popCurMethodDefs()) {
            ENFORCE(!!method);
            insSeq->stats.emplace_back(move(method));
        }
        return tree;
    }

private:
    vector<unique_ptr<ast::ClassDef>> sortedClasses() {
        ENFORCE(classStack.empty());
        auto ret = move(classes);
        classes.clear();
        return ret;
    }

    ast::ClassDef::RHS_store addMethods(core::MutableContext ctx, ast::ClassDef::RHS_store rhs) {
        if (curMethodSet().methods.size() == 1 && rhs.size() == 1 &&
            (ast::cast_tree<ast::EmptyTree>(rhs[0].get()) != nullptr)) {
            // It was only 1 method to begin with, put it back
            rhs.pop_back();
            rhs.emplace_back(move(popCurMethodDefs()[0]));
            return rhs;
        }
        for (auto &method : popCurMethodDefs()) {
            ENFORCE(method.get() != nullptr);
            rhs.emplace_back(move(method));
        }
        return rhs;
    }

    vector<unique_ptr<ast::MethodDef>> popCurMethodDefs() {
        auto ret = move(curMethodSet().methods);
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

class ResolveVariablesWalk {
public:
    unique_ptr<ast::Expression> postTransformUnresolvedIdent(core::MutableContext ctx,
                                                             unique_ptr<ast::UnresolvedIdent> id) {
        core::SymbolRef klass;

        switch (id->kind) {
            case ast::UnresolvedIdent::Class:
                klass = ctx.contextClass();
                break;
            case ast::UnresolvedIdent::Instance:
                klass = ctx.selfClass();
                break;
            default:
                // These should have been removed in the namer
                Error::notImplemented();
        }

        core::SymbolRef sym = klass.data(ctx)->findMemberTransitive(ctx, id->name);
        if (!sym.exists()) {
            if (auto e = ctx.state.beginError(id->loc, core::errors::Resolver::UndeclaredVariable)) {
                e.setHeader("Use of undeclared variable `{}`", id->name.toString(ctx));
            }
            if (id->kind == ast::UnresolvedIdent::Class) {
                sym = ctx.state.enterStaticFieldSymbol(id->loc, klass, id->name);
            } else {
                sym = ctx.state.enterFieldSymbol(id->loc, klass, id->name);
            }
            sym.data(ctx)->resultType = core::Types::untyped(ctx, sym);
        }

        return make_unique<ast::Field>(id->loc, sym);
    };
};

class ResolveSanityCheckWalk {
public:
    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
        ENFORCE(original->symbol != core::Symbols::todo());
        return original;
    }
    unique_ptr<ast::Expression> postTransformMethodDef(core::MutableContext ctx, unique_ptr<ast::MethodDef> original) {
        ENFORCE(original->symbol != core::Symbols::todo());
        return original;
    }
    unique_ptr<ast::Expression> postTransformUnresolvedConstantLit(core::MutableContext ctx,
                                                                   unique_ptr<ast::UnresolvedConstantLit> original) {
        ENFORCE(false, "should have been removed");
        return original;
    }
    unique_ptr<ast::Expression> postTransformUnresolvedIdent(core::MutableContext ctx,
                                                             unique_ptr<ast::UnresolvedIdent> original) {
        Error::raise("These should have all been removed");
    }
    unique_ptr<ast::Expression> postTransformSelf(core::MutableContext ctx, unique_ptr<ast::Self> original) {
        ENFORCE(original->claz != core::Symbols::todo());
        return original;
    }
    unique_ptr<ast::Expression> postTransformBlock(core::MutableContext ctx, unique_ptr<ast::Block> original) {
        ENFORCE(original->symbol != core::Symbols::todo());
        return original;
    }
    unique_ptr<ast::ConstantLit> postTransformConstantLit(core::MutableContext ctx,
                                                          unique_ptr<ast::ConstantLit> original) {
        ENFORCE(original->typeAlias.get() != nullptr || original->constantSymbol().exists());
        return original;
    }
};
}; // namespace

vector<unique_ptr<ast::Expression>> Resolver::run(core::MutableContext ctx, vector<unique_ptr<ast::Expression>> trees) {
    trees = ResolveConstantsWalk::resolveConstants(ctx, move(trees));
    finalizeAncestors(ctx.state);
    trees = resolveSigs(ctx, move(trees));
    finalizeResolution(ctx.state);
    sanityCheck(ctx, trees);

    return trees;
}

vector<unique_ptr<ast::Expression>> Resolver::resolveSigs(core::MutableContext ctx,
                                                          vector<unique_ptr<ast::Expression>> trees) {
    ResolveSignaturesWalk sigs;
    ResolveVariablesWalk vars;
    Timer timeit(ctx.state.errorQueue->logger, "resolver.sigs_vars_and_flatten");
    for (auto &tree : trees) {
        tree = ast::TreeMap::apply(ctx, sigs, move(tree));
        tree = ast::TreeMap::apply(ctx, vars, move(tree));

        // declared in here since it holds onto state
        FlattenWalk flatten;
        tree = ast::TreeMap::apply(ctx, flatten, move(tree));
        tree = flatten.addClasses(ctx, move(tree));
        tree = flatten.addMethods(ctx, move(tree));
    }

    return trees;
}

void Resolver::sanityCheck(core::MutableContext ctx, vector<unique_ptr<ast::Expression>> &trees) {
    if (debug_mode) {
        Timer timeit(ctx.state.errorQueue->logger, "resolver.sanity_check");
        ResolveSanityCheckWalk sanity;
        for (auto &tree : trees) {
            tree = ast::TreeMap::apply(ctx, sanity, move(tree));
        }
    }
}

vector<unique_ptr<ast::Expression>> Resolver::runTreePasses(core::MutableContext ctx,
                                                            vector<unique_ptr<ast::Expression>> trees) {
    trees = ResolveConstantsWalk::resolveConstants(ctx, move(trees));
    trees = resolveSigs(ctx, move(trees));
    sanityCheck(ctx, trees);

    return trees;
}

std::vector<std::unique_ptr<ast::Expression>>
Resolver::runConstantResolution(core::MutableContext ctx, std::vector<std::unique_ptr<ast::Expression>> trees) {
    trees = ResolveConstantsWalk::resolveConstants(ctx, move(trees));
    sanityCheck(ctx, trees);

    return trees;
}

} // namespace sorbet::resolver
