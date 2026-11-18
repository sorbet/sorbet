# typed: strict

class Test::Infra < PackageSpec
  test!
  layer 'test_infra'
  strict_dependencies 'false'

  import BaseLib
  import Test::Unit
end
