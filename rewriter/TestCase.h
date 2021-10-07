#ifndef SORBET_REWRITER_TESTCASE_H
#define SORBET_REWRITER_TESTCASE_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *    class MyTest < ActiveSupport::TestCase
 *      setup do
 *        @a = 1
 *      end
 *
 *      test "the equality" do
 *        assert_equal 1, @a
 *      end
 *
 *      teardown do
 *        @a = nil
 *      end
 *    end
 *
 * into
 *
 *    class MyTest < ActiveSupport::TestCase
 *      def setup
 *        @a = 1
 *      end
 *
 *      def test_the_equality
 *        assert_equal 1, @a
 *      end
 *
 *      def teardown
 *        @a = nil
 *      end
 *    end
 */
class TestCase final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *klass);

    TestCase() = delete;
};

} // namespace sorbet::rewriter

#endif
