# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::CyclePackage < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'

  import Foo::Bar::CyclePackage::SubPackage
  import Foo::MyPackage

  export Foo::Bar::CyclePackage::OtherClass
  export Foo::Bar::CyclePackage::ImportMeTestOnly
  export Test::Foo::Bar::CyclePackage::TestUtil
end
