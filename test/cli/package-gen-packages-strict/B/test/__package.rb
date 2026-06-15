# typed: strict

class Test::B < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import B
  import C
end
