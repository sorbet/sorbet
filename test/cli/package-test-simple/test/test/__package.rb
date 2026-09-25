# frozen_string_literal: true
# typed: strict

class Test::Critic < PackageSpec
  test!

  import Critic, uses_internals: true
end
