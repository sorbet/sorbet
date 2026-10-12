# typed: true
# enable-experimental-rspec: true

module RSpec
  module Core
    class ExampleGroup
    end
  end
end

# shared_examples/shared_context/shared_examples_for defined inside an
# RSpec.local_context block — bare or RSpec.-prefixed — should be hoisted to the
# top-level scope so consumers can resolve them via include_context /
# include_examples in any describe block. At runtime RSpec.local_context
# re-evaluates the block body in each consumer's example group, so any
# shared_examples-family call inside (regardless of receiver) ends up registered.
RSpec.local_context do
  RSpec.shared_context 'nested context inside local context' do
    let(:value) { 42 }
  end

  RSpec.shared_examples 'nested examples inside local context' do
    it 'works' do
    end
  end

  RSpec.shared_examples_for 'nested examples_for inside local context' do
    it 'also works' do
    end
  end

  # Bare-receiver forms should be hoisted alongside the RSpec.-prefixed ones.
  shared_context 'bare nested context inside local context' do
    let(:value) { 42 }
  end

  shared_examples 'bare nested examples inside local context' do
    it 'works' do
    end
  end

  shared_examples_for 'bare nested examples_for inside local context' do
    it 'also works' do
    end
  end
end

RSpec.describe 'consumer' do
  include_context 'nested context inside local context'
  include_examples 'nested examples inside local context'
  include_examples 'nested examples_for inside local context'

  include_context 'bare nested context inside local context'
  include_examples 'bare nested examples inside local context'
  include_examples 'bare nested examples_for inside local context'
end
