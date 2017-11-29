#include "namer/namer.h"
#include "../ast/ast.h"
#include "../core/Context.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "core/core.h"

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
            Error::check(node.get() != nullptr);
            return owner;
        }

        auto newOwner = squashNames(ctx, owner, constLit->scope);
        return ctx.state.enterClassSymbol(constLit->loc, newOwner, constLit->cnst);
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
    };

    vector<LocalFrame> scopeStack;

    unique_ptr<ast::Expression> addAncestor(core::Context ctx, ast::ClassDef *klass,
                                            unique_ptr<ast::Expression> &node) {
        auto send = ast::cast_tree<ast::Send>(node.get());
        if (send == nullptr) {
            Error::check(node.get() != nullptr);
            return nullptr;
        }

        if (send->fun != core::Names::include()) {
            return nullptr;
        }
        if (ast::cast_tree<ast::Self>(send->recv.get()) == nullptr) {
            // ignore `something.include`
            return nullptr;
        }

        if (send->args.size() != 1) {
            ctx.state.errors.error(send->loc, core::ErrorClass::IncludeMutipleParam,
                                   "`include` should only be passed a single constant. You passed {} parameters.",
                                   send->args.size());
            return nullptr;
        }
        auto constLit = ast::cast_tree<ast::ConstantLit>(send->args[0].get());
        if (constLit == nullptr) {
            ctx.state.errors.error(send->loc, core::ErrorClass::IncludeNotConstant,
                                   "`include` must be passed a constant literal. You passed {}.",
                                   send->args[0]->toString(ctx));
            return nullptr;
        }
        if (send->block != nullptr) {
            ctx.state.errors.error(send->loc, core::ErrorClass::IncludePassedBlock,
                                   "`include` can not be passed a block.");
            return nullptr;
        }
        // TODO check that send->block is empty
        return move(send->args[0]);
    }

public:
    ast::ClassDef *preTransformClassDef(core::Context ctx, ast::ClassDef *klass) {
        klass->symbol = squashNames(ctx, ctx.owner, klass->name);
        scopeStack.emplace_back();
        return klass;
    }

    ast::ClassDef *postTransformClassDef(core::Context ctx, ast::ClassDef *klass) {
        scopeStack.pop_back();
        klass->symbol = squashNames(ctx, ctx.owner, klass->name);
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
        klass->rhs.erase(toRemove, klass->rhs.end());
        return klass;
    }

    void fillInArgs(core::Context ctx, ast::MethodDef::ARGS_store &args) {
        bool inShadows = false;

        for (auto &arg : args) {
            core::NameRef name;

            if (ast::ShadowArg *sarg = ast::cast_tree<ast::ShadowArg>(arg.get())) {
                auto id = ast::cast_tree<ast::UnresolvedIdent>(sarg->expr.get());
                Error::check(id != nullptr);
                name = id->name;
                inShadows = true;
            } else {
                Error::check(!inShadows, "shadow argument followed by non-shadow argument!");
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

    ast::Send *preTransformSend(core::Context ctx, ast::Send *original) {
        if (original->args.size() == 1 && ast::cast_tree<ast::ZSuperArgs>(original->args[0].get()) != nullptr) {
            original->args.clear();
            core::SymbolRef method = ctx.enclosingMethod();
            if (method.exists()) {
                for (auto arg : ctx.enclosingMethod().info(ctx).argumentsOrMixins) {
                    original->args.emplace_back(make_unique<ast::Ident>(original->loc, arg));
                }
            } else {
                ctx.state.errors.error(original->loc, core::ErrorClass::SelfOutsideClass, "super outside of method");
            }
        }
        return original;
    }

    ast::MethodDef *preTransformMethodDef(core::Context ctx, ast::MethodDef *method) {
        scopeStack.emplace_back();
        core::SymbolRef owner = ctx.enclosingClass();
        if (owner == core::GlobalState::noSymbol()) {
            // Root methods end up going on object
            owner = core::GlobalState::defn_Object();
        }

        if (method->isSelf) {
            if (owner.info(ctx).isClass()) {
                owner = owner.info(ctx).singletonClass(ctx);
            }
        }
        Error::check(owner.info(ctx).isClass());

        method->symbol = ctx.state.enterMethodSymbol(method->loc, owner, method->name);
        fillInArgs(ctx.withOwner(method->symbol), method->args);
        method->symbol.info(ctx).definitionLoc = method->loc;

        return method;
    }

    ast::MethodDef *postTransformMethodDef(core::Context ctx, ast::MethodDef *method) {
        scopeStack.pop_back();
        return method;
    }

    ast::Block *preTransformBlock(core::Context ctx, ast::Block *blk) {
        core::SymbolRef owner = ctx.owner;
        if (owner == core::GlobalState::noSymbol()) {
            // Root methods end up going on object
            owner = core::GlobalState::defn_Object();
        }
        blk->symbol = ctx.state.enterMethodSymbol(
            blk->loc, owner, ctx.state.freshNameUnique(core::UniqueNameKind::Namer, core::Names::blockTemp()));

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

        return blk;
    }

    ast::Block *postTransformBlock(core::Context ctx, ast::Block *blk) {
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
                core::SymbolRef sym = root.findMember(nm->name);
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

    ast::Assign *postTransformAssign(core::Context ctx, ast::Assign *asgn) {
        ast::ConstantLit *lhs = ast::cast_tree<ast::ConstantLit>(asgn->lhs.get());
        if (lhs == nullptr) {
            return asgn;
        }

        // TODO(nelhage): forbid dynamic constant definition
        core::SymbolRef scope = squashNames(ctx, ctx.owner, lhs->scope);
        core::SymbolRef cnst = ctx.state.enterStaticFieldSymbol(lhs->loc, scope, lhs->cnst);
        cnst.info(ctx).resultType = core::Types::dynamic();

        return new ast::Assign(asgn->loc, make_unique<ast::Ident>(lhs->loc, cnst), move(asgn->rhs));
    }

private:
    NameInserter() {
        scopeStack.emplace_back();
    }
};

unique_ptr<ast::Expression> Namer::run(core::Context &ctx, unique_ptr<ast::Expression> tree) {
    NameInserter nameInserter;
    return ast::TreeMap<NameInserter>::apply(ctx, nameInserter, move(tree));
}

} // namespace namer
}; // namespace ruby_typer
