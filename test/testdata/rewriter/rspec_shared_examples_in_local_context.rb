# typed: true
# enable-experimental-rspec: true

module RSpec
  module Core
    class ExampleGroup
    end
  end
end

# RSpec.shared_examples/shared_context/shared_examples_for defined inside a
# RSpec.local_context block should be registered as root-scoped constants and
# be resolvable via include_context / include_examples in any describe block.
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
end

RSpec.describe 'consumer' do
  include_context 'nested context inside local context'
  include_examples 'nested examples inside local context'
  include_examples 'nested examples_for inside local context'
end
