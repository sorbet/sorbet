# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::FalseCyclePackage < PackageSpec
  layer 'lib'
  strict_dependencies 'false'

  import Foo::Bar::FalseCyclePackage::SubPackage
  import Foo::MyPackage

  export Foo::Bar::FalseCyclePackage::OtherClass
  export Foo::Bar::FalseCyclePackage::ImportMeTestOnly
  export Test::Foo::Bar::FalseCyclePackage::TestUtil
end
