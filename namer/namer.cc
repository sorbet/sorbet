#include "namer/namer.h"
#include "../core/Symbols.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "core/Context.h"
#include "core/Names/namer.h"
#include "core/core.h"
#include "core/errors/namer.h"

#include <unordered_map>

using namespace std;

namespace ruby_typer {
namespace namer {

/**
 * Used with TreeMap to insert all the class and method symbols into the symbol
 * table.
 */
class NameInserter {
    friend class Namer;
    core::SymbolRef squashNames(core::Context ctx, core::SymbolRef owner, unique_ptr<ast::Expression> &node) {
        auto constLit = ast::cast_tree<ast::ConstantLit>(node.get());
        if (constLit == nullptr) {
            ENFORCE(node.get() != nullptr);
            return owner;
        }

        auto newOwner = squashNames(ctx, owner, constLit->scope);
        core::SymbolRef existing = newOwner.info(ctx).findMember(ctx, constLit->cnst);
        if (existing.exists()) {
            return existing;
        }
        auto sym = ctx.state.enterClassSymbol(constLit->loc, newOwner, constLit->cnst);
        sym.info(ctx).resultType = make_unique<core::ClassType>(sym.info(ctx).singletonClass(ctx));
        return sym;
    }

    core::SymbolRef arg2Symbol(core::Context ctx, ast::Expression *arg) {
        auto loc = arg->loc;
        bool optional = false, keyword = false, block = false, repeated = false;

        while (true) {
            if (ast::UnresolvedIdent *nm = ast::cast_tree<ast::UnresolvedIdent>(arg)) {
                core::SymbolRef sym = ctx.state.enterMethodArgumentSymbol(loc, ctx.owner, nm->name);
                core::Symbol &info = sym.info(ctx);
                if (optional) {
                    info.setOptional();
                }
                if (keyword) {
                    info.setKeyword();
                }
                if (block) {
                    info.setBlockArgument();
                }
                if (repeated) {
                    info.setRepeated();
                }
                return sym;
            }
            typecase(arg,
                     [&](ast::RestArg *rest) {
                         repeated = true;
                         arg = rest->expr.get();
                     },
                     [&](ast::KeywordArg *kw) {
                         keyword = true;
                         arg = kw->expr.get();
                     },
                     [&](ast::OptionalArg *opt) {
                         optional = true;
                         arg = opt->expr.get();
                     },
                     [&](ast::BlockArg *opt) {
                         block = true;
                         arg = opt->expr.get();
                     },
                     [&](ast::ShadowArg *opt) { arg = opt->expr.get(); });
        }
    }

    struct LocalFrame {
        unordered_map<core::NameRef, core::LocalVariable> locals;
        bool moduleFunctionActive;
    };

    vector<LocalFrame> scopeStack;

    unique_ptr<ast::Expression> addAncestor(core::Context ctx, ast::ClassDef *klass,
                                            unique_ptr<ast::Expression> &node) {
        auto send = ast::cast_tree<ast::Send>(node.get());
        if (send == nullptr) {
            ENFORCE(node.get() != nullptr);
            return nullptr;
        }

        if (send->fun != core::Names::include()) {
            return nullptr;
        }
        if (!ast::isa_tree<ast::Self>(send->recv.get())) {
            // ignore `something.include`
            return nullptr;
        }

        if (send->args.size() != 1) {
            ctx.state.error(send->loc, core::errors::Namer::IncludeMutipleParam,
                            "`include` should only be passed a single constant. You passed {} parameters.",
                            send->args.size());
            return nullptr;
        }
        auto constLit = ast::cast_tree<ast::ConstantLit>(send->args[0].get());
        if (constLit == nullptr) {
            ctx.state.error(send->loc, core::errors::Namer::IncludeNotConstant,
                            "`include` must be passed a constant literal. You passed {}.",
                            send->args[0]->toString(ctx));
            return nullptr;
        }
        if (send->block != nullptr) {
            ctx.state.error(send->loc, core::errors::Namer::IncludePassedBlock, "`include` can not be passed a block.");
            return nullptr;
        }
        // TODO check that send->block is empty
        return move(send->args[0]);
    }

