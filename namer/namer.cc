#include "namer/namer.h"
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
        return ctx.state.enterClassSymbol(newOwner, constLit->cnst);
    }

    ast::Ident *arg2Ident(ast::Reference *arg) {
        while (true) {
            if (ast::Ident *id = dynamic_cast<ast::Ident *>(arg)) {
                return id;
            }
            typecase(arg,

                     [&](ast::RestArg *rest) { arg = rest->expr.get(); },
                     [&](ast::KeywordArg *kw) { arg = kw->expr.get(); },
                     [&](ast::OptionalArg *opt) { arg = opt->expr.get(); },
                     [&](ast::ShadowArg *opt) { arg = opt->expr.get(); });
        }
    }

    struct LocalFrame {
        bool is_block;
        unordered_map<ast::NameRef, ast::SymbolRef> locals;
    };

    vector<LocalFrame> scopeStack;

    unique_ptr<ast::Expression> addAncestor(ast::Context ctx, ast::ClassDef *klass, unique_ptr<ast::Statement> &node) {
        auto send = dynamic_cast<ast::Send *>(node.get());
        if (send == nullptr) {
            Error::check(node.get() != nullptr);
            return nullptr;
        }

        if (send->fun != ast::Names::include()) {
            return nullptr;
        }
        auto recv = dynamic_cast<ast::EmptyTree *>(send->recv.get());
        if (recv == nullptr) {
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
            remove_if(klass->rhs.begin(), klass->rhs.end(), [this, ctx, klass](unique_ptr<ast::Statement> &line) {
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

    void fillInArgs(ast::Context ctx, vector<unique_ptr<ast::Expression>> &args, ast::Symbol &symbol) {
        // Fill in the arity right with TODOs
        for (auto &arg : args) {
            if (auto *iarg = dynamic_cast<ast::Ident *>(arg.get())) {
                postTransformIdent(ctx.withOwner(symbol.ref(ctx)), iarg);
                symbol.argumentsOrMixins.push_back(iarg->symbol);
            } else {
                symbol.argumentsOrMixins.push_back(ast::GlobalState::defn_todo());
            }
        }
    }

    ast::MethodDef *preTransformMethodDef(ast::Context ctx, ast::MethodDef *method) {
        scopeStack.emplace_back();
        ast::SymbolRef owner = ownerFromContext(ctx);
        if (method->isSelf) {
            owner = owner.info(ctx).singletonClass(ctx);
        }

        method->symbol = ctx.state.enterMethodSymbol(owner, method->name);
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
            owner, ctx.state.freshNameUnique(ast::UniqueNameKind::Namer, ast::Names::blockTemp()));
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
            ast::Ident *id = arg2Ident(ref);
            if (id->symbol != ctx.state.defn_lvar_todo())
                continue;

            frame.locals.erase(id->name);
        }

        fillInArgs(ctx, blk->args, symbol);
        return blk;
    }

    ast::Block *postTransformBlock(ast::Context ctx, ast::Block *blk) {
        scopeStack.pop_back();
        return blk;
    }

    ast::Ident *postTransformIdent(ast::Context ctx, ast::Ident *ident) {
        if (ident->symbol == ctx.state.defn_lvar_todo()) {
            auto &frame = scopeStack.back();
            ast::SymbolRef &cur = frame.locals[ident->name];
            if (!cur.exists()) {
                cur = ctx.state.enterFieldSymbol(ctx.owner, ident->name);
                frame.locals[ident->name] = cur;
            }
            ident->symbol = cur;
        }
        return ident;
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
        ctx.state.enterStaticFieldSymbol(scope, lhs->cnst);

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

unique_ptr<ast::Statement> Namer::run(ast::Context &ctx, unique_ptr<ast::Statement> tree) {
    NameInserter nameInserter;
    tree = ast::TreeMap<NameInserter>::apply(ctx, nameInserter, move(tree));
    return Namer::resolve(ctx, move(tree));
}

} // namespace namer
}; // namespace ruby_typer
