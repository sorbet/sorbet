#include "rewriter/TestCase.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"

namespace sorbet::rewriter {

std::vector<ast::TreePtr> TestCase::run(core::MutableContext ctx, ast::Send *send) {
    std::vector<ast::TreePtr> stats;
    if (ctx.state.runningUnderAutogen) {
        return stats;
    }

    if (send->fun != core::Names::test()) {
        return stats;
    }

    if (send->args.size() != 1) {
        return stats;
    }

    if (send->numPosArgs != 1) {
        return stats;
    }

    if (!send->block) {
        return stats;
    }

    auto *arg0 = ast::cast_tree<ast::Literal>(send->args[0]);
    if (!arg0 || !arg0->isString(ctx)) {
        return stats;
    }

    // auto formatted_name = fmt::format("test_{0}", arg0->asString(ctx).toString(ctx));
    // auto name = arg0->asString(ctx).data(ctx);
    // std::cout << "name: " << name->toString(ctx) << std::endl;

    auto snake_case_name = absl::StrReplaceAll(arg0->asString(ctx).toString(ctx), {{" ", "_"}});
    auto name = ctx.state.enterNameUTF8("test_" + snake_case_name);

    // Generate sigs?
    //
    auto loc = send->loc;
    auto block = ast::cast_tree<ast::Block>(send->block);
    auto method = ast::MK::SyntheticMethod0(loc, loc, name, std::move(block->body));
    stats.emplace_back(std::move(method));

    return stats;
}

}; // namespace sorbet::rewriter
