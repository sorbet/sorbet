# frozen_string_literal: true
# typed: strict

class Test::Foo::Bar::FalsePackage < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import Foo::Bar::FalsePackage
  export Test::Foo::Bar::FalsePackage::TestUtil
end
