#include "rewriter/Regexp.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

vector<unique_ptr<ast::Expression>> Regexp::replaceDSL(core::MutableContext ctx, ast::Assign *asgn) {
    auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
    if (lhs == nullptr) {
        return {};
    }

    auto send = ast::cast_tree<ast::Send>(asgn->rhs.get());
    if (send == nullptr || send->fun != core::Names::new_()) {
        return {};
    }

    auto recv = ast::cast_tree<ast::ConstantLit>(send->recv.get());
    if (recv == nullptr || recv->symbol != core::Symbols::Regexp()) {
        return {};
    }

    vector<unique_ptr<ast::Expression>> stats;
    auto type = ast::MK::Constant(send->loc, core::Symbols::Regexp());
    auto newRhs = ast::MK::Let(send->loc, std::move(asgn->rhs), std::move(type));
    stats.emplace_back(ast::MK::Assign(asgn->loc, std::move(asgn->lhs), std::move(newRhs)));
    return stats;
}

}; // namespace sorbet::rewriter
