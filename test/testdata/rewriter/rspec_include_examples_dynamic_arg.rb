# typed: true
# enable-experimental-rspec: true
# enable-experimental-requires-ancestor: true

module RSpec
  module Core
    class ExampleGroup
    end
  end
end

# When include_examples / include_context is called with a non-literal argument
# (e.g. a block parameter passed through from a parameterized shared_examples),
# the rewriter must NOT attempt to rewrite it to `include(ConstantName)` because
# the name cannot be resolved at compile time.  Previously this produced a
# spurious "Unable to resolve constant <shared_examples 'foo'>" error.

RSpec.shared_examples 'suite alpha' do
  it 'passes' do
  end
end

RSpec.shared_examples 'suite beta' do
  it 'also passes' do
  end
end

# Parameterized shared_examples that accepts the name of another shared_examples
# as a block argument and forwards it via include_examples.
RSpec.shared_examples 'runner' do |suite_name|
  include_examples suite_name

  context 'nested' do
    include_examples suite_name
  end
end

RSpec.describe 'consumer' do
  # Literal argument — still works as before.
  include_examples 'suite alpha'
  include_examples 'suite beta'
end

# Dynamic arg in a non-parameterized describe block (exercises the runSingle path).
# The call should be silently dropped rather than producing a spurious error.
def suite_helper
  'suite alpha'
end

RSpec.describe 'top-level dynamic' do
  include_examples suite_helper
end
