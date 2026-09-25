# frozen_string_literal: true
# typed: strict

class Test::Foo::Bar::AppFalseCyclePackage < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import Foo::Bar::AppFalseCyclePackage
  export Test::Foo::Bar::AppFalseCyclePackage::TestUtil
end
