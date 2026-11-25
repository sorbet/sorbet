# typed: true
# enable-experimental-rspec: true

module RSpec
  module Core
    class ExampleGroup
    end
  end
end

# `RSpec.shared_context` / `RSpec.shared_examples` defined inside a parameterized
# shared context (one with block params) should be registered as root-scoped
# constants and be resolvable via `include_context` / `include_examples`.
RSpec.shared_context 'outer parameterized context' do |setup_version: 'v1'|
  RSpec.shared_context 'nested context inside parameterized' do
    let(:value) { 42 }
  end

  RSpec.shared_examples 'nested examples inside parameterized' do
    it 'works' do
    end
  end

  RSpec.shared_examples_for 'nested examples_for inside parameterized' do
    it 'also works' do
    end
  end
end

RSpec.describe 'consumer' do
  include_context 'outer parameterized context'
  include_context 'nested context inside parameterized'
  include_examples 'nested examples inside parameterized'
  include_examples 'nested examples_for inside parameterized'
end
