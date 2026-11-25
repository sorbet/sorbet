# typed: strict

class Test::D < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'dag'

  import D, uses_internals: true
end
