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
#
# If a future change re-rooted bare definitions, the conflicting sigs below
# would merge and surface as type errors like the `module A`/`module B` case
# above. The `include_examples` consumers also pin that the bare-nested
# constant is reachable from within its own describe's ancestor walk — a
# scoping regression that narrowed the nesting too far would surface as an
# unresolved-constant error there.
module C
  RSpec.describe 'group C' do
    extend T::Sig

    shared_examples 'independent name' do
      extend T::Sig

      sig { returns(String) }
      let(:value) { 'hello' }
    end

    describe 'consumer of independent name in C' do
      include_examples 'independent name'
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

    describe 'consumer of independent name in D' do
      include_examples 'independent name'
    end
  end
end
