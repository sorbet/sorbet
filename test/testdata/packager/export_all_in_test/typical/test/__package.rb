# frozen_string_literal: true
# typed: strict

class Test::Typical < PackageSpec
  test!
  import Typical, uses_internals: true
  export Test::Typical::Example
end
