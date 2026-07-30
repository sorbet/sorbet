# frozen_string_literal: true
# typed: strict

class Test::Foo::Bar::FalseCyclePackage < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import Foo::Bar::FalseCyclePackage
  export Test::Foo::Bar::FalseCyclePackage::TestUtil
end
