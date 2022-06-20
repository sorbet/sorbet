#include "rewriter/TestCase.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"

namespace sorbet::rewriter {
void TestCase::run(core::MutableContext ctx, ast::ClassDef *klass) {
    std::vector<ast::ExpressionPtr> stats;

    // Go through all class definition statements and find all setups, tests and teardowns
    std::vector<ast::ExpressionPtr> testSends, setupAndTeardownSends;
    for (auto &stat : klass->rhs) {
        if (auto *send = ast::cast_tree<ast::Send>(stat)) {
            if (send->fun == core::Names::test()) {
                if (send->numPosArgs() == 1 && !send->hasKwArgs() && send->hasBlock()) {
                    auto *arg0 = ast::cast_tree<ast::Literal>(send->getPosArg(0));

                    if (arg0 && arg0->isString()) {
                        testSends.push_back(std::move(stat));
                        continue;
                    }
                }
            } else if (send->fun == core::Names::setup() || send->fun == core::Names::teardown()) {
                if (send->hasBlock() && !send->hasPosArgs() && !send->hasKwArgs()) {
                    // send->args only contains block.
                    setupAndTeardownSends.push_back(std::move(stat));
                    continue;
                }
            }
        }
        stats.emplace_back(std::move(stat));
    }

    // Rewrite all test sends as method definitions
    for (auto &stat : testSends) {
        auto *send = ast::cast_tree<ast::Send>(stat);
        auto loc = send->loc;
        auto *arg0 = ast::cast_tree<ast::Literal>(send->getPosArg(0));
        auto *block = send->block();

        auto snake_case_name = absl::StrReplaceAll(arg0->asString().toString(ctx), {{" ", "_"}});
        auto name = ctx.state.enterNameUTF8("test_" + snake_case_name);
        auto method = ast::MK::SyntheticMethod0(loc, loc, name, std::move(block->body));
        auto method_with_sig = ast::MK::InsSeq1(method.loc(), ast::MK::SigVoid(method.loc(), {}), std::move(method));

        stats.emplace_back(std::move(method_with_sig));
    }

    // If there are test definitions, then we can probably assume that the setup and teardown invocations are safe to
    // rewrite as well. If there aren't any test sends, then emplace back the original statements into the class
    if (!testSends.empty()) {
        for (auto &stat : setupAndTeardownSends) {
            auto *send = ast::cast_tree<ast::Send>(stat);
            auto loc = send->loc;
            auto block = send->block();
            auto method_name = send->fun == core::Names::setup() ? core::Names::initialize() : core::Names::teardown();

            auto method = ast::MK::SyntheticMethod0(loc, loc, method_name, std::move(block->body));
            auto method_with_sig =
                ast::MK::InsSeq1(method.loc(), ast::MK::SigVoid(method.loc(), {}), std::move(method));

            stats.emplace_back(std::move(method_with_sig));
        }
    } else {
        for (auto &stat : setupAndTeardownSends) {
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
