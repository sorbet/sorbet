# frozen_string_literal: true
# typed: strict

class Test::Business2 < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import Business2
  import Service1
end
