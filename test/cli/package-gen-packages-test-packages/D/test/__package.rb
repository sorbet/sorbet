# typed: strict

class Test::D < PackageSpec
  test!

  strict_dependencies 'dag'
  layer 'util'

  import D, uses_internals: true
end
