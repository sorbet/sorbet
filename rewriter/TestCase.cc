#include "rewriter/TestCase.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"

namespace sorbet::rewriter {

std::vector<ast::TreePtr> TestCase::run(core::MutableContext ctx, ast::Send *send) {
    std::vector<ast::TreePtr> stats;
    if (ctx.state.runningUnderAutogen) {
        return stats;
    }

    std::cout << "Checking method name" << std::endl;
    if (send->fun != core::Names::test1()) {
        return stats;
    }

    std::cout << "Checking nunmber of args" << std::endl;
    if (send->args.size() != 1) {
        return stats;
    }

    std::cout << "Checking number of posArgs" << std::endl;
    if (send->numPosArgs != 1) {
        return stats;
    }

    std::cout << "Checking if block argument" << std::endl;
    if (!send->block) {
        return stats;
    }

    std::cout << "Checking if first arg is string" << std::endl;
    auto *arg0 = ast::cast_tree<ast::Literal>(send->args[0]);
    if (!arg0 || !arg0->isString(ctx)) {
        return stats;
    }

    auto name = arg0->asString(ctx);
    // Generate sigs?
    //
    std::cout << "Generate method" << std::endl;
    auto loc = send->loc;
    auto block = ast::cast_tree<ast::Block>(send->block);
    auto method = ast::MK::SyntheticMethod0(loc, loc, name, std::move(block->body));
    stats.emplace_back(std::move(method));

    return stats;
}

}; // namespace sorbet::rewriter