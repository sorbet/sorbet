# frozen_string_literal: true

# typed: strict

class Critic::Utils < PackageSpec
  test_import Critic

  export Critic::Utils::UnitTest
end