    void aliasMethod(core::Context ctx, core::SymbolRef owner, core::NameRef newName, core::SymbolRef method) {
        core::SymbolRef alias = ctx.state.enterMethodSymbol(method.info(ctx).definitionLoc, owner, newName);
        alias.info(ctx).resultType = make_shared<core::AliasType>(method);
    }

    void aliasModuleFunction(core::Context ctx, core::SymbolRef method) {
        core::SymbolRef owner = method.info(ctx).owner;
        aliasMethod(ctx, owner.info(ctx).singletonClass(ctx), method.info(ctx).name, method);
    }

    core::SymbolRef methodOwner(core::Context ctx) {
        core::SymbolRef owner = ctx.enclosingClass();
        if (owner == core::GlobalState::noSymbol()) {
            // Root methods end up going on object
            owner = core::GlobalState::defn_Object();
        }
        return owner;
    }

public:
    ast::ClassDef *preTransformClassDef(core::Context ctx, ast::ClassDef *klass) {
        klass->symbol = squashNames(ctx, ctx.owner, klass->name);
        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass->name.get());
        if ((ident != nullptr) && ident->name == core::Names::singletonClass()) {
            ENFORCE(ident->kind == ast::UnresolvedIdent::Class);
            klass->symbol = ctx.contextClass().info(ctx).singletonClass(ctx);
        } else {
            switch (klass->kind) {
                case ast::ClassDefKind::Class:
                    klass->symbol.info(ctx).setIsModule(false);
                    break;
                case ast::ClassDefKind ::Module:
                    klass->symbol.info(ctx).setIsModule(true);
                    break;
            }
        }
        scopeStack.emplace_back();
        return klass;
    }

    ast::ClassDef *postTransformClassDef(core::Context ctx, ast::ClassDef *klass) {
        scopeStack.pop_back();
        if (klass->kind == ast::Class && !klass->symbol.info(ctx).superClass.exists() &&
            klass->symbol != core::GlobalState::defn_BasicObject()) {
            klass->symbol.info(ctx).superClass = core::GlobalState::defn_todo();
        }

        auto toRemove =
            remove_if(klass->rhs.begin(), klass->rhs.end(), [this, ctx, klass](unique_ptr<ast::Expression> &line) {
                auto newAncestor = addAncestor(ctx, klass, line);
                if (newAncestor) {
                    klass->ancestors.emplace_back(move(newAncestor));
                    return true;
                }
                return false;
            });
        klass->symbol.info(ctx).definitionLoc = klass->loc;
        klass->symbol.info(ctx).resultType = make_unique<core::ClassType>(klass->symbol.info(ctx).singletonClass(ctx));
        klass->rhs.erase(toRemove, klass->rhs.end());
        return klass;
    }

    void fillInArgs(core::Context ctx, ast::MethodDef::ARGS_store &args) {
        bool inShadows = false;

        for (auto &arg : args) {
            core::NameRef name;

            if (ast::ShadowArg *sarg = ast::cast_tree<ast::ShadowArg>(arg.get())) {
                auto id = ast::cast_tree<ast::UnresolvedIdent>(sarg->expr.get());
                ENFORCE(id != nullptr);
                name = id->name;
                inShadows = true;
            } else {
                ENFORCE(!inShadows, "shadow argument followed by non-shadow argument!");
                core::SymbolRef sym = arg2Symbol(ctx, arg.get());
                ctx.owner.info(ctx).argumentsOrMixins.push_back(sym);
                name = sym.info(ctx).name;
            }

            core::LocalVariable local = ctx.state.enterLocalSymbol(ctx.owner, name);
            scopeStack.back().locals[name] = local;

            unique_ptr<ast::Expression> localExpr = make_unique<ast::Local>(arg->loc, local);
            arg.swap(localExpr);
        }
    }

