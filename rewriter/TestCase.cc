#include "rewriter/TestCase.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"

namespace sorbet::rewriter {
void TestCase::run(core::MutableContext ctx, ast::ClassDef *klass) {
    std::vector<ast::ExpressionPtr> stats;

    // Go through all class definition statements and find all setups
    std::vector<ast::ExpressionPtr> setups;
    bool isTest = false;
    for (auto &stat : klass->rhs) {
        if (auto *send = ast::cast_tree<ast::Send>(stat)) {
            if (send->fun == core::Names::test()) {
                if (send->numPosArgs() == 1 && !send->hasKwArgs() && send->hasBlock()) {
                    auto *arg0 = ast::cast_tree<ast::Literal>(send->getPosArg(0));

                    if (arg0 && arg0->isString(ctx)) {
                        isTest = true;
                        continue;
                    }
                }
            } else if (send->fun == core::Names::setup()) {
                if (send->hasBlock() && !send->hasPosArgs() && !send->hasKwArgs()) {
                    // send->args only contains block.
                    setups.push_back(std::move(stat));
                    continue;
                }
            }
        }
        stats.emplace_back(std::move(stat));
    }

    // If there are test definitions, then rewrite setup block invocations into initialize to avoid having the nilable
    // instance variables issue
    if (isTest) {
        for (auto &stat : setups) {
            auto *send = ast::cast_tree<ast::Send>(stat);
            auto loc = send->loc;
            auto block = send->block();

            auto method = ast::MK::SyntheticMethod0(loc, loc, core::Names::initialize(), std::move(block->body));
            auto method_with_sig =
                ast::MK::InsSeq1(method.loc(), ast::MK::SigVoid(method.loc(), {}), std::move(method));

            stats.emplace_back(std::move(method_with_sig));
        }
    } else {
        for (auto &stat : setups) {
            stats.emplace_back(std::move(stat));
        }
    }

    klass->rhs.clear();
    klass->rhs.reserve(stats.size());
    for (auto &stat : stats) {
        klass->rhs.emplace_back(std::move(stat));
    }
}

}; // namespace sorbet::rewriter
