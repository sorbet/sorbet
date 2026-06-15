# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Test::Foo::MyPackage < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import Foo::MyPackage
  import Foo::Bar::FalsePackageTest
end
