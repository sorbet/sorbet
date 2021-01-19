#ifndef SORBET_REWRITER_TESTCASE_H
#define SORBET_REWRITER_TESTCASE_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *    class MyTest < ActiveSupport::TestCase
 *      test "the truth" do
 *          assert true
 *      end
 *    end
 *
 * into
 *
 *    class MyTest < ActiveSupport::TestCase
 *      def test_the_truth
 *        assert true
 *      end
 *    end
 */
class TestCase final {
public:
    static std::vector<ast::TreePtr> run(core::MutableContext ctx, ast::Send *send);

    TestCase() = delete;
};

} // namespace sorbet::rewriter

#endif
