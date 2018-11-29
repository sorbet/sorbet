#include "dsl/Struct.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "dsl/dsl.h"

using namespace std;

namespace sorbet::dsl {

// TODO this isn't quite right since the scoping will change. This would
// really all be easier if we could run after class naming :/
unique_ptr<ast::Expression> dupName(ast::Expression *node) {
    auto empty = ast::cast_tree<ast::EmptyTree>(node);
    if (empty) {
        return ast::MK::EmptyTree(node->loc);
    }
    auto cnst = ast::cast_tree<ast::UnresolvedConstantLit>(node);
    ENFORCE(cnst);
    auto newScope = dupName(cnst->scope.get());
    ENFORCE(newScope);
    return ast::MK::UnresolvedConstant(node->loc, std::move(newScope), cnst->cnst);
}

vector<unique_ptr<ast::Expression>> Struct::replaceDSL(core::MutableContext ctx, ast::Assign *asgn) {
    vector<unique_ptr<ast::Expression>> empty;

    auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
    if (lhs == nullptr) {
        return empty;
    }

    auto send = ast::cast_tree<ast::Send>(asgn->rhs.get());
    if (send == nullptr) {
        return empty;
    }

    auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv.get());
    if (recv == nullptr) {
        return empty;
    }

    if (!ast::isa_tree<ast::EmptyTree>(recv->scope.get()) || recv->cnst != core::Symbols::Struct().data(ctx)->name ||
        send->fun != core::Names::new_() || send->args.empty()) {
        return empty;
    }

    core::Loc loc = asgn->loc;

    ast::MethodDef::ARGS_store newArgs;
    ast::Hash::ENTRY_store sigKeys;
    ast::Hash::ENTRY_store sigValues;
    ast::ClassDef::RHS_store body;

    for (auto &arg : send->args) {
        auto sym = ast::cast_tree<ast::Literal>(arg.get());
        if (!sym || !sym->isSymbol(ctx)) {
            return empty;
        }
        core::NameRef name = sym->asSymbol(ctx);

        sigKeys.emplace_back(ast::MK::Symbol(loc, name));
        sigValues.emplace_back(ast::MK::Constant(loc, core::Symbols::BasicObject()));
        newArgs.emplace_back(make_unique<ast::OptionalArg>(loc, ast::MK::Local(loc, name), ast::MK::Nil(loc)));

        body.emplace_back(ast::MK::Method0(loc, loc, name, ast::MK::EmptyTree(loc), ast::MethodDef::DSLSynthesized));
        body.emplace_back(ast::MK::Method1(loc, loc, name.addEq(ctx), ast::MK::Local(loc, core::Names::arg0()),
                                           ast::MK::Local(loc, core::Names::arg0()), ast::MethodDef::DSLSynthesized));
    }

    body.emplace_back(
        ast::MK::Sig(loc, ast::MK::Hash(loc, std::move(sigKeys), std::move(sigValues)), dupName(asgn->lhs.get())));
    body.emplace_back(ast::MK::Method(loc, loc, core::Names::new_(), std::move(newArgs),
                                      ast::MK::Cast(loc, dupName(asgn->lhs.get())),
                                      ast::MethodDef::SelfMethod | ast::MethodDef::DSLSynthesized));

    ast::ClassDef::ANCESTORS_store ancestors;
    ancestors.emplace_back(ast::MK::UnresolvedConstant(loc, ast::MK::Constant(loc, core::Symbols::root()),
                                                       ctx.state.enterNameConstant(core::Names::Struct())));

    vector<unique_ptr<ast::Expression>> stats;
    stats.emplace_back(make_unique<ast::ClassDef>(loc, loc, core::Symbols::todo(), std::move(asgn->lhs),
                                                  std::move(ancestors), std::move(body), ast::ClassDefKind::Class));
    return stats;
}

}; // namespace sorbet::dsl
