# typed: strict

class Test::Unit < PackageSpec
  test!
  layer 'test_unit'
  strict_dependencies 'false'

  import Test::Helper
end
