# typed: strict

class Test::T2 < PackageSpec
  test!

  strict_dependencies 'layered'
  layer 'application'

  test_import Test::T1
end
