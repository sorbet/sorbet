# typed: true
# enable-experimental-rspec: true

module RSpec
  module Core
    class ExampleGroup
    end
  end
end

# Because shared_examples constants are placed at root scope (matching RSpec's
# global registry), two shared_examples with the same name in different modules
# map to the same constant and their definitions are merged.
#
# This mirrors RSpec's own behavior: redefining a shared example with an existing
# name produces a warning in RSpec, and the convention is that names are globally
# unique. Sorbet surfaces the conflict as a type error when the merged definitions
# are incompatible.

module A
  RSpec.describe 'group A' do
    extend T::Sig

    shared_examples 'same name' do
      extend T::Sig

      sig { returns(String) }
      let(:value) { 'hello' } # error: Expected `Integer` but found `String("hello")` for method result type
    end
  end
end

module B
  RSpec.describe 'group B' do
    extend T::Sig

    # This shared_examples has the same name as the one in module A.
    # Both definitions are merged into ::<shared_examples 'same name'>,
    # and the conflicting sigs produce an error on both bodies.
    shared_examples 'same name' do
      extend T::Sig

      sig { returns(Integer) }
      let(:value) { 'world' } # error: Expected `Integer` but found `String("world")` for method result type
    end
  end
end
