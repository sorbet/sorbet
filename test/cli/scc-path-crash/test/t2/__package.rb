# typed: strict

class Test::T2 < PackageSpec
  strict_dependencies 'layered'
  layer 'application'

  test_import Test::T1
end
