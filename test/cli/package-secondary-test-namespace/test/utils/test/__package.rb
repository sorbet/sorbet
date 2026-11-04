# frozen_string_literal: true

# typed: strict

class Test::Critic::Utils < PackageSpec
  test!

  import Critic::Utils, uses_internals: true
  import Critic
end