    ast::Expression *postTransformSend(core::Context ctx, ast::Send *original) {
        if (original->args.size() == 1 && ast::isa_tree<ast::ZSuperArgs>(original->args[0].get())) {
            original->args.clear();
            core::SymbolRef method = ctx.enclosingMethod();
            if (method.exists()) {
                for (auto arg : ctx.enclosingMethod().info(ctx).argumentsOrMixins) {
                    original->args.emplace_back(make_unique<ast::Ident>(original->loc, arg));
                }
            } else {
                ctx.state.error(original->loc, core::errors::Namer::SelfOutsideClass, "super outside of method");
            }
        }
        ast::MethodDef *mdef;
        if (original->args.size() == 1 && (mdef = ast::cast_tree<ast::MethodDef>(original->args[0].get())) != nullptr) {
            switch (original->fun._id) {
                case core::Names::private_()._id:
                case core::Names::privateClassMethod()._id:
                    mdef->symbol.info(ctx).setPrivate();
                    break;
                case core::Names::protected_()._id:
                    mdef->symbol.info(ctx).setProtected();
                    break;
                case core::Names::public_()._id:
                    mdef->symbol.info(ctx).setPublic();
                    break;
                case core::Names::moduleFunction()._id:
                    aliasModuleFunction(ctx, mdef->symbol);
                    break;
                default:
                    return original;
            }
            return original->args[0].release();
        }
        if (ast::isa_tree<ast::Self>(original->recv.get())) {
            switch (original->fun._id) {
                case core::Names::moduleFunction()._id: {
                    if (original->args.empty()) {
                        scopeStack.back().moduleFunctionActive = true;
                        break;
                    }
                    for (auto &arg : original->args) {
                        auto sym = ast::cast_tree<ast::SymbolLit>(arg.get());
                        if (sym == nullptr) {
                            ctx.state.error(arg->loc, core::errors::Namer::DynamicDSLInvocation,
                                            "Unsupported argument to {}: arguments must be symbol literals",
                                            original->fun.toString(ctx));
                            continue;
                        }
                        core::SymbolRef meth = methodOwner(ctx).info(ctx).findMember(ctx, sym->name);
                        if (!meth.exists()) {
                            ctx.state.error(arg->loc, core::errors::Namer::MethodNotFound, "{}: no such method: {}",
                                            original->fun.toString(ctx), sym->name.toString(ctx));
                            continue;
                        }
                        aliasModuleFunction(ctx, meth);
                    }
                    break;
                }
                case core::Names::aliasMethod()._id: {
                    vector<core::NameRef> args;
                    for (auto &arg : original->args) {
                        auto sym = ast::cast_tree<ast::SymbolLit>(arg.get());
                        if (sym == nullptr) {
                            ctx.state.error(arg->loc, core::errors::Namer::DynamicDSLInvocation,
                                            "Unsupported argument to {}: arguments must be symbol literals",
                                            original->fun.toString(ctx));
                            continue;
                        }
                        args.push_back(sym->name);
                    }
                    if (original->args.size() != 2) {
                        ctx.state.error(original->loc, core::errors::Namer::InvalidAlias,
                                        "Wrong number of arguments to {}; Expected 2", original->fun.toString(ctx));
                        break;
                    }
                    if (args.size() != 2) {
                        break;
                    }
                    core::SymbolRef meth = methodOwner(ctx).info(ctx).findMember(ctx, args[1]);
                    if (!meth.exists()) {
                        ctx.state.error(original->args[1]->loc, core::errors::Namer::MethodNotFound,
                                        "{}: no such method: {}", original->fun.toString(ctx), args[1].toString(ctx));
                        break;
                    }
                    aliasMethod(ctx, methodOwner(ctx), args[0], meth);
                    break;
                }
            }
        }

        return original;
    }

    ast::MethodDef *preTransformMethodDef(core::Context ctx, ast::MethodDef *method) {
        scopeStack.emplace_back();
        core::SymbolRef owner = methodOwner(ctx);

        if (method->isSelf) {
            if (owner.info(ctx).isClass()) {
                owner = owner.info(ctx).singletonClass(ctx);
            }
        }
        ENFORCE(owner.info(ctx).isClass());

        auto sym = owner.info(ctx).findMember(ctx, method->name);
        if (sym.exists()) {
            ctx.state.error(
                core::ComplexError(method->loc, core::errors::Namer::RedefinitionOfMethod,
                                   method->name.toString(ctx) + ": Method redefined",
                                   core::ErrorLine::from(sym.info(ctx).definitionLoc, "Original definition")));
            method->symbol = sym;
            // TODO Check that the previous args match the new ones instead of
            // just wiping the original ones
            sym.info(ctx).arguments().clear();
        }
        method->symbol = ctx.state.enterMethodSymbol(method->loc, owner, method->name);
        fillInArgs(ctx.withOwner(method->symbol), method->args);
        method->symbol.info(ctx).definitionLoc = method->loc;

        pushEnclosingArgs(move(method->args));

        return method;
    }

