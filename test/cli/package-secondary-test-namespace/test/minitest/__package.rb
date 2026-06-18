# frozen_string_literal: true

# typed: strict

class Minitest < PackageSpec
  test!

  import Critic

  export Minitest::Tests
end
