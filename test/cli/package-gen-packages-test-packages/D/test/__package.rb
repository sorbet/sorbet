# typed: strict

class Test::D < PackageSpec
  test!

  strict_dependencies 'false'
  layer 'util'

  import D, uses_internals: true
end
