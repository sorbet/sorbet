# typed: strict

class Test::T2 < PackageSpec
  test!

  strict_dependencies 'false'
  layer 'application'

  test_import Test::T1
end
