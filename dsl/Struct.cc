#include "dsl/Struct.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names/dsl.h"
#include "core/core.h"
#include "dsl/dsl.h"

using namespace std;

namespace ruby_typer {
namespace dsl {

// TODO this isn't quite right since the scoping will change. This would
// really all be easier if we could run after class naming :/
unique_ptr<ast::Expression> dupName(ast::Expression *node) {
    auto empty = ast::cast_tree<ast::EmptyTree>(node);
    if (empty) {
        return ast::MK::EmptyTree(node->loc);
    }
    auto cnst = ast::cast_tree<ast::ConstantLit>(node);
    ENFORCE(cnst);
    auto newScope = dupName(cnst->scope.get());
    ENFORCE(newScope);
    return ast::MK::Constant(node->loc, move(newScope), cnst->cnst);
}

vector<unique_ptr<ast::Expression>> Struct::replaceDSL(core::Context ctx, ast::Assign *asgn) {
    vector<unique_ptr<ast::Expression>> empty;

    auto lhs = ast::cast_tree<ast::ConstantLit>(asgn->lhs.get());
    if (lhs == nullptr) {
        return empty;
    }

    auto send = ast::cast_tree<ast::Send>(asgn->rhs.get());
    if (send == nullptr) {
        return empty;
    }

    auto recv = ast::cast_tree<ast::ConstantLit>(send->recv.get());
    if (recv == nullptr) {
        return empty;
    }

    if (!ast::isa_tree<ast::EmptyTree>(recv->scope.get()) || recv->cnst != core::Symbols::Struct().data(ctx).name ||
        send->fun != core::Names::new_() || send->args.empty()) {
        return empty;
    }

    core::Loc loc = asgn->loc;

    ast::ClassDef::RHS_store body;
    ast::MethodDef::ARGS_store newArgs;
    ast::Hash::ENTRY_store sigKeys;
    ast::Hash::ENTRY_store sigValues;
    ast::Hash::ENTRY_store keys;
    ast::Hash::ENTRY_store values;

    for (auto &arg : send->args) {
        auto sym = ast::cast_tree<ast::SymbolLit>(arg.get());
        if (!sym) {
            return empty;
        }
        body.emplace_back(
            ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::attrAccessor(), ast::MK::Symbol(loc, sym->name)));
        auto key = ctx.state.enterNameUTF8("@" + sym->name.toString(ctx));
        keys.emplace_back(ast::MK::Symbol(loc, key));
        values.emplace_back(ast::MK::Ident(loc, core::Symbols::BasicObject()));
        sigKeys.emplace_back(ast::MK::Symbol(loc, sym->name));
        sigValues.emplace_back(ast::MK::Ident(loc, core::Symbols::BasicObject()));
        newArgs.emplace_back(make_unique<ast::OptionalArg>(loc, ast::MK::Local(loc, sym->name),
                                                           ast::MK::Ident(loc, core::Symbols::nil())));
    }
    body.emplace(body.begin(), ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::declareVariables(),
                                              make_unique<ast::Hash>(loc, move(keys), move(values))));

    body.emplace_back(ast::MK::Sig(loc, ast::MK::Hash(loc, move(sigKeys), move(sigValues)), dupName(asgn->lhs.get())));
    body.emplace_back(
        ast::MK::Method(loc, core::Names::new_(), move(newArgs), ast::MK::Cast(loc, dupName(asgn->lhs.get())), true));

    ast::ClassDef::ANCESTORS_store ancestors;
    ancestors.emplace_back(ast::MK::Ident(loc, core::Symbols::Struct()));

    vector<unique_ptr<ast::Expression>> stats;
    stats.emplace_back(make_unique<ast::ClassDef>(loc, core::Symbols::todo(), move(asgn->lhs), move(ancestors),
                                                  move(body), ast::ClassDefKind::Class));
    return stats;
}

} // namespace dsl
}; // namespace ruby_typer
