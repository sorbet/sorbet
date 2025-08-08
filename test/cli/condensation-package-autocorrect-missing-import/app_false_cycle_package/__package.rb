# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::AppFalseCyclePackage < PackageSpec
  layer 'app'
  strict_dependencies 'false'

  import Foo::Bar::AppFalseCyclePackage::SubPackage
  import Foo::MyPackage

  export Foo::Bar::AppFalseCyclePackage::OtherClass
  export Foo::Bar::AppFalseCyclePackage::ImportMeTestOnly
  export Test::Foo::Bar::AppFalseCyclePackage::TestUtil
end
