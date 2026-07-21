# frozen_string_literal: true

# typed: strict

class Minitest < PackageSpec
  test_import Critic

  export Minitest::Tests
end
