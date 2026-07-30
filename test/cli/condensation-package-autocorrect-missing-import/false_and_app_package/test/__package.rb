# frozen_string_literal: true
# typed: strict

class Test::Foo::Bar::FalseAndAppPackage < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import Foo::Bar::FalseAndAppPackage
  export Test::Foo::Bar::FalseAndAppPackage::TestUtil
end
