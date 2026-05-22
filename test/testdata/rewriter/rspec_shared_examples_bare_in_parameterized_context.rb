# typed: true
# enable-experimental-rspec: true

module RSpec
  module Core
    class ExampleGroup
    end
  end
end

# Bare `shared_examples` / `shared_context` / `shared_examples_for` nested inside a
# parameterized `RSpec.shared_context` / `RSpec.shared_examples` at runtime register
# the example group into the *consumer's* example group on each include of the outer
# (RSpec re-runs the outer's block per include). The rewriter mirrors that without
# polluting Sorbet's root scope: the inner module is synthesized nested under the
# outer's synthetic module, so it's reachable only through the consumer's ancestor
# chain — consumers that include the outer find it, others get a resolver error
# (matching RSpec's "example group not found" runtime failure).

RSpec.shared_context 'outer parameterized context' do |setup_version: 'v1'|
  shared_context 'bare nested context inside parameterized' do
    let(:value) { 42 }
  end

  shared_examples 'bare nested examples inside parameterized' do
    it 'works' do
    end
  end

  shared_examples_for 'bare nested examples_for inside parameterized' do
    it 'also works' do
    end
  end
end

# Consumer that includes the outer: all three nested constants resolve via the
# include chain (ancestor lookup walks through the outer's synthetic module).
RSpec.describe 'consumer that includes the outer' do
  include_context 'outer parameterized context'
  include_context 'bare nested context inside parameterized'
  include_examples 'bare nested examples inside parameterized'
  include_examples 'bare nested examples_for inside parameterized'
end

# Consumer that does NOT include the outer: each nested-only constant fails to
# resolve. This matches RSpec's runtime behavior — without the outer in scope,
# the registry has no entry for these names.
RSpec.describe 'consumer that does not include the outer' do
  include_context 'bare nested context inside parameterized'
  #               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Unable to resolve constant `<shared_examples 'bare nested context inside parameterized'>`
  include_examples 'bare nested examples inside parameterized'
  #                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Unable to resolve constant `<shared_examples 'bare nested examples inside parameterized'>`
  include_examples 'bare nested examples_for inside parameterized'
  #                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Unable to resolve constant `<shared_examples 'bare nested examples_for inside parameterized'>`
end
