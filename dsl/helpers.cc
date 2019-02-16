#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/core.h"
#include "dsl/ChalkODMProp.h"
#include "dsl/helpers.h"
#include "dsl/util.h"

using namespace std;

namespace sorbet::dsl {

unique_ptr<ast::Expression> mkGet(core::Loc loc, core::NameRef name, unique_ptr<ast::Expression> rhs) {
    return ast::MK::Method0(loc, loc, name, move(rhs), ast::MethodDef::DSLSynthesized);
}

unique_ptr<ast::Expression> mkSet(core::Loc loc, core::NameRef name, core::Loc argLoc,
                                  unique_ptr<ast::Expression> rhs) {
    return ast::MK::Method1(loc, loc, name, ast::MK::Local(argLoc, core::Names::arg0()), move(rhs),
                            ast::MethodDef::DSLSynthesized);
}

unique_ptr<ast::Expression> mkNilable(core::Loc loc, unique_ptr<ast::Expression> type) {
    return ast::MK::Send1(loc, ast::MK::T(loc), core::Names::nilable(), move(type));
}

unique_ptr<ast::Expression> mkMutator(core::MutableContext ctx, core::Loc loc, core::NameRef className) {
    auto chalk = ast::MK::UnresolvedConstant(loc, ast::MK::Constant(loc, core::Symbols::root()),
                                             ctx.state.enterNameConstant(core::Names::Chalk()));
    auto odm = ast::MK::UnresolvedConstant(loc, move(chalk), ctx.state.enterNameConstant(core::Names::ODM()));
    auto mutator = ast::MK::UnresolvedConstant(loc, move(odm), ctx.state.enterNameConstant(core::Names::Mutator()));
    auto private_ =
        ast::MK::UnresolvedConstant(loc, move(mutator), ctx.state.enterNameConstant(core::Names::Private()));
    return ast::MK::UnresolvedConstant(loc, move(private_), ctx.state.enterNameConstant(className));
}

unique_ptr<ast::Expression> thunkBody(core::MutableContext ctx, ast::Expression *node) {
    auto send = ast::cast_tree<ast::Send>(node);
    if (send == nullptr) {
        return nullptr;
    }
    if (send->fun != core::Names::lambda() && send->fun != core::Names::proc()) {
        return nullptr;
    }
    if (!ast::isa_tree<ast::Self>(send->recv.get())) {
        return nullptr;
    }
    if (send->block == nullptr) {
        return nullptr;
    }
    if (!send->block->args.empty()) {
        return nullptr;
    }
    return move(send->block->body);
}

bool isProbablySymbol(core::MutableContext ctx, ast::Expression *type, core::SymbolRef sym) {
    auto cnst = ast::cast_tree<ast::UnresolvedConstantLit>(type);
    if (cnst) {
        if (cnst->cnst != sym.data(ctx)->name) {
            return false;
        }
        if (ast::isa_tree<ast::EmptyTree>(cnst->scope.get())) {
            return true;
        }

        auto scope_cnst = ast::cast_tree<ast::UnresolvedConstantLit>(cnst->scope.get());
        if (scope_cnst && ast::isa_tree<ast::EmptyTree>(scope_cnst->scope.get()) &&
            scope_cnst->cnst == core::Symbols::T().data(ctx)->name) {
            return true;
        }

        auto scope_cnst_lit = ast::cast_tree<ast::ConstantLit>(cnst->scope.get());
        if (scope_cnst_lit && scope_cnst_lit->constantSymbol() == core::Symbols::root()) {
            return true;
        }
        return false;
    }

    auto send = ast::cast_tree<ast::Send>(type);
    if (send && send->fun == core::Names::squareBrackets() && isProbablySymbol(ctx, send->recv.get(), sym)) {
        return true;
    }

    return false;
}

} // namespace sorbet::dsl
