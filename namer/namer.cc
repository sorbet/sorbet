#include "namer/namer.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"

#include <unordered_map>

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

    std::vector<std::unordered_map<ast::NameRef, ast::SymbolRef>> namesForLocals;

    unique_ptr<ast::Expression> addAncestor(ast::Context ctx, ast::ClassDef *klass, unique_ptr<ast::Statement> &node) {
        auto send = dynamic_cast<ast::Send *>(node.get());
        if (send == nullptr) {
            Error::check(node.get() != nullptr);
            return nullptr;
        }

        if (send->fun != ast::Names::include()) {
            return nullptr;
        }
        if (dynamic_cast<ast::EmptyTree *>(send->recv.get()) != nullptr) {
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
        // TODO check that send->block is empty
        return std::move(send->args[0]);
    }

public:
    ast::ClassDef *preTransformClassDef(ast::Context ctx, ast::ClassDef *klass) {
        klass->symbol = squashNames(ctx, ctx.owner, klass->name);
        namesForLocals.emplace_back();
        return klass;
    }

    ast::ClassDef *postTransformClassDef(ast::Context ctx, ast::ClassDef *klass) {
        namesForLocals.pop_back();
        klass->symbol = squashNames(ctx, ctx.owner, klass->name);
        for (auto it = klass->rhs.begin(); it != klass->rhs.end();) {
            auto &line = *it;
            auto newAncestor = addAncestor(ctx, klass, line);
            if (newAncestor) {
                klass->ancestors.emplace_back(std::move(newAncestor));
                klass->rhs.erase(it);
            } else {
                it++;
            }
        }
        return klass;
    }

    ast::MethodDef *preTransformMethodDef(ast::Context ctx, ast::MethodDef *method) {
        auto args = std::vector<ast::SymbolRef>();
        // Fill in the arity right with TODOs
        for (auto &UNUSED(_) : method->args) {
            args.push_back(ast::ContextBase::defn_todo());
        }

        auto result = ast::ContextBase::defn_todo();

        ctx.state.enterSymbol(ownerFromContext(ctx), method->name, result, args, true);
        namesForLocals.emplace_back();
        return method;
    }

    ast::MethodDef *postTransformMethodDef(ast::Context ctx, ast::MethodDef *method) {
        namesForLocals.pop_back();
        return method;
    }

    ast::Ident *postTransformIdent(ast::Context ctx, ast::Ident *ident) {
        if (ident->symbol.isPlaceHolder()) {
            ast::SymbolRef &cur = namesForLocals.back()[ident->name];
            if (!cur.exists()) {
                std::vector<ast::SymbolRef> args;
                cur = ctx.state.enterSymbol(ctx.owner, ident->name, ctx.state.defn_todo(), args, false);
            }
            ident->symbol = cur;
        }
        return ident;
    }

private:
    ast::SymbolRef ownerFromContext(ast::Context ctx) {
        auto owner = ctx.owner;
        if (owner == ast::ContextBase::defn_root()) {
            // Root methods end up going on object
            owner = ast::ContextBase::defn_object();
        }
        return owner;
    }

    NameInserter() {
        namesForLocals.emplace_back();
    }
};

unique_ptr<ast::Statement> Namer::run(ast::Context &ctx, unique_ptr<ast::Statement> tree) {
    NameInserter nameInserter;
    return ast::TreeMap<NameInserter>::apply(ctx, nameInserter, std::move(tree));
}

} // namespace namer
}; // namespace ruby_typer
