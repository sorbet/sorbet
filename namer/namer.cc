#include "namer/namer.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"

namespace ruby_typer {
namespace namer {

/**
 * Used with TreeMap to return you a SymbolRef collapsing all the `Foo::Bar::Baz`.
 */
class NamespaceCollapeser {
public:
    NamespaceCollapeser(ast::SymbolRef name) {
        this->name = name;
    }

    ast::SymbolRef getName() {
        return name;
    }

    ast::ConstantLit *postTransformConstantLit(ast::Context ctx, ast::ConstantLit *c) {
        this->name = ctx.state.enterClassSymbol(getName(), c->cnst);
        return c;
    }

private:
    ast::SymbolRef name;
};

/**
 * Used with TreeMap to insert all the class and method symbols into the symbol
 * table.
 */
class NameInserter {
public:
    ast::ClassDef *preTransformClassDef(ast::Context ctx, ast::ClassDef *klass) {
        NamespaceCollapeser nc(ctx.owner);
        auto newName = ast::TreeMap<NamespaceCollapeser>::apply(ctx, nc, std::move(klass->name));
        klass->name = ast::desugar::stat2Expr(newName);
        klass->symbol = nc.getName();
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
        return method;
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
};

unique_ptr<ast::Statement> Namer::run(ast::Context &ctx, unique_ptr<ast::Statement> tree) {
    NameInserter nameInserter;
    return ast::TreeMap<NameInserter>::apply(ctx, nameInserter, std::move(tree));
}

} // namespace namer
}; // namespace ruby_typer
