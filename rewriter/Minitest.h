#ifndef SORBET_REWRITER_MINITEST_H
#define SORBET_REWRITER_MINITEST_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *    class MyTest
 *      describe 'foo' do
 *        it 'bar' do
 *          baz
 *        end
 *      end
 *    end
 *
 * into
 *
 *    class MyTest
 *      class <class_foo> < self
 *        sig {void}
 *        def <test_bar>
 *          baz
 *        end
 *      end
 *    end
 *
 * which is sort of a lie in that each `test_` method should actually running in
 * its own instance where `.name` returns `test_0001_bar` but I think it is
 * close enough for our purposes.
 */
class Minitest final {
public:
    static std::vector<ast::ExpressionPtr> run(core::MutableContext ctx, bool isClass, ast::Send *send);

    // Class-level pass. Looks for `RSpec.configure do |config| ... end` blocks containing the
    // pairing `config.include_context 'NAME', :TAG` + `config.define_derived_metadata(...) do |m|
    // m[:TAG] = true end`. For each `NAME` so identified, finds the matching
    // `RSpec.shared_context 'NAME', :TAG do ... end` in the same file and clones its
    // direct-child bare `shared_examples`/`shared_context`/`shared_examples_for` calls as
    // top-level `RSpec.`-prefixed sends. The per-statement `run` pass then emits these as
    // root-scoped synthetic modules so consumers that pull the outer in via
    // `define_derived_metadata` (instead of an explicit lexical `include_context`) can still
    // resolve them.
    static void runOnClassDef(core::MutableContext ctx, ast::ClassDef *classDef);

    Minitest() = delete;
};

} // namespace sorbet::rewriter

#endif
