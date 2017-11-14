#include "namer/namer.h"
#include "../ast/ast.h"
#include "../core/Context.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
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
        auto constLit = dynamic_cast<ast::ConstantLit *>(node.get());
        if (constLit == nullptr) {
            Error::check(node.get() != nullptr);
            return owner;
        }

        auto newOwner = squashNames(ctx, owner, constLit->scope);
        return ctx.state.enterClassSymbol(constLit->loc, newOwner, constLit->cnst);
    }

    ast::UnresolvedIdent *arg2NameRef(ast::Reference *arg) {
        while (true) {
            if (ast::UnresolvedIdent *nm = dynamic_cast<ast::UnresolvedIdent *>(arg)) {
                return nm;
            }
            typecase(arg, [&](ast::RestArg *rest) { arg = rest->expr.get(); },
                     [&](ast::KeywordArg *kw) { arg = kw->expr.get(); },
                     [&](ast::OptionalArg *opt) { arg = opt->expr.get(); },
                     [&](ast::ShadowArg *opt) { arg = opt->expr.get(); },
                     [&](ast::BlockArg *opt) { arg = opt->expr.get(); });
        }
    }

    struct LocalFrame {
        unordered_map<core::NameRef, core::LocalVariable> locals;
    };

    vector<LocalFrame> scopeStack;

    unique_ptr<ast::Expression> addAncestor(core::Context ctx, ast::ClassDef *klass,
                                            unique_ptr<ast::Expression> &node) {
        auto send = dynamic_cast<ast::Send *>(node.get());
        if (send == nullptr) {
            Error::check(node.get() != nullptr);
            return nullptr;
        }

        if (send->fun != core::Names::include()) {
            return nullptr;
        }
        if (dynamic_cast<ast::Self *>(send->recv.get()) == nullptr) {
            // ignore `something.include`
            return nullptr;
        }

        if (send->args.size() != 1) {
            ctx.state.errors.error(send->loc, core::ErrorClass::IncludeMutipleParam,
                                   "`include` should only be passed a single constant. You passed {} parameters.",
                                   send->args.size());
            return nullptr;
        }
        auto constLit = dynamic_cast<ast::ConstantLit *>(send->args[0].get());
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

    unique_ptr<ast::Expression> fillInArg(core::Context ctx, ast::UnresolvedIdent *nmarg, core::Symbol &method) {
        core::SymbolRef owner = method.ref(ctx);
        auto tx = postTransformUnresolvedIdent(ctx.withOwner(owner), nmarg);
        if (tx != nmarg) {
            unique_ptr<ast::Expression> res(tx);
            ast::Local *id = dynamic_cast<ast::Local *>(tx);
            Error::check(id != nullptr);
            method.argumentsOrMixins.push_back(ctx.state.enterMethodArgumentSymbol(nmarg->loc, owner, nmarg->name));
            return res;
        }
        Error::notImplemented();
    }

    void fillInArgs(core::Context ctx, ast::MethodDef::ARGS_store &args, core::Symbol &symbol) {
        // Fill in the arity right with TODOs
        symbol.argumentsOrMixins.reserve(args.size());
        for (auto &arg : args) {
            ast::Reference *ref = dynamic_cast<ast::Reference *>(arg.get());
            Error::check(ref != nullptr);
            arg = fillInArg(ctx, arg2NameRef(ref), symbol);
        }
    }

    ast::MethodDef *preTransformMethodDef(core::Context ctx, ast::MethodDef *method) {
        scopeStack.emplace_back();
        core::SymbolRef owner = ownerFromContext(ctx);
        if (method->isSelf) {
            owner = owner.info(ctx).singletonClass(ctx);
        }

        method->symbol = ctx.state.enterMethodSymbol(method->loc, owner, method->name);
        core::Symbol &symbol = method->symbol.info(ctx);
        fillInArgs(ctx, method->args, symbol);
        symbol.definitionLoc = method->loc;

        return method;
    }

    ast::MethodDef *postTransformMethodDef(core::Context ctx, ast::MethodDef *method) {
        scopeStack.pop_back();
        return method;
    }

    ast::Block *preTransformBlock(core::Context ctx, ast::Block *blk) {
        core::SymbolRef owner = ownerFromContext(ctx);
        blk->symbol = ctx.state.enterMethodSymbol(
            blk->loc, owner, ctx.state.freshNameUnique(core::UniqueNameKind::Namer, core::Names::blockTemp()));
        core::Symbol &symbol = blk->symbol.info(ctx);

        scopeStack.emplace_back();
        auto &parent = *(scopeStack.end() - 2);
        auto &frame = scopeStack.back();

        // We inherit our parent's locals
        for (auto &binding : parent.locals) {
            frame.locals.insert(binding);
        }
        // Except that any arguments we have exist in our own frame
        for (auto &arg : blk->args) {
            ast::Reference *ref = dynamic_cast<ast::Reference *>(arg.get());
            if (ref == nullptr)
                continue;
            ast::UnresolvedIdent *nm = arg2NameRef(ref);
            if (nm->kind != ast::UnresolvedIdent::Local)
                continue;

            frame.locals[nm->name] = ctx.state.newTemporary(core::UniqueNameKind::Namer, nm->name, blk->symbol);
        }

        fillInArgs(ctx, blk->args, symbol);
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
        ast::ConstantLit *lhs = dynamic_cast<ast::ConstantLit *>(asgn->lhs.get());
        if (lhs == nullptr)
            return asgn;

        // TODO(nelhage): forbid dynamic constant definition
        core::SymbolRef scope = squashNames(ctx, ctx.owner, lhs->scope);
        core::SymbolRef cnst = ctx.state.enterStaticFieldSymbol(lhs->loc, scope, lhs->cnst);
        cnst.info(ctx).resultType = core::Types::dynamic();

        return asgn;
    }

private:
    core::SymbolRef ownerFromContext(core::Context ctx) {
        core::SymbolRef owner = ctx.owner;
        if (owner == core::GlobalState::defn_root()) {
            // Root methods end up going on object
            owner = core::GlobalState::defn_object();
        }
        return owner;
    }

    NameInserter() {
        scopeStack.emplace_back();
    }
};

unique_ptr<ast::Expression> Namer::run(core::Context &ctx, unique_ptr<ast::Expression> tree) {
    NameInserter nameInserter;
    tree = ast::TreeMap<NameInserter>::apply(ctx, nameInserter, move(tree));
    return Namer::resolve(ctx, move(tree));
}

} // namespace namer
}; // namespace ruby_typer
