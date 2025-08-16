# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::AppCyclePackage < PackageSpec
  layer 'app'
  strict_dependencies 'layered'

  import Foo::Bar::AppCyclePackage::SubPackage
  import Foo::MyPackage

  export Foo::Bar::AppCyclePackage::OtherClass
  export Foo::Bar::AppCyclePackage::ImportMeTestOnly
  export Test::Foo::Bar::AppCyclePackage::TestUtil
end