# typed: strict

class Test::T1 < PackageSpec
  test!

  strict_dependencies 'false'
  layer 'application'

  import Test::T2
end
