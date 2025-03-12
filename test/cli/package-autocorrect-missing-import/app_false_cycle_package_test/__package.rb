# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::AppFalseCyclePackageTest < PackageSpec
  layer 'app'
  strict_dependencies 'false'

  import Foo::Bar::AppFalseCyclePackageTest::SubPackage
  import Foo::MyPackage

  export Foo::Bar::AppFalseCyclePackageTest::OtherClass
end
