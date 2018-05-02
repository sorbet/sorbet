#include "core/errors/resolver.h"
#include "ast/Helpers.h"
#include "ast/Trees.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/ErrorQueue.h"
#include "core/Names/resolver.h"
#include "core/StrictLevel.h"
#include "core/core.h"
#include "resolver/resolver.h"
#include "resolver/type_syntax.h"

#include "absl/strings/str_cat.h"

#include <algorithm> // find_if
#include <list>
#include <vector>

using namespace std;

namespace ruby_typer {
namespace resolver {
namespace {

/*
 * Ruby supports resolving constants via your ancestors -- superclasses and
 * mixins. Since superclass and mixins are themselves constant references, we
 * thus may not be able to resolve certain constants until after we've resolved
 * others.
 *
 * To solve this, we collect any failed resolutions into a pair of TODO lists,
 * and repeatedly walk them until we succeed or stop making progress. In
 * practice this loop terminates after 3 or fewer passes on most real codebases.
 *
 * This walk replaces ast::ConstantLit nodes with ast::Ident nodes, initially
 * filled in with core::Symbol::todo(), and then attempts to resolve them. Any
 * resolutions that fail are collected into the `todo_` list, or the separate
 * `todo_ancestors_` list if the constant is being resolved as part of an
 * ancestor (superclass or mixin).
 *
 * We track the latter (ancestors) separately for the dual reasons that (1) Upon
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

        Nesting(shared_ptr<Nesting> parent, core::SymbolRef scope) : parent(parent), scope(scope) {}
    };
    shared_ptr<Nesting> nesting_;

    struct ResolutionItem {
        shared_ptr<Nesting> scope;
        unique_ptr<ast::ConstantLit> cnst;
        ast::Ident *out;

        ResolutionItem() = default;
        ResolutionItem(ResolutionItem &&rhs) = default;
        ResolutionItem &operator=(ResolutionItem &&rhs) = default;

        ResolutionItem(const ResolutionItem &rhs) = delete;
        const ResolutionItem &operator=(const ResolutionItem &rhs) = delete;
    };
    struct AncestorResolutionItem {
        ResolutionItem resolv;
        core::SymbolRef klass;

        bool isSuperclass; // true if superclass, false for mixin

        AncestorResolutionItem() = default;
        AncestorResolutionItem(AncestorResolutionItem &&rhs) = default;
        AncestorResolutionItem &operator=(AncestorResolutionItem &&rhs) = default;

        AncestorResolutionItem(const AncestorResolutionItem &rhs) = delete;
        const AncestorResolutionItem &operator=(const AncestorResolutionItem &rhs) = delete;
    };

    struct AliasResolutionItem {
        core::SymbolRef lhs;
        unique_ptr<ast::Ident> rhs;

        AliasResolutionItem() = default;
        AliasResolutionItem(AliasResolutionItem &&) = default;
        AliasResolutionItem &operator=(AliasResolutionItem &&rhs) = default;

        AliasResolutionItem(const AliasResolutionItem &) = delete;
        const AliasResolutionItem &operator=(const AliasResolutionItem &) = delete;
    };

    vector<ResolutionItem> todo_;
    vector<AncestorResolutionItem> todo_ancestors_;
    vector<AliasResolutionItem> todo_aliases_;

    static core::SymbolRef resolveLhs(core::MutableContext ctx, shared_ptr<Nesting> nesting, core::NameRef name) {
        Nesting *scope = nesting.get();
        while (scope != nullptr) {
            auto lookup = scope->scope.data(ctx).findMember(ctx, name);
            if (lookup.exists()) {
                return lookup;
            }
            scope = scope->parent.get();
        }
        return nesting->scope.data(ctx).findMemberTransitive(ctx, name);
    }

    static core::SymbolRef resolveConstant(core::MutableContext ctx, shared_ptr<Nesting> nesting, ast::ConstantLit *c) {
        if (ast::isa_tree<ast::EmptyTree>(c->scope.get())) {
            core::SymbolRef result = resolveLhs(ctx, nesting, c->cnst);
            return result;
        }
        if (ast::ConstantLit *scope = ast::cast_tree<ast::ConstantLit>(c->scope.get())) {
            auto resolved = resolveConstant(ctx, nesting, scope);
            if (!resolved.exists()) {
                return resolved;
            }
            c->scope = make_unique<ast::Ident>(scope->loc, resolved);
        }

        if (ast::Ident *id = ast::cast_tree<ast::Ident>(c->scope.get())) {
            core::SymbolRef resolved = id->symbol.data(ctx).dealias(ctx);
            if (resolved.data(ctx).resultType != nullptr && resolved.data(ctx).resultType->isDynamic()) {
                return core::Symbols::untyped();
            }

            core::SymbolRef result = resolved.data(ctx).findMember(ctx, c->cnst);
            return result;
        } else {
            if (auto e = ctx.state.beginError(c->loc, core::errors::Resolver::DynamicConstant)) {
                e.setHeader("Dynamic constant references are unsupported `{}`", c->toString(ctx));
            }
            c->scope = make_unique<ast::Ident>(c->loc, core::Symbols::untyped());
            return core::Symbols::untyped();
        }
    }

    static bool resolveJob(core::MutableContext ctx, ResolutionItem &job) {
        ENFORCE(job.out->symbol == core::Symbols::todo());

        core::SymbolRef resolved = resolveConstant(ctx.withOwner(job.scope->scope), job.scope, job.cnst.get());
        if (!resolved.exists()) {
            return false;
        }
        job.out->symbol = resolved;
        return true;
    }

    static bool resolveAliasJob(core::MutableContext ctx, AliasResolutionItem &it) {
        if (it.rhs->symbol != core::Symbols::todo()) {
            it.lhs.data(ctx).resultType = make_unique<core::AliasType>(it.rhs->symbol);
            return true;
        }
        return false;
    }

    static bool resolveJob(core::MutableContext ctx, AncestorResolutionItem &job) {
        // It's possible for ancestors to enter the resolver as Idents. We will
        // see those with resolv.out->symbol already populated, and resolv.cnst
        // == nullptr. We skip resolving the constant in that case, but still
        // handle the rest of the ancestor-population mechanism here.
        if (job.resolv.cnst != nullptr) {
            if (!resolveJob(ctx, job.resolv)) {
                return false;
            }
        }
        auto resolved = job.resolv.out->symbol;

        if (!resolved.data(ctx).isClass()) {
            if (auto e = ctx.state.beginError(job.resolv.cnst->loc, core::errors::Resolver::DynamicSuperclass)) {
                e.setHeader("Superclasses and mixins must be statically resolved to classes");
            }
            resolved = core::Symbols::StubClass();
        }

        if (resolved == job.klass || resolved.data(ctx).derivesFrom(ctx, job.klass)) {
            if (auto e = ctx.state.beginError(job.resolv.cnst->loc, core::errors::Resolver::CircularDependency)) {
                e.setHeader("Circular dependency: `{}` and `{}` are declared as parents of each other",
                            job.klass.data(ctx).show(ctx), resolved.data(ctx).show(ctx));
            }
            resolved = core::Symbols::StubClass();
        }

        if (job.isSuperclass) {
            if (resolved == core::Symbols::todo()) {
                // No superclass specified
            } else if (!job.klass.data(ctx).superClass.exists() ||
                       job.klass.data(ctx).superClass == core::Symbols::todo() ||
                       job.klass.data(ctx).superClass == resolved) {
                job.klass.data(ctx).superClass = resolved;
            } else {
                if (auto e =
                        ctx.state.beginError(job.resolv.cnst->loc, core::errors::Resolver::RedefinitionOfParents)) {
                    e.setHeader("Class parents redefined for class `{}`", job.klass.data(ctx).show(ctx));
                }
            }
        } else {
            job.klass.data(ctx).mixins().emplace_back(resolved);
        }

        return true;
    }

    void transformAncestor(core::MutableContext ctx, core::SymbolRef klass, unique_ptr<ast::Expression> &ancestor,
                           bool isSuperclass = false) {
        AncestorResolutionItem job;
        job.resolv.scope = isSuperclass ? nesting_->parent : nesting_;
        job.klass = klass;
        job.isSuperclass = isSuperclass;

        if (ast::ConstantLit *cnst = ast::cast_tree<ast::ConstantLit>(ancestor.get())) {
            job.resolv.cnst.reset(cnst);
            ancestor.release();

            auto id = make_unique<ast::Ident>(cnst->loc, core::Symbols::todo());
            job.resolv.out = id.get();
            ancestor = move(id);
        } else if (ast::Self *self = ast::cast_tree<ast::Self>(ancestor.get())) {
            auto id = make_unique<ast::Ident>(self->loc, ctx.contextClass());
            job.resolv.out = id.get();
            ancestor = move(id);
        } else if (ast::Ident *id = ast::cast_tree<ast::Ident>(ancestor.get())) {
            job.resolv.out = id;
        } else {
            if (auto e = ctx.state.beginError(ancestor->loc, core::errors::Resolver::DynamicSuperclass)) {
                e.setHeader("Superclasses and mixins must be statically resolved");
            }
            return;
        }

        if (resolveJob(ctx, job)) {
            core::categoryCounterInc("resolve.constants.ancestor", "firstpass");
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
    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
        core::SymbolRef klass = original->symbol;
        if (original->kind == ast::Module && klass.data(ctx).mixins().empty()) {
            klass.data(ctx).mixins().emplace_back(core::Symbols::BasicObject());
        }

        for (auto &ancst : original->ancestors) {
            bool isSuperclass = (original->kind == ast::Class && &ancst == &original->ancestors.front());

            transformAncestor(ctx, klass, ancst, isSuperclass);
        }

        auto singleton = klass.data(ctx).singletonClass(ctx);
        for (auto &ancst : original->singleton_ancestors) {
            transformAncestor(ctx, singleton, ancst);
        }

        nesting_ = move(nesting_->parent);
        return original;
    }

    unique_ptr<ast::Expression> postTransformConstantLit(core::MutableContext ctx, unique_ptr<ast::ConstantLit> c) {
        auto id = make_unique<ast::Ident>(c->loc, core::Symbols::todo());
        ResolutionItem job{nesting_, move(c), id.get()};
        if (resolveJob(ctx, job)) {
            core::categoryCounterInc("resolve.constants.nonancestor", "firstpass");
        } else {
            todo_.emplace_back(move(job));
        }
        return id;
    }

    unique_ptr<ast::Expression> postTransformAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        auto *id = ast::cast_tree<ast::Ident>(asgn->lhs.get());
        if (id == nullptr || !id->symbol.data(ctx).isStaticField()) {
            return asgn;
        }

        auto *rhs = ast::cast_tree<ast::Ident>(asgn->rhs.get());
        if (rhs == nullptr || !rhs->symbol.data(ctx).isClass()) {
            return asgn;
        }

        auto item = AliasResolutionItem{id->symbol, unique_ptr<ast::Ident>(rhs)};
        asgn->rhs.release();

        if (resolveAliasJob(ctx, item)) {
            core::categoryCounterInc("resolve.constants.aliases", "firstpass");
        } else {
            // TODO(perf) currently, by construction the last item in resolve todo list is the one this alias depends on
            // We may be able to get some perf by using this
            this->todo_aliases_.emplace_back(move(item));
        }
        return make_unique<ast::EmptyTree>(asgn->loc);
    }

    static vector<unique_ptr<ast::Expression>> resolveConstants(core::MutableContext ctx,
                                                                vector<unique_ptr<ast::Expression>> trees) {
        ctx.trace("Resolving constants");
        ResolveConstantsWalk constants(ctx);

        for (auto &tree : trees) {
            tree = ast::TreeMap::apply(ctx, constants, move(tree));
        }

        auto todo = move(constants.todo_);
        auto todo_ancestors = move(constants.todo_ancestors_);
        auto todo_aliases = move(constants.todo_aliases_);
        bool progress = true;

        while (!(todo.empty() && todo_ancestors.empty()) && progress) {
            core::counterInc("resolve.constants.retries");
            {
                // This is an optimization. The order should not matter semantically
                // We try to resolve most ancestors second because this makes us much more likely to resolve everything
                // else.
                int orig_size = todo_ancestors.size();
                auto it = remove_if(todo_ancestors.begin(), todo_ancestors.end(),
                                    [ctx](AncestorResolutionItem &job) -> bool { return resolveJob(ctx, job); });
                todo_ancestors.erase(it, todo_ancestors.end());
                progress = (orig_size != todo_ancestors.size());
                core::categoryCounterAdd("resolve.constants.ancestor", "retry", orig_size - todo_ancestors.size());
            }
            {
                int orig_size = todo.size();
                auto it = remove_if(todo.begin(), todo.end(),
                                    [ctx](ResolutionItem &job) -> bool { return resolveJob(ctx, job); });
                todo.erase(it, todo.end());
                core::categoryCounterAdd("resolve.constants.nonancestor", "retry", orig_size - todo.size());
            }
            {
                // This is an optimization. The order should not matter semantically
                // This is done as a "pre-step" because the first iteration of this effectively ran in TreeMap.
                // every item in todo_aliases implicitly depends on an item in item in todo
                // there would be no point in running the todo_aliases step before todo

                int orig_size = todo_aliases.size();
                auto it = remove_if(todo_aliases.begin(), todo_aliases.end(),
                                    [ctx](AliasResolutionItem &it) -> bool { return resolveAliasJob(ctx, it); });
                todo_aliases.erase(it, todo_aliases.end());
                progress = progress || (orig_size != todo_aliases.size());
                core::categoryCounterAdd("resolve.constants.aliases", "retry", orig_size - todo_aliases.size());
            }
        }

        // fill in unresolved aliases with untyped. Errors for their rhs will be reported.

        for (auto &e : todo_aliases) {
            e.lhs.data(ctx).resultType = make_shared<core::AliasType>(core::Symbols::untyped());
        }

        // We can no longer resolve new constants. All the code below reports errors

        core::categoryCounterAdd("resolve.constants.nonancestor", "failure", todo.size());
        core::categoryCounterAdd("resolve.constants.ancestor", "failure", todo_ancestors.size());

        for (auto &it : todo_ancestors) {
            todo.emplace_back(move(it.resolv));
        }
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
        std::sort(todo.begin(), todo.end(), [ctx](auto &lhs, auto &rhs) -> bool {
            core::StrictLevel left = core::StrictLevel::Strong;
            core::StrictLevel right = core::StrictLevel::Strong;
            if (lhs.cnst->loc.file.exists()) {
                left = lhs.cnst->loc.file.data(ctx).strict;
            }
            if (rhs.cnst->loc.file.exists()) {
                right = rhs.cnst->loc.file.data(ctx).strict;
            }

            if (left != right) {
                return right < left;
            }
            if (lhs.cnst->loc.file != rhs.cnst->loc.file) {
                return lhs.cnst->loc.file < rhs.cnst->loc.file;
            }
            if (lhs.cnst->loc.begin_pos != rhs.cnst->loc.begin_pos) {
                return lhs.cnst->loc.begin_pos < rhs.cnst->loc.begin_pos;
            }
            return lhs.cnst->loc.end_pos < rhs.cnst->loc.end_pos;
        });

        for (auto &job : todo) {
            if (!resolveJob(ctx, job)) {
                ast::ConstantLit *inner = job.cnst.get();
                while (auto *scope = ast::cast_tree<ast::ConstantLit>(inner->scope.get())) {
                    inner = scope;
                }
                if (auto e = ctx.state.beginError(job.cnst->loc, core::errors::Resolver::StubConstant)) {
                    e.setHeader("Unable to resolve constant `{}`", inner->cnst.show(ctx));
                }

                core::SymbolRef scope;
                if (auto *id = ast::cast_tree<ast::Ident>(inner->scope.get())) {
                    scope = id->symbol;
                } else {
                    scope = job.scope->scope;
                }
                core::SymbolRef stub = ctx.state.enterClassSymbol(inner->loc, scope, inner->cnst);
                stub.data(ctx).superClass = core::Symbols::StubClass();
                stub.data(ctx).resultType = core::Types::dynamic();
                stub.data(ctx).setIsModule(false);
                bool resolved = resolveJob(ctx, job);
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

        auto sig = TypeSyntax::parseSig(ctx, send, nullptr);

        if (!sig.seen.returns && !sig.seen.void_) {
            if (sig.seen.args || !(sig.seen.abstract || sig.seen.override_ || sig.seen.implementation ||
                                   sig.seen.overridable || sig.seen.abstract)) {
                if (auto e = ctx.state.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Malformed `sig`: No return type specified. Specify one with .returns()");
                }
            }
        }
        if (sig.seen.returns && sig.seen.void_) {
            if (auto e = ctx.state.beginError(exprLoc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed `sig`: Don't use both .returns() and .void");
            }
        }

        if (sig.seen.abstract) {
            method.data(ctx).setAbstract();
        }
        if (!sig.typeArgs.empty()) {
            method.data(ctx).setGenericMethod();
            for (auto &typeSpec : sig.typeArgs) {
                if (typeSpec.type) {
                    auto name = ctx.state.freshNameUnique(core::UniqueNameKind::TypeVarName, typeSpec.name, 1);
                    auto sym = ctx.state.enterTypeArgument(typeSpec.loc, method, name, core::Variance::CoVariant);
                    typeSpec.type->sym = sym;
                    sym.data(ctx).resultType = typeSpec.type;
                }
            }
        }
        auto &methodInfo = method.data(ctx);

        methodInfo.resultType = sig.returns;

        for (auto it = methodInfo.arguments().begin(); it != methodInfo.arguments().end(); /* nothing */) {
            core::SymbolRef arg = *it;
            auto spec = find_if(sig.argTypes.begin(), sig.argTypes.end(),
                                [&](auto &spec) { return spec.name == arg.data(ctx).name; });
            if (spec != sig.argTypes.end()) {
                ENFORCE(spec->type != nullptr);
                arg.data(ctx).resultType = spec->type;
                arg.data(ctx).definitionLoc = spec->loc;
                sig.argTypes.erase(spec);
                ++it;
            } else if (isOverloaded) {
                it = methodInfo.arguments().erase(it);
            } else if (arg.data(ctx).resultType != nullptr) {
                ++it;
            } else {
                arg.data(ctx).resultType = core::Types::dynamic();
                if (sig.seen.args || sig.seen.returns || sig.seen.void_) {
                    // Only error if we have any types
                    if (auto e = ctx.state.beginError(arg.data(ctx).definitionLoc,
                                                      core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed sig. Type not specified for argument `{}`",
                                    arg.data(ctx).name.toString(ctx));
                    }
                }
                ++it;
            }

            if (isOverloaded && arg.data(ctx).isKeyword()) {
                if (auto e = ctx.state.beginError(arg.data(ctx).definitionLoc,
                                                  core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Malformed sig. Overloaded functions cannot have keyword arguments:  `{}`",
                                arg.data(ctx).name.toString(ctx));
                }
            }
        }

