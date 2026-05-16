# typed: true
# enable-experimental-rspec: true

module RSpec
  module Core
    class ExampleGroup
    end
  end
end

# Coverage for review-suggested gaps in #10273:
#
#   1. Two-level bare nesting: a bare `shared_context` inside a parameterized outer,
#      with a bare `shared_examples` inside that. Both inner levels should be nested
#      (not root-scoped) so consumers that include the outer can reach them via the
#      ancestor chain, while consumers that don't get unresolved-constant errors.
#
#   2. `it_behaves_like 'bare nested name'`: the synthesized isolation class
#      inherits from the consumer's describe class, so the unrooted constant
#      reference still resolves through the consumer's ancestor chain.
#
#   3. Root-scope vs. bare-nested name collision: when an `RSpec.`-prefixed
#      shared_examples and a bare-nested one share a name, consumers that include
#      the outer pick up the nested one via ancestor lookup; consumers that don't
#      still find the root-scoped one through `Object`.

# (1) Two-level bare nesting.
RSpec.shared_context 'two-level outer' do |x|
  shared_context 'middle bare context' do
    shared_examples 'inner bare examples' do
      it 'works' do
      end
    end
  end
end

RSpec.describe 'two-level consumer that includes outer and middle' do
  include_context 'two-level outer'
  include_context 'middle bare context'
  include_examples 'inner bare examples'
end

RSpec.describe 'two-level consumer that skips the outer' do
  include_context 'middle bare context'
  #               ^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Unable to resolve constant `<shared_examples 'middle bare context'>`
  include_examples 'inner bare examples'
  #                ^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Unable to resolve constant `<shared_examples 'inner bare examples'>`
end

# (2) it_behaves_like resolving a bare-nested constant.
RSpec.shared_context 'it_behaves_like outer' do |x|
  shared_examples 'bare nested for it_behaves_like' do
    it 'works' do
    end
  end
end

RSpec.describe 'consumer using it_behaves_like with bare nested name' do
  include_context 'it_behaves_like outer'
  it_behaves_like 'bare nested for it_behaves_like'
end

RSpec.describe 'consumer using it_behaves_like without including the outer' do
  it_behaves_like 'bare nested for it_behaves_like'
  #               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Unable to resolve constant `<shared_examples 'bare nested for it_behaves_like'>`
end

# (3) Name collision: same name registered at root scope and bare-nested.
RSpec.shared_examples 'collision name' do
  it 'root-scoped version' do
  end
end

RSpec.shared_context 'collision outer' do |x|
  shared_examples 'collision name' do
    it 'bare-nested version' do
    end
  end
end

# Consumer that includes the outer: ancestor lookup walks through the outer's
# synthetic module first, finding the nested `collision name` before falling
# through to `Object`'s root-scoped one.
RSpec.describe 'collision consumer including outer' do
  include_context 'collision outer'
  include_examples 'collision name'
end

# Consumer that does NOT include the outer: ancestor lookup falls through to
# `Object` and finds the root-scoped version.
RSpec.describe 'collision consumer skipping outer' do
  include_examples 'collision name'
end

# (4) Two separate consumers including the same parameterized outer.
# At runtime each consumer's `include_context` re-runs the outer's block as
# `class_exec` on the consumer, registering the inner under the consumer's
# class. Sorbet collapses this into a single nested module under the outer's
# synthetic class; both consumers reach the inner via the include chain.
RSpec.shared_context 'shared outer for two consumers' do |x|
  shared_examples 'shared inner for two consumers' do
    it 'works' do
    end
  end
end

RSpec.describe 'first consumer of the shared outer' do
  include_context 'shared outer for two consumers'
  include_examples 'shared inner for two consumers'
end

RSpec.describe 'second consumer of the shared outer' do
  include_context 'shared outer for two consumers'
  include_examples 'shared inner for two consumers'
end

# (5) Transitive include: B's shared_context includes A's outer, and the
# consumer includes only B. The bare-nested constant declared in A should
# still resolve from the consumer's describe via the transitive ancestor walk
# (consumer -> B -> A).
RSpec.shared_context 'transitive A' do |x|
  shared_examples 'bare nested in transitive A' do
    it 'works' do
    end
  end
end

RSpec.shared_context 'transitive B' do
  include_context 'transitive A'
end

RSpec.describe 'transitive consumer including only B' do
  include_context 'transitive B'
  include_examples 'bare nested in transitive A'
end

# Negative companion: a consumer that skips the transitive chain entirely
# cannot reach A's bare-nested inner — the unrooted constant has no path
# through the consumer's ancestors.
RSpec.describe 'consumer skipping the transitive chain' do
  include_examples 'bare nested in transitive A'
  #                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Unable to resolve constant `<shared_examples 'bare nested in transitive A'>`
end

# (6) Bare shared_examples with block params nested in a parameterized outer.
# At runtime: `include_context 'outer with params for inner', :foo` re-runs the
# outer's block on the consumer with `outer_x=:foo`, registering the inner
# (which itself accepts a block param) against the consumer's class.
# `include_examples 'inner with params', :bar` then runs the inner's block on a
# nested isolated group with `inner_y=:bar`. The rewriter fake-test_each's
# both the outer and the inner so Sorbet sees the methods that get synthesized.
RSpec.shared_context 'outer with params for inner' do |outer_x|
  shared_examples 'inner with params' do |inner_y|
    it 'uses both' do
    end
  end
end

RSpec.describe 'consumer of parameterized inner' do
  include_context 'outer with params for inner'
  include_examples 'inner with params'
end

RSpec.describe 'consumer that skips the outer for parameterized inner' do
  include_examples 'inner with params'
  #                ^^^^^^^^^^^^^^^^^^^ error-with-dupes: Unable to resolve constant `<shared_examples 'inner with params'>`
end
