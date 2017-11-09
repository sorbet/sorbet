#include "namer/namer.h"
#include "../ast/ast.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"

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
    ast::SymbolRef squashNames(ast::Context ctx, ast::SymbolRef owner, unique_ptr<ast::Expression> &node) {
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
        bool is_block;
        unordered_map<ast::NameRef, ast::SymbolRef> locals;
    };

    vector<LocalFrame> scopeStack;

    unique_ptr<ast::Expression> addAncestor(ast::Context ctx, ast::ClassDef *klass, unique_ptr<ast::Expression> &node) {
        auto send = dynamic_cast<ast::Send *>(node.get());
        if (send == nullptr) {
            Error::check(node.get() != nullptr);
            return nullptr;
        }

        if (send->fun != ast::Names::include()) {
            return nullptr;
        }
        if (dynamic_cast<ast::Self *>(send->recv.get()) == nullptr) {
            // ignore `something.include`
            return nullptr;
        }

        if (send->args.size() != 1) {
            ctx.state.errors.error(send->loc, ast::ErrorClass::IncludeMutipleParam,
                                   "`include` should only be passed a single constant. You passed {} parameters.",
                                   send->args.size());
            return nullptr;
        }
        auto constLit = dynamic_cast<ast::ConstantLit *>(send->args[0].get());
        if (constLit == nullptr) {
            ctx.state.errors.error(send->loc, ast::ErrorClass::IncludeNotConstant,
                                   "`include` must be passed a constant literal. You passed {}.",
                                   send->args[0]->toString(ctx));
            return nullptr;
        }
        if (send->block != nullptr) {
            ctx.state.errors.error(send->loc, ast::ErrorClass::IncludePassedBlock,
                                   "`include` can not be passed a block.");
            return nullptr;
        }
        // TODO check that send->block is empty
        return move(send->args[0]);
    }

public:
    ast::ClassDef *preTransformClassDef(ast::Context ctx, ast::ClassDef *klass) {
        klass->symbol = squashNames(ctx, ctx.owner, klass->name);
        scopeStack.emplace_back();
        return klass;
    }

    ast::ClassDef *postTransformClassDef(ast::Context ctx, ast::ClassDef *klass) {
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

    unique_ptr<ast::Expression> fillInArg(ast::Context ctx, ast::UnresolvedIdent *nmarg, ast::Symbol &method) {
        auto tx = postTransformUnresolvedIdent(ctx.withOwner(method.ref(ctx)), nmarg);
        if (tx != nmarg) {
            unique_ptr<ast::Expression> res(tx);
            ast::Ident *id = dynamic_cast<ast::Ident *>(tx);
            Error::check(id != nullptr);
            method.argumentsOrMixins.push_back(id->symbol);
            return res;
        }
        Error::notImplemented();
    }

    void fillInArgs(ast::Context ctx, vector<unique_ptr<ast::Expression>> &args, ast::Symbol &symbol) {
        // Fill in the arity right with TODOs
        for (auto &arg : args) {
            ast::Reference *ref = dynamic_cast<ast::Reference *>(arg.get());
            Error::check(ref != nullptr);
            arg = fillInArg(ctx, arg2NameRef(ref), symbol);
        }
    }

    ast::MethodDef *preTransformMethodDef(ast::Context ctx, ast::MethodDef *method) {
        scopeStack.emplace_back();
        ast::SymbolRef owner = ownerFromContext(ctx);
        if (method->isSelf) {
            owner = owner.info(ctx).singletonClass(ctx);
        }

        method->symbol = ctx.state.enterMethodSymbol(method->loc, owner, method->name);
        ast::Symbol &symbol = method->symbol.info(ctx);
        fillInArgs(ctx, method->args, symbol);
        symbol.definitionLoc = method->loc;

        return method;
    }

    ast::MethodDef *postTransformMethodDef(ast::Context ctx, ast::MethodDef *method) {
        scopeStack.pop_back();
        return method;
    }

    ast::Block *preTransformBlock(ast::Context ctx, ast::Block *blk) {
        ast::SymbolRef owner = ownerFromContext(ctx);
        blk->symbol = ctx.state.enterMethodSymbol(
            blk->loc, owner, ctx.state.freshNameUnique(ast::UniqueNameKind::Namer, ast::Names::blockTemp()));
        ast::Symbol &symbol = blk->symbol.info(ctx);

        scopeStack.emplace_back();
        auto &parent = *(scopeStack.end() - 2);
        auto &frame = scopeStack.back();
        frame.is_block = true;

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

            frame.locals.erase(nm->name);
        }

        fillInArgs(ctx, blk->args, symbol);
        return blk;
    }

    ast::Block *postTransformBlock(ast::Context ctx, ast::Block *blk) {
        scopeStack.pop_back();
        return blk;
    }

    ast::Expression *postTransformUnresolvedIdent(ast::Context ctx, ast::UnresolvedIdent *nm) {
        switch (nm->kind) {
            case ast::UnresolvedIdent::Local: {
                auto &frame = scopeStack.back();
                ast::SymbolRef &cur = frame.locals[nm->name];
                if (!cur.exists()) {
                    cur = ctx.state.enterLocalSymbol(ctx.owner, nm->name);
                    frame.locals[nm->name] = cur;
                }
                return new ast::Ident(nm->loc, cur);
            }
            case ast::UnresolvedIdent::Global: {
                ast::Symbol &root = ctx.state.defn_root().info(ctx);
                ast::SymbolRef sym = root.findMember(nm->name);
                if (!sym.exists()) {
                    sym = ctx.state.enterFieldSymbol(nm->loc, ctx.state.defn_root(), nm->name);
                }
                return new ast::Ident(nm->loc, sym);
            }
            default:
                return nm;
        }
    }

    ast::Self *postTransformSelf(ast::Context ctx, ast::Self *self) {
        self->claz = ctx.selfClass();
        return self;
    }

    ast::Assign *postTransformAssign(ast::Context ctx, ast::Assign *asgn) {
        ast::ConstantLit *lhs = dynamic_cast<ast::ConstantLit *>(asgn->lhs.get());
        if (lhs == nullptr)
            return asgn;

        // TODO(nelhage): forbid dynamic constant definition
        ast::SymbolRef scope = squashNames(ctx, ctx.owner, lhs->scope);
        ast::SymbolRef cnst = ctx.state.enterStaticFieldSymbol(lhs->loc, scope, lhs->cnst);
        cnst.info(ctx).resultType = ast::Types::dynamic();

        return asgn;
    }

private:
    ast::SymbolRef ownerFromContext(ast::Context ctx) {
        ast::SymbolRef owner = ctx.owner;
        if (owner == ast::GlobalState::defn_root()) {
            // Root methods end up going on object
            owner = ast::GlobalState::defn_object();
        }
        return owner;
    }

    NameInserter() {
        scopeStack.emplace_back();
    }
};

unique_ptr<ast::Expression> Namer::run(ast::Context &ctx, unique_ptr<ast::Expression> tree) {
    NameInserter nameInserter;
    tree = ast::TreeMap<NameInserter>::apply(ctx, nameInserter, move(tree));
    return Namer::resolve(ctx, move(tree));
}

} // namespace namer
}; // namespace ruby_typer
