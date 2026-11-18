# typed: strict

class Test::Helper < PackageSpec
  test!
  layer 'test_helper'
  strict_dependencies 'false'

  import AppService
  import Test::Infra
end
