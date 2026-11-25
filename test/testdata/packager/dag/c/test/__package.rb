# frozen_string_literal: true
# typed: strict

class Test::C < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import C
  import B
end
