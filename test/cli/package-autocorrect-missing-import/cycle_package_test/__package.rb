# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::CyclePackageTest < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'

  import Foo::Bar::CyclePackageTest::SubPackage
  import Foo::MyPackage

  export Foo::Bar::CyclePackageTest::OtherClass
end
