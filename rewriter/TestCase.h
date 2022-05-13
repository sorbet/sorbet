#ifndef SORBET_REWRITER_TESTCASE_H
#define SORBET_REWRITER_TESTCASE_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars any test that includes invocations to `test "string" do ... end`
 *
 *    class MyTest < AnyParent
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
 *    class MyTest < AnyParent
 *      def initialize
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
 */
class TestCase final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *klass);

    TestCase() = delete;
};

} // namespace sorbet::rewriter

#endif
