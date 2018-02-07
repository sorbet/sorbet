#include "namer/namer.h"
#include "../core/Context.h"
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
        core::SymbolRef existing = newOwner.data(ctx).findMember(ctx, constLit->cnst);
        if (existing.exists()) {
            return existing;
        }
        auto sym = ctx.state.enterClassSymbol(constLit->loc, newOwner, constLit->cnst);
        sym.data(ctx).resultType = make_unique<core::ClassType>(sym.data(ctx).singletonClass(ctx));
        return sym;
    }

    core::SymbolRef arg2Symbol(core::Context ctx, ast::Expression *arg) {
        auto loc = arg->loc;
        bool optional = false, keyword = false, block = false, repeated = false;

        while (true) {
            if (ast::UnresolvedIdent *nm = ast::cast_tree<ast::UnresolvedIdent>(arg)) {
                core::SymbolRef sym = ctx.state.enterMethodArgumentSymbol(loc, ctx.owner, nm->name);
                core::Symbol &data = sym.data(ctx);
                if (optional) {
                    data.setOptional();
                }
                if (keyword) {
                    data.setKeyword();
                }
                if (block) {
                    data.setBlockArgument();
                }
                if (repeated) {
                    data.setRepeated();
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
    u4 scopeId;

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
                            "`include` must be passed a constant literal. You passed a {}.", send->args[0]->nodeName());
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
        core::SymbolRef alias = ctx.state.enterMethodSymbol(method.data(ctx).definitionLoc, owner, newName);
        alias.data(ctx).resultType = make_shared<core::AliasType>(method);
    }

    void aliasModuleFunction(core::Context ctx, core::SymbolRef method) {
        core::SymbolRef owner = method.data(ctx).owner;
        aliasMethod(ctx, owner.data(ctx).singletonClass(ctx), method.data(ctx).name, method);
    }

    core::SymbolRef methodOwner(core::Context ctx) {
        core::SymbolRef owner = ctx.owner.data(ctx).enclosingClass(ctx);
        if (owner == core::Symbols::noSymbol()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        return owner;
    }

public:
    ast::ClassDef *preTransformClassDef(core::Context ctx, ast::ClassDef *klass) {
        if (klass->symbol == core::Symbols::todo()) {
            klass->symbol = squashNames(ctx, ctx.owner, klass->name);
        } else {
            // Desugar populates a top-level root() ClassDef. Nothing else
            // should have been resolved by now.
            ENFORCE(klass->symbol == core::Symbols::root());
        }
        auto *ident = ast::cast_tree<ast::UnresolvedIdent>(klass->name.get());
        if ((ident != nullptr) && ident->name == core::Names::singleton()) {
            ENFORCE(ident->kind == ast::UnresolvedIdent::Class);
            klass->symbol = ctx.contextClass().data(ctx).singletonClass(ctx);
        } else {
            switch (klass->kind) {
                case ast::ClassDefKind::Class:
                    klass->symbol.data(ctx).setIsModule(false);
                    break;
                case ast::ClassDefKind ::Module:
                    klass->symbol.data(ctx).setIsModule(true);
                    break;
            }
        }
        scopeStack.emplace_back();
        scopeId = 0;
        return klass;
    }

    ast::ClassDef *postTransformClassDef(core::Context ctx, ast::ClassDef *klass) {
        scopeStack.pop_back();
        if (klass->kind == ast::Class && !klass->symbol.data(ctx).superClass.exists() &&
            klass->symbol != core::Symbols::BasicObject()) {
            klass->symbol.data(ctx).superClass = core::Symbols::todo();
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
        klass->symbol.data(ctx).definitionLoc = klass->loc;
        klass->symbol.data(ctx).resultType = make_unique<core::ClassType>(klass->symbol.data(ctx).singletonClass(ctx));
        klass->rhs.erase(toRemove, klass->rhs.end());
        return klass;
    }

    core::LocalVariable enterLocal(core::Context ctx, core::NameRef name) {
        if (!ctx.owner.data(ctx).isBlockSymbol(ctx)) {
            return core::LocalVariable(name, 0);
        }
        return core::LocalVariable(name, scopeId);
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
                ctx.owner.data(ctx).argumentsOrMixins.push_back(sym);
                name = sym.data(ctx).name;
            }

            core::LocalVariable local = enterLocal(ctx, name);
            scopeStack.back().locals[name] = local;

            unique_ptr<ast::Expression> localExpr = make_unique<ast::Local>(arg->loc, local);
            arg.swap(localExpr);
        }
    }

    ast::Expression *postTransformSend(core::Context ctx, ast::Send *original) {
        if (original->args.size() == 1 && ast::isa_tree<ast::ZSuperArgs>(original->args[0].get())) {
            original->args.clear();
            core::SymbolRef method = ctx.owner.data(ctx).enclosingMethod(ctx);
            if (method.exists()) {
                for (auto arg : ctx.owner.data(ctx).enclosingMethod(ctx).data(ctx).argumentsOrMixins) {
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
                    mdef->symbol.data(ctx).setPrivate();
                    break;
                case core::Names::protected_()._id:
                    mdef->symbol.data(ctx).setProtected();
                    break;
                case core::Names::public_()._id:
                    mdef->symbol.data(ctx).setPublic();
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
                        core::SymbolRef meth = methodOwner(ctx).data(ctx).findMember(ctx, sym->name);
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
                    core::SymbolRef meth = methodOwner(ctx).data(ctx).findMember(ctx, args[1]);
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
        ++scopeId;
        core::SymbolRef owner = methodOwner(ctx);

        if (method->isSelf) {
            if (owner.data(ctx).isClass()) {
                owner = owner.data(ctx).singletonClass(ctx);
            }
        }
        ENFORCE(owner.data(ctx).isClass());

        auto sym = owner.data(ctx).findMember(ctx, method->name);
        if (sym.exists()) {
            if (method->loc == sym.data(ctx).definitionLoc) {
                // Reparsing the same file
                method->symbol = sym;
                pushEnclosingArgs(move(method->args));
                return method;
            }
            ctx.state.error(
                core::ComplexError(method->loc, core::errors::Namer::RedefinitionOfMethod,
                                   method->name.toString(ctx) + ": Method redefined",
                                   core::ErrorLine::from(sym.data(ctx).definitionLoc, "Previous definition")));
            // TODO Check that the previous args match the new ones instead of
            // just moving the original one to the side
            ctx.state.mangleRenameSymbol(sym, method->name, core::UniqueNameKind::Namer);
        }
        method->symbol = ctx.state.enterMethodSymbol(method->loc, owner, method->name);
        fillInArgs(ctx.withOwner(method->symbol), method->args);
        method->symbol.data(ctx).definitionLoc = method->loc;

        pushEnclosingArgs(move(method->args));

        return method;
    }

    ast::MethodDef *postTransformMethodDef(core::Context ctx, ast::MethodDef *method) {
        method->args = popEnclosingArgs();
        ENFORCE(method->args.size() == method->symbol.data(ctx).arguments().size());
        scopeStack.pop_back();
        if (scopeStack.back().moduleFunctionActive) {
            aliasModuleFunction(ctx, method->symbol);
        }
        ENFORCE(method->args.size() == method->symbol.data(ctx).arguments().size(), method->name.toString(ctx), ": ",
                method->args.size(), " != ", method->symbol.data(ctx).arguments().size());
        // All this info is now in the symbol, lets not keep detritus to aide confusion.
        method->args.clear();
        return method;
    }

    ast::Block *preTransformBlock(core::Context ctx, ast::Block *blk) {
        core::SymbolRef owner = ctx.owner;
        if (owner == core::Symbols::noSymbol()) {
            // Root methods end up going on object
            owner = core::Symbols::Object();
        }
        blk->symbol =
            ctx.state.enterMethodSymbol(blk->loc, owner,
                                        ctx.state.freshNameUnique(core::UniqueNameKind::Namer, core::Names::blockTemp(),
                                                                  ++(owner.data(ctx).uniqueCounter)));

        scopeStack.emplace_back();
        ++scopeId;
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
                    cur = enterLocal(ctx, nm->name);
                    frame.locals[nm->name] = cur;
                }
                return new ast::Local(nm->loc, cur);
            }
            case ast::UnresolvedIdent::Global: {
                core::Symbol &root = core::Symbols::root().data(ctx);
                core::SymbolRef sym = root.findMember(ctx, nm->name);
                if (!sym.exists()) {
                    sym = ctx.state.enterFieldSymbol(nm->loc, core::Symbols::root(), nm->name);
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
        if (send == nullptr) {
            return fillAssign(ctx, lhs, asgn);
        }
        auto *shouldBeT = ast::cast_tree<ast::ConstantLit>(send->recv.get());
        if (shouldBeT == nullptr || !(shouldBeT->cnst.data(ctx).kind == core::NameKind::CONSTANT &&
                                      shouldBeT->cnst.data(ctx).cnst.original == core::Names::T())) {
            return fillAssign(ctx, lhs, asgn);
        }

        auto *typeName = ast::cast_tree<ast::ConstantLit>(asgn->lhs.get());
        if (typeName == nullptr) {
            return fillAssign(ctx, lhs, asgn);
        }

        switch (send->fun._id) {
            case core::Names::typeDecl()._id: {
                core::Variance variance = core::Variance::Invariant;

                if (!send->args.empty()) {
                    if (send->args.size() > 2) {
                        ctx.state.error(send->loc, core::errors::Namer::InvalidTypeDefinition,
                                        "Too many args in type definition");
                        return new ast::EmptyTree(asgn->loc);
                    }
                    auto *symbol = ast::cast_tree<ast::SymbolLit>(send->args[0].get());
                    if (symbol) {
                        if (symbol->name == core::Names::covariant()) {
                            variance = core::Variance::CoVariant;
                        } else if (symbol->name == core::Names::contravariant()) {
                            variance = core::Variance::ContraVariant;
                        } else if (symbol->name == core::Names::invariant()) {
                            variance = core::Variance::Invariant;
                        } else {
                            ctx.state.error(symbol->loc, core::errors::Namer::InvalidTypeDefinition,
                                            "Invalid variance kind, only :{} and :{} are supported",
                                            core::Names::covariant().toString(ctx),
                                            core::Names::contravariant().toString(ctx));
                        }
                    } else {
                        if (send->args.size() != 1 || ast::cast_tree<ast::Hash>(send->args[0].get()) == nullptr) {
                            ctx.state.error(send->args[0]->loc, core::errors::Namer::InvalidTypeDefinition,
                                            "Invalid param, must be a :symbol");
                        }
                    }
                }

                auto sym = ctx.state.enterTypeMember(asgn->loc, ctx.owner.data(ctx).enclosingClass(ctx), typeName->cnst,
                                                     variance);

                if (!send->args.empty()) {
                    auto *hash = ast::cast_tree<ast::Hash>(send->args.back().get());
                    if (hash) {
                        int i = -1;
                        for (auto &keyExpr : hash->keys) {
                            i++;
                            auto *key = ast::cast_tree<ast::SymbolLit>(keyExpr.get());
                            if (key && key->name == core::Names::fixed()) {
                                // Leave it in the tree for the resolver to chew on.
                                sym.data(ctx).setFixed();
                                asgn->lhs = make_unique<ast::Ident>(asgn->lhs->loc, sym);
                                return asgn;
                            }
                        }
                        ctx.state.error(send->loc, core::errors::Namer::InvalidTypeDefinition,
                                        "Missing required param :fixed");
                    }
                }
                return new ast::EmptyTree(asgn->loc);
            }
            default:
                return fillAssign(ctx, lhs, asgn);
        }
    }

    ast::Expression *postTransformYield(core::Context ctx, ast::Yield *yield) {
        auto method = ctx.owner.data(ctx).enclosingMethod(ctx);
        core::SymbolRef blockArg;
        for (auto arg : method.data(ctx).arguments()) {
            if (arg.data(ctx).isBlockArgument()) {
                blockArg = arg;
                break;
            }
        }
        if (!blockArg) {
            auto name = core::Names::blkArg();
            blockArg = ctx.state.enterMethodArgumentSymbol(yield->loc, method, name);
            blockArg.data(ctx).setBlockArgument();
            blockArg.data(ctx).resultType = core::Types::dynamic();
            method.data(ctx).argumentsOrMixins.push_back(blockArg);

            // Also put it in the MethodDef since we rely on that being correct for blocks
            core::LocalVariable local = enterLocal(ctx, name);
            scopeStack.back().locals[name] = local;
            auto localExpr = make_unique<ast::Local>(yield->loc, local);
            peekEnclosingArgs().emplace_back(move(localExpr));
        }

        auto name = blockArg.data(ctx).name;
        core::LocalVariable local = enterLocal(ctx, name);
        scopeStack.back().locals[name] = local;
        auto recv = make_unique<ast::Local>(yield->loc, local);
        ast::Send::ARGS_store nargs;
        nargs.emplace_back(move(yield->expr));

        return new ast::Send(yield->loc, move(recv), core::Names::call(), move(nargs));
    }

private:
    NameInserter() {
        scopeStack.emplace_back();
        scopeId = 0;
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
