# frozen_string_literal: true
# typed: strict

class Test::Foo::Bar::CyclePackage < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import Foo::Bar::CyclePackage
  export Test::Foo::Bar::CyclePackage::TestUtil
end
