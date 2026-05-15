# typed: true
# enable-experimental-rspec: true

module RSpec
  module Core
    class ExampleGroup
    end
  end
end

# `RSpec.shared_examples` writes to RSpec's global registry, so two definitions
# with the same name collide at runtime regardless of where the call appears.
# The rewriter mirrors that by placing both at root scope; Sorbet merges them
# and surfaces the conflicting sigs as type errors on both bodies.

module A
  RSpec.shared_examples 'same name' do
    extend T::Sig

    sig { returns(String) }
    let(:value) { 'hello' } # error: Expected `Integer` but found `String("hello")` for method result type
  end
end

module B
  RSpec.shared_examples 'same name' do
    extend T::Sig

    sig { returns(Integer) }
    let(:value) { 'world' } # error: Expected `Integer` but found `String("world")` for method result type
  end
end

# Bare `shared_examples` inside `RSpec.describe` is nested under the describe,
# not registered globally. Two definitions with the same name in different
# describes live in different nested scopes and do not collide — matching
# RSpec runtime, where the registry context is the enclosing example group
# class, not the global `:main` context.
module C
  RSpec.describe 'group C' do
    extend T::Sig

    shared_examples 'independent name' do
      extend T::Sig

      sig { returns(String) }
      let(:value) { 'hello' }
    end
  end
end

module D
  RSpec.describe 'group D' do
    extend T::Sig

    shared_examples 'independent name' do
      extend T::Sig

      sig { returns(Integer) }
      let(:value) { 42 }
    end
  end
end