    ast::MethodDef *postTransformMethodDef(core::Context ctx, ast::MethodDef *method) {
        method->args = popEnclosingArgs();
        ENFORCE(method->args.size() == method->symbol.info(ctx).arguments().size());
        scopeStack.pop_back();
        if (scopeStack.back().moduleFunctionActive) {
            aliasModuleFunction(ctx, method->symbol);
        }
        ENFORCE(method->args.size() == method->symbol.info(ctx).arguments().size(), method->name.toString(ctx), ": ",
                method->args.size(), " != ", method->symbol.info(ctx).arguments().size());
        // All this info is now in the symbol, lets not keep detritus to aide confusion.
        method->args.clear();
        return method;
    }

    ast::Block *preTransformBlock(core::Context ctx, ast::Block *blk) {
        core::SymbolRef owner = ctx.owner;
        if (owner == core::GlobalState::noSymbol()) {
            // Root methods end up going on object
            owner = core::GlobalState::defn_Object();
        }
        blk->symbol =
            ctx.state.enterMethodSymbol(blk->loc, owner,
                                        ctx.state.freshNameUnique(core::UniqueNameKind::Namer, core::Names::blockTemp(),
                                                                  ++(owner.info(ctx).uniqueCounter)));

        scopeStack.emplace_back();
        auto &parent = *(scopeStack.end() - 2);
        auto &frame = scopeStack.back();

        // We inherit our parent's locals
        for (auto &binding : parent.locals) {
            frame.locals.insert(binding);
        }

        // If any of our arguments shadow our parent, fillInArgs will overwrite
        // them in `frame.locals`
        fillInArgs(ctx.withOwner(blk->symbol), blk->args);

        pushEnclosingArgs(move(blk->args));

        return blk;
    }

    ast::Block *postTransformBlock(core::Context ctx, ast::Block *blk) {
        blk->args = popEnclosingArgs();
        scopeStack.pop_back();
        return blk;
    }

    ast::Expression *postTransformUnresolvedIdent(core::Context ctx, ast::UnresolvedIdent *nm) {
        switch (nm->kind) {
            case ast::UnresolvedIdent::Local: {
                auto &frame = scopeStack.back();
                core::LocalVariable &cur = frame.locals[nm->name];
                if (!cur.exists()) {
                    cur = ctx.state.enterLocalSymbol(ctx.owner, nm->name);
                    frame.locals[nm->name] = cur;
                }
                return new ast::Local(nm->loc, cur);
            }
            case ast::UnresolvedIdent::Global: {
                core::Symbol &root = ctx.state.defn_root().info(ctx);
                core::SymbolRef sym = root.findMember(ctx, nm->name);
                if (!sym.exists()) {
                    sym = ctx.state.enterFieldSymbol(nm->loc, ctx.state.defn_root(), nm->name);
                }
                return new ast::Ident(nm->loc, sym);
            }
            default:
                return nm;
        }
    }

    ast::Self *postTransformSelf(core::Context ctx, ast::Self *self) {
        self->claz = ctx.selfClass();
        return self;
    }

    ast::Assign *fillAssign(core::Context ctx, ast::ConstantLit *lhs, ast::Assign *asgn) {
        // TODO(nelhage): forbid dynamic constant definition
        core::SymbolRef scope = squashNames(ctx, ctx.owner, lhs->scope);
        core::SymbolRef cnst = ctx.state.enterStaticFieldSymbol(lhs->loc, scope, lhs->cnst);
        auto loc = lhs->loc;
        asgn->lhs = make_unique<ast::Ident>(loc, cnst);
        return asgn;
    }