        for (auto spec : sig.argTypes) {
            if (auto e = ctx.state.beginError(spec.loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Unknown argument name `{}`", spec.name.toString(ctx));
            }
        }
    }

    void processMixesInClassMethods(core::MutableContext ctx, ast::Send *send) {
        if (!ctx.owner.data(ctx).isClassModule()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` can only be declared inside a module, not a class", send->fun.data(ctx).show(ctx));
            }
            // Keep processing it anyways
        }

        if (send->args.size() != 1) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Wrong number of arguments to `{}`: Expected: `{}`, got: `{}`",
                            send->fun.data(ctx).show(ctx), 1, send->args.size());
            }
            return;
        }
        auto *id = ast::cast_tree<ast::Ident>(send->args.front().get());
        if (id == nullptr || !id->symbol.data(ctx).isClass()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Argument to `{}` must be statically resolvable to a module",
                            send->fun.data(ctx).show(ctx));
            }
            return;
        }
        if (id->symbol.data(ctx).isClassClass()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` is a class, not a module; Only modules may be mixins",
                            id->symbol.data(ctx).show(ctx));
            }
            return;
        }
        if (id->symbol == ctx.owner) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("Must not pass your self to `{}`", send->fun.data(ctx).show(ctx));
            }
            return;
        }
        auto existing = ctx.owner.data(ctx).findMember(ctx, core::Names::classMethods());
        if (existing.exists()) {
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMixinDeclaration)) {
                e.setHeader("`{}` can only be declared once per module", send->fun.data(ctx).show(ctx));
            }
            return;
        }
        ctx.owner.data(ctx).members.emplace_back(core::Names::classMethods(), id->symbol);
    }

    void processClassBody(core::MutableContext ctx, unique_ptr<ast::ClassDef> &klass) {
        InlinedVector<unique_ptr<ast::Expression>, 1> lastSig;
        for (auto &stat : klass->rhs) {
            typecase(
                stat.get(),

                [&](ast::Send *send) {
                    if (TypeSyntax::isSig(ctx, send)) {
                        if (!lastSig.empty()) {
                            if (!ctx.withOwner(klass->symbol).permitOverloadDefinitions()) {
                                if (auto e = ctx.state.beginError(lastSig[0]->loc,
                                                                  core::errors::Resolver::InvalidMethodSignature)) {
                                    e.setHeader("Unused type annotation. No method def before next annotation");
                                    e.addErrorLine(send->loc, "Type annotation that will be used instead");
                                }
                            }
                        }
                        lastSig.emplace_back(move(stat));
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
                    if (!lastSig.empty()) {
                        core::prodCounterInc("types.sig.count");

                        bool isOverloaded =
                            lastSig.size() > 1 && ctx.withOwner(klass->symbol).permitOverloadDefinitions();

                        if (isOverloaded) {
                            mdef->symbol.data(ctx).setOverloaded();
                            int i = 1;

                            while (i < lastSig.size()) {
                                auto overload = ctx.state.enterNewMethodOverload(lastSig[i]->loc, mdef->symbol, i);
                                fillInInfoFromSig(ctx, overload, ast::cast_tree<ast::Send>(lastSig[i].get()),
                                                  isOverloaded);
                                if (i + 1 < lastSig.size()) {
                                    overload.data(ctx).setOverloaded();
                                }
                                i++;
                            }
                        }

                        fillInInfoFromSig(ctx, mdef->symbol, ast::cast_tree<ast::Send>(lastSig[0].get()), isOverloaded);

                        // OVERLOAD
                        lastSig.clear();
                    }

                    if (mdef->symbol.data(ctx).isAbstract()) {
                        if (!ast::isa_tree<ast::EmptyTree>(mdef->rhs.get())) {
                            if (auto e = ctx.state.beginError(mdef->rhs->loc,
                                                              core::errors::Resolver::AbstractMethodWithBody)) {
                                e.setHeader("Abstract methods must not contain any code in their body");
                            }

                            mdef->rhs = ast::MK::EmptyTree(mdef->rhs->loc);
                        }
                        if (!mdef->symbol.data(ctx).enclosingClass(ctx).data(ctx).isClassAbstract()) {
                            if (auto e = ctx.state.beginError(mdef->loc,
                                                              core::errors::Resolver::AbstractMethodOutsideAbstract)) {
                                e.setHeader("Before declaring an abstract method, you must mark your class/module "
                                            "as abstract using `abstract!` or `interface!`");
                            }
                        }
                    } else if (mdef->symbol.data(ctx).enclosingClass(ctx).data(ctx).isClassInterface()) {
                        if (auto e =
                                ctx.state.beginError(mdef->loc, core::errors::Resolver::ConcreteMethodInInterface)) {
                            e.setHeader("All methods in an interface must be declared abstract");
                        }
                    }
                },
                [&](ast::ClassDef *cdef) {
                    // Leave in place
                },

                [&](ast::Assign *assgn) {
                    if (ast::Ident *id = ast::cast_tree<ast::Ident>(assgn->lhs.get())) {
                        if (id->symbol.data(ctx).name.data(ctx).kind == core::CONSTANT) {
                            stat.reset(nullptr);
                        }
                    }
                },

                [&](ast::EmptyTree *e) { stat.reset(nullptr); },

                [&](ast::Expression *e) {});
        }

        if (!lastSig.empty()) {
            if (auto e = ctx.state.beginError(lastSig[0]->loc, core::errors::Resolver::InvalidMethodSignature)) {
                e.setHeader("Malformed sig. No method def following it");
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
    shared_ptr<core::Type> resolveConstantType(core::MutableContext ctx, unique_ptr<ast::Expression> &expr) {
        shared_ptr<core::Type> result;
        typecase(expr.get(), [&](ast::Literal *a) { result = a->value; },
                 [&](ast::Cast *cast) {
                     if (cast->cast != core::Names::let()) {
                         if (auto e = ctx.state.beginError(cast->loc, core::errors::Resolver::ConstantAssertType)) {
                             e.setHeader("Use T.let() to specify the type of constants");
                         }
                     }
                     result = cast->type;
                 },
                 [&](ast::Expression *) { result = core::Types::dynamic(); });
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

        auto *cast = ast::cast_tree<ast::Cast>(asgn->rhs.get());
        if (cast == nullptr) {
            return false;
        } else if (cast->cast != core::Names::let()) {
            if (auto e = ctx.state.beginError(cast->loc, core::errors::Resolver::ConstantAssertType)) {
                e.setHeader("Use T.let() to specify the type of constants");
            }
        }

        core::SymbolRef scope;
        if (uid->kind == ast::UnresolvedIdent::Class) {
            if (!ctx.owner.data(ctx).isClass()) {
                if (auto e = ctx.state.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                    e.setHeader("Class variables must be declared at class scope");
                }
            }

            scope = ctx.contextClass();
        } else {
            if (ctx.owner.data(ctx).isClass()) {
                // Declaring a class instance variable
            } else {
                // Inside a method; declaring a normal instance variable
                if (ctx.owner.data(ctx).name != core::Names::initialize()) {
                    if (auto e = ctx.state.beginError(uid->loc, core::errors::Resolver::InvalidDeclareVariables)) {
                        e.setHeader("Instance variables must be declared inside `initialize`");
                    }
                }
            }
            scope = ctx.selfClass();
        }

        auto prior = scope.data(ctx).findMember(ctx, uid->name);
        if (prior.exists()) {
            if (auto e = ctx.state.beginError(uid->loc, core::errors::Resolver::DuplicateVariableDeclaration)) {
                e.setHeader("Illegal variable redeclaration");
                e.addErrorLine(prior.data(ctx).definitionLoc, "Previous declaration is here:");
            }
            return false;
        }
        core::SymbolRef var;

        if (uid->kind == ast::UnresolvedIdent::Class) {
            var = ctx.state.enterStaticFieldSymbol(uid->loc, scope, uid->name);
        } else {
            var = ctx.state.enterFieldSymbol(uid->loc, scope, uid->name);
        }

        var.data(ctx).resultType = cast->type;
        return true;
    }

public:
    int sendCount = 0;

    unique_ptr<ast::Assign> postTransformAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        if (handleDeclaration(ctx, asgn)) {
            return asgn;
        }

        auto *id = ast::cast_tree<ast::Ident>(asgn->lhs.get());
        if (id == nullptr) {
            return asgn;
        }

        auto &data = id->symbol.data(ctx);
        if (data.isTypeMember()) {
            ENFORCE(data.isFixed());
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
                        data.resultType = TypeSyntax::getResultType(ctx, hash->values[i], emptySig);
                    }
                }
            }
        } else if (data.isStaticField()) {
            if (data.resultType != nullptr) {
                return asgn;
            }
            data.resultType = resolveConstantType(ctx, asgn->rhs);
        }

        return asgn;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> original) {
        processClassBody(ctx.withOwner(original->symbol), original);
        return original;
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> send) {
        auto *id = ast::cast_tree<ast::Ident>(send->recv.get());
        if (id == nullptr) {
            sendCount++;
            return send;
        }
        if (id->symbol != core::Symbols::T()) {
            sendCount++;
            return send;
        }
        switch (send->fun._id) {
            case core::Names::let()._id:
            case core::Names::assertType()._id:
            case core::Names::cast()._id: {
                if (send->args.size() < 2) {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidCast)) {
                        e.setHeader("Not enough arguments to `{}`: Expected: `{}`, got: `{}`",
                                    "T." + send->fun.toString(ctx), 2, send->args.size());
                    }
                    return send;
                }

                auto expr = move(send->args[0]);
                ParsedSig emptySig;
                auto type = TypeSyntax::getResultType(ctx, send->args[1], emptySig);
                return make_unique<ast::Cast>(send->loc, type, move(expr), send->fun);
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
            return ast::isa_tree<ast::ConstantLit>(asgn->lhs.get());
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
        return make_unique<ast::InsSeq>(klass->loc, move(inits), make_unique<ast::EmptyTree>(core::Loc::none()));
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

        auto nm = core::Names::staticInit();
        if (classDef->symbol == core::Symbols::root()) {
            // Every file may have its own top-level code, so uniqify the names.
            //
            // NOTE(nelhage): In general, we potentially need to do this for
            // every class, since Ruby allows reopening classes. However, since
            // pay-server bans that behavior, this should be OK here.
            nm = ctx.state.freshNameUnique(core::UniqueNameKind::Namer, nm, classDef->loc.file.id());
        }

        auto sym = ctx.state.enterMethodSymbol(inits->loc, classDef->symbol, nm);

        auto init = make_unique<ast::MethodDef>(inits->loc, sym, core::Names::staticInit(),
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

    std::unique_ptr<ast::Expression> addClasses(core::MutableContext ctx, std::unique_ptr<ast::Expression> tree) {
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
            tree = make_unique<ast::InsSeq>(tree->loc, move(stats), move(tree));
            return addClasses(ctx, move(tree));
        }

        for (auto &clas : sortedClasses()) {
            ENFORCE(!!clas);
            insSeq->stats.emplace_back(move(clas));
        }
        return tree;
    }

    std::unique_ptr<ast::Expression> addMethods(core::MutableContext ctx, std::unique_ptr<ast::Expression> tree) {
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

        core::SymbolRef sym = klass.data(ctx).findMemberTransitive(ctx, id->name);
        if (!sym.exists()) {
            if (auto e = ctx.state.beginError(id->loc, core::errors::Resolver::UndeclaredVariable)) {
                e.setHeader("Use of undeclared variable `{}`", id->name.toString(ctx));
            }
            if (id->kind == ast::UnresolvedIdent::Class) {
                sym = ctx.state.enterStaticFieldSymbol(id->loc, klass, id->name);
            } else {
                sym = ctx.state.enterFieldSymbol(id->loc, klass, id->name);
            }
            sym.data(ctx).resultType = core::Types::dynamic();
        }

        return make_unique<ast::Ident>(id->loc, sym);
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
    unique_ptr<ast::Expression> postTransformConstDef(core::MutableContext ctx, unique_ptr<ast::ConstDef> original) {
        ENFORCE(original->symbol != core::Symbols::todo());
        return original;
    }
    unique_ptr<ast::Expression> postTransformIdent(core::MutableContext ctx, unique_ptr<ast::Ident> original) {
        ENFORCE(original->symbol != core::Symbols::todo());
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
};
}; // namespace

std::vector<std::unique_ptr<ast::Expression>> Resolver::run(core::MutableContext ctx,
                                                            std::vector<std::unique_ptr<ast::Expression>> trees) {
    trees = ResolveConstantsWalk::resolveConstants(ctx, move(trees));
    ResolveSignaturesWalk sigs;
    ResolveVariablesWalk vars;

    ctx.trace("Resolving sigs and vars");
    for (auto &tree : trees) {
        tree = ast::TreeMap::apply(ctx, sigs, move(tree));
        tree = ast::TreeMap::apply(ctx, vars, move(tree));

        // declared in here since it holds onto state
        FlattenWalk flatten;
        tree = ast::TreeMap::apply(ctx, flatten, move(tree));
        tree = flatten.addClasses(ctx, move(tree));
        tree = flatten.addMethods(ctx, move(tree));
    }
    core::prodCounterAdd("types.input.sends.total", sigs.sendCount);

    ctx.trace("Finalizing resolution");
    finalizeResolution(ctx.state);

    if (debug_mode) {
        ctx.trace("Sanity checking");
        ResolveSanityCheckWalk sanity;
        for (auto &tree : trees) {
            tree = ast::TreeMap::apply(ctx, sanity, move(tree));
        }
    }

    return trees;
} // namespace namer

} // namespace resolver
} // namespace ruby_typer
