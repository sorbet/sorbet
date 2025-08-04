# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::FalseCyclePackageTest < PackageSpec
  layer 'lib'
  strict_dependencies 'false'

  import Foo::Bar::FalseCyclePackageTest::SubPackage
  import Foo::MyPackage

  export Foo::Bar::FalseCyclePackageTest::OtherClass
end