    ast::Expression *postTransformAssign(core::Context ctx, ast::Assign *asgn) {
        ast::ConstantLit *lhs = ast::cast_tree<ast::ConstantLit>(asgn->lhs.get());
        if (lhs == nullptr) {
            return asgn;
        }

        auto *send = ast::cast_tree<ast::Send>(asgn->rhs.get());
        if (send == nullptr || send->fun != core::Names::typeDecl()) {
            return fillAssign(ctx, lhs, asgn);
        }
        auto *shouldBeT = ast::cast_tree<ast::ConstantLit>(send->recv.get());
        if (shouldBeT == nullptr || !(shouldBeT->cnst.name(ctx).kind == core::NameKind::CONSTANT &&
                                      shouldBeT->cnst.name(ctx).cnst.original == core::Names::T())) {
            return fillAssign(ctx, lhs, asgn);
        }

        auto *typeName = ast::cast_tree<ast::ConstantLit>(asgn->lhs.get());
        if (typeName == nullptr) {
            return fillAssign(ctx, lhs, asgn);
        }

        core::Variance variance;
        if (send->args.empty()) {
            variance = core::Variance::Invariant;
        } else {
            auto *symbol = ast::cast_tree<ast::SymbolLit>(send->args[0].get());
            if (send->args.size() > 1 || symbol == nullptr) {
                ctx.state.error(asgn->loc, core::errors::Namer::InvalidTypeDefinition, "Invalid type definition");
                return new ast::EmptyTree(asgn->loc);
            }
            if (symbol->name == core::Names::covariant()) {
                variance = core::Variance::CoVariant;
            } else if (symbol->name == core::Names::contravariant()) {
                variance = core::Variance::ContraVariant;
            } else if (symbol->name == core::Names::invariant()) {
                variance = core::Variance::Invariant;
            } else {
                ctx.state.error(asgn->loc, core::errors::Namer::InvalidTypeDefinition,
                                "Invalid variance kind, only :{} and :{} are supported",
                                core::Names::covariant().toString(ctx), core::Names::contravariant().toString(ctx));
                variance = core::Variance::Invariant;
            }
        }
        ctx.state.enterTypeMember(asgn->loc, ctx.enclosingClass(), typeName->cnst, variance);
        return new ast::EmptyTree(asgn->loc);
    }

    ast::Expression *postTransformYield(core::Context ctx, ast::Yield *yield) {
        auto method = ctx.enclosingMethod();
        core::SymbolRef blockArg;
        for (auto arg : method.info(ctx).arguments()) {
            if (arg.info(ctx).isBlockArgument()) {
                blockArg = arg;
                break;
            }
        }
        if (!blockArg) {
            auto name = core::Names::blkArg();
            blockArg = ctx.state.enterMethodArgumentSymbol(yield->loc, method, name);
            blockArg.info(ctx).setBlockArgument();
            blockArg.info(ctx).resultType = core::Types::dynamic();
            method.info(ctx).argumentsOrMixins.push_back(blockArg);

            // Also put it in the MethodDef since we rely on that being correct for blocks
            core::LocalVariable local = ctx.state.enterLocalSymbol(ctx.owner, name);
            scopeStack.back().locals[name] = local;
            auto localExpr = make_unique<ast::Local>(yield->loc, local);
            peekEnclosingArgs().emplace_back(move(localExpr));
        }

        auto name = blockArg.info(ctx).name;
        core::LocalVariable local = ctx.state.enterLocalSymbol(ctx.owner, name);
        scopeStack.back().locals[name] = local;
        auto recv = make_unique<ast::Local>(yield->loc, local);
        ast::Send::ARGS_store nargs;
        nargs.emplace_back(move(yield->expr));

        return new ast::Send(yield->loc, move(recv), core::Names::call(), move(nargs));
    }

private:
    NameInserter() {
        scopeStack.emplace_back();
    }

    vector<ast::MethodDef::ARGS_store> enclosingArgsStack;
    void pushEnclosingArgs(ast::MethodDef::ARGS_store arg) {
        enclosingArgsStack.emplace_back(move(arg));
    }
    ast::MethodDef::ARGS_store popEnclosingArgs() {
        auto ret = move(enclosingArgsStack.back());
        enclosingArgsStack.pop_back();
        return ret;
    }
    ast::MethodDef::ARGS_store &peekEnclosingArgs() {
        return enclosingArgsStack.back();
    }
}; // namespace namer

unique_ptr<ast::Expression> Namer::run(core::Context ctx, unique_ptr<ast::Expression> tree) {
    NameInserter nameInserter;
    return ast::TreeMap<NameInserter>::apply(ctx, nameInserter, move(tree));
}

} // namespace namer
}; // namespace ruby_typer
