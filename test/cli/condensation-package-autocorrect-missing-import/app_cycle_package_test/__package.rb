# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::AppCyclePackageTest < PackageSpec
  layer 'app'
  strict_dependencies 'layered'

  import Foo::Bar::AppCyclePackageTest::SubPackage
  import Foo::MyPackage

  export Foo::Bar::AppCyclePackageTest::OtherClass
end
